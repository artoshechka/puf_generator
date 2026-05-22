#!/usr/bin/env python3
"""Send LOGS command to ESP32 over serial and print the log dump to stdout.

Usage:
    python3 scripts/get_board_logs.py
    python3 scripts/get_board_logs.py --port /dev/cu.usbmodem101
    python3 scripts/get_board_logs.py --out board.log
"""

import argparse
import glob
import sys
import time

_DUMP_BEGIN = "--- LOG DUMP BEGIN"
_DUMP_END = "--- LOG DUMP END ---"
_TIMEOUT = 10.0  # секунд ожидания ответа


def eprint(*args, **kwargs) -> None:
    print(*args, file=sys.stderr, **kwargs)


def detect_port() -> str:
    candidates = (
        glob.glob("/dev/cu.usbmodem*")
        + glob.glob("/dev/cu.SLAB_USBtoUART*")
        + glob.glob("/dev/ttyUSB*")
        + glob.glob("/dev/ttyACM*")
    )
    if not candidates:
        sys.exit("No ESP32 port found. Plug in the device or use --port.")
    if len(candidates) > 1:
        eprint(f"Multiple ports found, using {candidates[0]}. Use --port to override.")
    return candidates[0]


def fetch_logs(port: str, baud: int) -> list[str]:
    try:
        import serial
    except ImportError:
        sys.exit("pyserial is required: pip install pyserial")

    eprint(f"Connecting to {port} at {baud} baud ...")

    with serial.Serial(port, baud, timeout=1) as ser:
        ser.reset_input_buffer()

        # Отправляем команду дампа.
        ser.write(b"LOGS\n")
        ser.flush()
        eprint("Sent LOGS command, waiting for response ...")

        lines: list[str] = []
        inside = False
        deadline = time.monotonic() + _TIMEOUT

        while time.monotonic() < deadline:
            raw = ser.readline()
            if not raw:
                continue
            line = raw.decode("utf-8", errors="replace").rstrip("\r\n")

            if _DUMP_BEGIN in line:
                inside = True
                eprint(line)
                continue
            if _DUMP_END in line:
                eprint(line)
                break
            if inside:
                lines.append(line)

        if not inside:
            sys.exit("Timed out: no log dump received. Is the firmware flashed?")

    return lines


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--port", help="Serial port (auto-detected if omitted)")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--out", help="Save logs to file instead of stdout")
    args = parser.parse_args()

    port = args.port or detect_port()
    logs = fetch_logs(port, args.baud)

    output = "\n".join(logs)

    if args.out:
        with open(args.out, "w") as f:
            f.write(output + "\n")
        eprint(f"Saved {len(logs)} lines to {args.out}")
    else:
        print(output)


if __name__ == "__main__":
    main()
