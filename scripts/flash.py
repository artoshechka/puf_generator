#!/usr/bin/env python3
"""Setup ESP-IDF and flash puf_generator to an ESP32."""

import argparse
import glob
import os
import subprocess
import sys

IDF_PATH = os.path.expanduser("~/esp/esp-idf")
IDF_TAG = "v5.4.1"
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXPORT_SH = os.path.join(IDF_PATH, "export.sh")


def run(cmd: str) -> None:
    result = subprocess.run(cmd, shell=True)
    if result.returncode != 0:
        sys.exit(result.returncode)


def idf(cmd: str) -> None:
    full = f". {EXPORT_SH} > /dev/null 2>&1 && {cmd}"
    result = subprocess.run(full, shell=True, executable="/bin/bash")
    if result.returncode != 0:
        sys.exit(result.returncode)


def ensure_idf() -> None:
    if not os.path.isdir(IDF_PATH):
        print(f"Cloning ESP-IDF {IDF_TAG} ...")
        run(
            f"git clone --recursive --depth 1 --branch {IDF_TAG} "
            f"https://github.com/espressif/esp-idf.git {IDF_PATH}"
        )

    check = subprocess.run(
        f". {EXPORT_SH} > /dev/null 2>&1",
        shell=True, executable="/bin/bash"
    )
    if check.returncode != 0:
        print("ESP-IDF environment broken or missing, running installer ...")
        run(f"{IDF_PATH}/install.sh esp32")


def detect_port() -> str:
    candidates = glob.glob("/dev/cu.usbmodem*") + glob.glob("/dev/cu.SLAB_USBtoUART*")
    if not candidates:
        sys.exit("No ESP32 port found. Plug in the device or pass --port manually.")
    if len(candidates) > 1:
        print(f"Multiple ports found, using {candidates[0]}. Use --port to override.")
    return candidates[0]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", help="Serial port (e.g. /dev/cu.usbmodem101)")
    parser.add_argument("--build-only", action="store_true", help="Build without flashing")
    args = parser.parse_args()

    ensure_idf()

    os.chdir(PROJECT_ROOT)
    idf("idf.py set-target esp32")

    if args.build_only:
        idf("idf.py build")
    else:
        port = args.port or detect_port()
        idf(f"idf.py -p {port} flash monitor")


if __name__ == "__main__":
    main()
