"""Shared serial helpers: port detection and default baud resolution.

All scripts in scripts/ should import from here instead of duplicating
their own glob lists. Honours the ESPPORT environment variable (same
convention ESP-IDF uses) and supports Windows COM ports when pyserial
is installed.
"""

import glob
import os
import sys


def _project_root() -> str:
    return os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def default_baud() -> int:
    """Return baud rate from sdkconfig, falling back to 115200.

    Notes
    -----
    When CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=y is selected (current default
    for this project on ESP32-C3) the baud rate is virtual — host pyserial
    ignores it. We still surface CONFIG_ESP_CONSOLE_UART_BAUDRATE so the
    value matches the firmware config for the UART case, and tools that
    might switch to UART later don't drift.
    """
    sdk = os.path.join(_project_root(), "sdkconfig")
    if not os.path.isfile(sdk):
        return 115200
    try:
        with open(sdk, encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if line.startswith("CONFIG_ESP_CONSOLE_UART_BAUDRATE="):
                    return int(line.split("=", 1)[1])
    except (OSError, ValueError):
        pass
    return 115200


def detect_port() -> str:
    """Locate the serial device for the attached ESP32.

    Resolution order:
    1. $ESPPORT (explicit override, same env var ESP-IDF respects)
    2. macOS / Linux globs (cu.usbmodem*, ttyUSB*, ttyACM*, SLAB_USBtoUART*)
    3. Windows COM ports enumerated via pyserial's list_ports

    Exits with a non-zero code if nothing is found.
    """
    explicit = os.environ.get("ESPPORT")
    if explicit:
        return explicit

    candidates: list[str] = (
        glob.glob("/dev/cu.usbmodem*")
        + glob.glob("/dev/cu.SLAB_USBtoUART*")
        + glob.glob("/dev/ttyUSB*")
        + glob.glob("/dev/ttyACM*")
    )

    if sys.platform.startswith("win"):
        try:
            from serial.tools import list_ports  # type: ignore
        except ImportError:
            pass
        else:
            for p in list_ports.comports():
                hwid = (p.hwid or "").upper()
                desc = (p.description or "").upper()
                if "USB" in hwid or "ESP" in desc or "CP210" in desc or "SILICON LABS" in desc:
                    candidates.append(p.device)

    if not candidates:
        sys.exit(
            "No ESP32 port found. Plug in the device, set ESPPORT, or pass --port."
        )
    if len(candidates) > 1:
        print(
            f"Multiple ports found, using {candidates[0]}. Set ESPPORT or pass --port to override.",
            file=sys.stderr,
        )
    return candidates[0]
