#!/usr/bin/env python3
"""Setup ESP-IDF and flash puf_generator to an ESP32."""

import argparse
import glob
import os
import subprocess
import sys

# NOTE: IDF_PATH is captured at import time. Set it before running the script.
IDF_PATH = os.path.expanduser(os.getenv("IDF_PATH", "~/esp/esp-idf"))
IDF_TAG = os.getenv("IDF_TAG", "v5.4.1")
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
IDF_PY = os.path.join(IDF_PATH, "tools", "idf.py")
ESPRESSIF_DIR = os.path.expanduser("~/.espressif")


def run(args: list[str]) -> None:
    result = subprocess.run(args)
    if result.returncode != 0:
        sys.exit(result.returncode)


def find_venv() -> str:
    pattern = os.path.join(ESPRESSIF_DIR, "python_env", "*/bin/python3")
    candidates = glob.glob(pattern)
    if not candidates:
        sys.exit("ESP-IDF Python venv not found. Run install.sh esp32 first.")
    return os.path.dirname(os.path.dirname(candidates[0]))


def build_env(venv: str) -> dict:
    env = os.environ.copy()
    env["IDF_PATH"] = IDF_PATH
    env["IDF_PYTHON_ENV_PATH"] = venv
    venv_bin = os.path.join(venv, "bin")
    idf_tools_bin = os.path.join(IDF_PATH, "tools")
    esp_tools = os.path.join(ESPRESSIF_DIR, "tools")
    xtensa_bins = glob.glob(os.path.join(esp_tools, "xtensa-esp-elf", "*", "xtensa-esp-elf", "bin"))
    xtensa_bins += glob.glob(os.path.join(esp_tools, "riscv32-esp-elf", "*", "riscv32-esp-elf", "bin"))
    rom_elfs = glob.glob(os.path.join(esp_tools, "esp-rom-elfs", "*"))
    if rom_elfs:
        env["ESP_ROM_ELF_DIR"] = rom_elfs[0]
    env["PATH"] = ":".join([venv_bin, idf_tools_bin] + xtensa_bins + [env.get("PATH", "")])
    return env


def idf(args: list[str]) -> None:
    venv = find_venv()
    python = os.path.join(venv, "bin", "python")
    env = build_env(venv)
    result = subprocess.run([python, IDF_PY] + args, cwd=PROJECT_ROOT, env=env)
    if result.returncode != 0:
        sys.exit(result.returncode)


def ensure_idf() -> None:
    if not os.path.isdir(IDF_PATH):
        print(f"Cloning ESP-IDF {IDF_TAG} ...")
        run([
            "git",
            "clone",
            "--recursive",
            "--depth",
            "1",
            "--branch",
            IDF_TAG,
            "https://github.com/espressif/esp-idf.git",
            IDF_PATH,
        ])

    if not glob.glob(os.path.join(ESPRESSIF_DIR, "python_env", "*/bin/python3")):
        print("Running ESP-IDF installer ...")
        run([os.path.join(IDF_PATH, "install.sh"), "esp32"])


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
    parser.add_argument("--monitor-only", action="store_true", help="Open monitor without building or flashing")
    parser.add_argument("--menuconfig", action="store_true", help="Open interactive firmware configuration menu")
    args = parser.parse_args()

    ensure_idf()

    os.chdir(PROJECT_ROOT)

    if args.menuconfig:
        idf(["menuconfig"])
    elif args.monitor_only:
        port = args.port or detect_port()
        idf(["-p", port, "monitor"])
    elif args.build_only:
        idf(["set-target", "esp32c3"])
        idf(["build"])
    else:
        port = args.port or detect_port()
        idf(["set-target", "esp32c3"])
        idf(["-p", port, "flash", "monitor"])


if __name__ == "__main__":
    main()
