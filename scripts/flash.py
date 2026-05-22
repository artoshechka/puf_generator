#!/usr/bin/env python3
"""Setup ESP-IDF and flash puf_generator to an ESP32."""

import argparse
import glob
import os
import subprocess
import sys
from typing import Optional

IDF_PATH = os.path.expanduser("~/esp/esp-idf")
IDF_TAG = "v5.4.1"
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def run(cmd: str, env: Optional[dict] = None) -> None:
    result = subprocess.run(cmd, shell=True, env=env)
    if result.returncode != 0:
        sys.exit(result.returncode)


def ensure_idf() -> None:
    if not os.path.isdir(IDF_PATH):
        print(f"Cloning ESP-IDF {IDF_TAG} ...")
        run(
            f"git clone --recursive --depth 1 --branch {IDF_TAG} "
            f"https://github.com/espressif/esp-idf.git {IDF_PATH}"
        )

    venv = os.path.expanduser("~/.espressif/python_env")
    if not os.path.isdir(venv):
        print("Running ESP-IDF installer ...")
        run(f"{IDF_PATH}/install.sh esp32")


def idf_env() -> dict:
    script = os.path.join(IDF_PATH, "export.sh")
    out = subprocess.check_output(
        f". {script} > /dev/null 2>&1 && env", shell=True, executable="/bin/bash"
    )
    env = {}
    for line in out.decode().splitlines():
        if "=" in line:
            k, _, v = line.partition("=")
            env[k] = v
    return env


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
    env = idf_env()

    port = args.port or (None if args.build_only else detect_port())

    os.chdir(PROJECT_ROOT)
    run(f"idf.py set-target esp32", env=env)

    if args.build_only:
        run("idf.py build", env=env)
    else:
        run(f"idf.py -p {port} flash monitor", env=env)


if __name__ == "__main__":
    main()
