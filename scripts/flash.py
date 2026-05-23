#!/usr/bin/env python3
"""Установка ESP-IDF и прошивка puf_generator на плату ESP32."""

import argparse
import glob
import os
import subprocess
import sys

# ВНИМАНИЕ: IDF_PATH считывается на этапе импорта модуля.
# Установите переменную окружения до запуска скрипта.
IDF_PATH = os.path.expanduser(os.getenv("IDF_PATH") or "~/esp/esp-idf")
IDF_TAG = os.getenv("IDF_TAG") or "v5.4.1"
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
IDF_PY = os.path.join(IDF_PATH, "tools", "idf.py")
ESPRESSIF_DIR = os.path.expanduser("~/.espressif")


def run(args: list[str]) -> None:
    """Запускает внешнюю команду и завершает работу при ненулевом коде возврата."""
    result = subprocess.run(args)
    if result.returncode != 0:
        sys.exit(result.returncode)


def find_venv() -> str:
    """Находит виртуальное окружение Python, установленное ESP-IDF."""
    pattern = os.path.join(ESPRESSIF_DIR, "python_env", "*/bin/python3")
    candidates = glob.glob(pattern)
    if not candidates:
        sys.exit("ESP-IDF Python venv not found. Run install.sh esp32 first.")
    return os.path.dirname(os.path.dirname(candidates[0]))


def build_env(venv: str) -> dict:
    """Формирует переменные окружения для запуска инструментов ESP-IDF."""
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
    """Запускает idf.py с переданными аргументами в подготовленном окружении."""
    venv = find_venv()
    python = os.path.join(venv, "bin", "python")
    env = build_env(venv)
    result = subprocess.run([python, IDF_PY] + args, cwd=PROJECT_ROOT, env=env)
    if result.returncode != 0:
        sys.exit(result.returncode)


def ensure_idf() -> None:
    """Клонирует ESP-IDF и запускает установщик при их отсутствии."""
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
    """Автоматически определяет последовательный порт подключённой платы ESP32."""
    candidates = glob.glob("/dev/cu.usbmodem*") + glob.glob("/dev/cu.SLAB_USBtoUART*")
    if not candidates:
        sys.exit("No ESP32 port found. Plug in the device or pass --port manually.")
    if len(candidates) > 1:
        print(f"Multiple ports found, using {candidates[0]}. Use --port to override.")
    return candidates[0]


def main() -> None:
    """Разбирает аргументы командной строки и запускает сборку, прошивку или монитор."""
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
