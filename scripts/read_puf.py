#!/usr/bin/env python3
"""Read PUF fingerprint from ESP32 over serial and print it to stdout.

Stdout receives only the raw hex fingerprint — suitable for piping:

    PUF=$(python3 scripts/read_puf.py)
    curl -X POST http://localhost:8080/devices/esp32-001/verify \\
         -H "Authorization: PUF $PUF"

All status messages go to stderr so they do not pollute the captured output.
"""

import argparse
import glob
import re
import sys
import time  # monotonic() для deadline

# Minimum fingerprint length in hex chars (64-bit = 16 chars).
# ESP32 default is 256-bit = 64 chars.
_MIN_HEX_CHARS = 16
_HEX_RE = re.compile(rf"^[0-9a-f]{{{_MIN_HEX_CHARS},}}$")


def eprint(*args, **kwargs) -> None:
    """Print to stderr."""
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


def read_fingerprint(port: str, baud: int, timeout: float, count: int) -> list[str]:
    try:
        import serial  # pyserial
    except ImportError:
        sys.exit("pyserial is required: pip install pyserial")

    eprint(f"Opening {port} at {baud} baud ...")
    fingerprints: list[str] = []

    with serial.Serial(port, baud, timeout=1) as ser:
        ser.reset_input_buffer()

        # Send PUF command — firmware responds with a fresh fingerprint.
        # This is more reliable than DTR reset which doesn't work on ESP32-C3
        # native USB (USB-Serial/JTAG doesn't wire DTR to the EN pin).
        for _ in range(count):
            ser.write(b"PUF\n")
            ser.flush()

        eprint(f"Waiting for fingerprint (timeout {timeout:.0f}s) ...")
        deadline = time.monotonic() + timeout

        while time.monotonic() < deadline and len(fingerprints) < count:
            try:
                raw = ser.readline()
            except Exception as e:
                # ESP32-C3 native USB briefly disconnects on certain operations.
                eprint(f"  serial error: {e}, retrying ...")
                time.sleep(0.5)
                ser.reset_input_buffer()
                continue
            line = raw.decode("ascii", errors="ignore").strip()
            if not line:
                continue
            if _HEX_RE.match(line):
                fingerprints.append(line)
                eprint(f"  [{len(fingerprints)}/{count}] captured {len(line) * 4}-bit fingerprint")
            else:
                eprint(f"  skip: {line[:80]}")

    return fingerprints


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--port", help="Serial port (auto-detected if omitted)")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--timeout", type=float, default=15.0, help="Seconds to wait for fingerprint (default: 15)")
    parser.add_argument("--count", type=int, default=1, help="Number of fingerprints to read (default: 1)")
    args = parser.parse_args()

    port = args.port or detect_port()
    fingerprints = read_fingerprint(port, args.baud, args.timeout, args.count)

    if not fingerprints:
        sys.exit("Timed out waiting for fingerprint. Check that the firmware is flashed and the board is running.")

    for fp in fingerprints:
        print(fp)  # stdout only — clean for piping


if __name__ == "__main__":
    main()
