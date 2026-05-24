#!/usr/bin/env python3
"""Сбор и анализ сырых счётчиков осцилляторов с платы.

Запуск:
    python3 scripts/analyze_raw.py [--port PORT] [--runs N]

Скрипт отправляет команду RAW N раз и выводит:
- Среднее значение и стандартное отклонение по каждому осциллятору за все запуски.
- Пары с минимальной разницей счётчиков (наиболее нестабильные биты).
- Прогноз количества стабильных и нестабильных битов.
"""

import argparse
import glob
import re
import sys
import time

_RUN_RE = re.compile(r"^RUN \d+: (.+)$")


def eprint(*a, **kw):
    """Выводит сообщение в стандартный поток ошибок."""
    print(*a, file=sys.stderr, **kw)


def detect_port() -> str:
    """Автоматически определяет последовательный порт подключённой платы ESP32."""
    candidates = (
        glob.glob("/dev/cu.usbmodem*")
        + glob.glob("/dev/cu.SLAB_USBtoUART*")
        + glob.glob("/dev/ttyUSB*")
        + glob.glob("/dev/ttyACM*")
    )
    if not candidates:
        sys.exit("No ESP32 port found.")
    return candidates[0]


def collect(port: str, baud: int, iterations: int) -> list[list[list[int]]]:
    """Собирает счётчики осцилляторов с платы за заданное число итераций.

    Возвращает список итераций; каждая итерация — список снимков RUN;
    каждый снимок — счётчики осцилляторов. Границы итераций сохраняются,
    чтобы анализ не смешивал переходы внутри одной команды RAW с
    переходами между разными RAW-вызовами.
    """
    try:
        import serial
    except ImportError:
        sys.exit("pip install pyserial")

    eprint(f"Opening {port} at {baud} baud ...")
    all_iterations: list[list[list[int]]] = []

    with serial.Serial(port, baud, timeout=2) as ser:
        ser.reset_input_buffer()

        for iteration in range(iterations):
            ser.write(b"RAW\n")
            ser.flush()
            eprint(f"  iteration {iteration + 1}/{iterations} ...")

            inside = False
            completed = False
            iter_runs: list[list[int]] = []
            deadline = time.monotonic() + 30
            while time.monotonic() < deadline:
                raw = ser.readline()
                line = raw.decode("ascii", errors="ignore").strip()
                if not line:
                    continue
                if line.startswith("RAW_BEGIN"):
                    inside = True
                    continue
                if line == "RAW_END":
                    completed = True
                    break
                if inside:
                    m = _RUN_RE.match(line)
                    if m:
                        try:
                            counts = list(map(int, m.group(1).split()))
                        except ValueError:
                            eprint(f"  invalid RUN line: {line}")
                            continue
                        iter_runs.append(counts)

            if not completed:
                eprint(f"  iteration {iteration + 1}: RAW_END not received within 30s")
            if iter_runs:
                all_iterations.append(iter_runs)

    return all_iterations


def analyze(iterations_data: list[list[list[int]]]) -> None:
    """Анализирует собранные счётчики и выводит статистику стабильности битов."""
    if not iterations_data:
        sys.exit("No data collected.")

    flat_runs = [run for iteration in iterations_data for run in iteration]
    n_osc = len(flat_runs[0])
    n_iter = len(iterations_data)
    n_runs = len(flat_runs)
    print(f"Oscillators: {n_osc}, iterations: {n_iter}, total run snapshots: {n_runs}\n")

    # Статистика по каждому осциллятору
    means = []
    stds = []
    for i in range(n_osc):
        vals = [r[i] for r in flat_runs]
        mean = sum(vals) / len(vals)
        std = (sum((v - mean) ** 2 for v in vals) / len(vals)) ** 0.5
        means.append(mean)
        stds.append(std)

    print("Oscillator  mean       std      cv(%)")
    for i in range(n_osc):
        cv = (stds[i] / means[i] * 100) if means[i] else 0
        print(f"  osc[{i:02d}]  {means[i]:10.1f}  {stds[i]:8.2f}  {cv:6.3f}%")

    # Анализ запаса по парам осцилляторов. Используем средние значения для оценки
    # ожидаемой стабильности бита. Перебираем все C(n,2) пар; реальная прошивка
    # формирует отпечаток из подмножества (по CONFIG_PUF_FINGERPRINT_BITS) — пара
    # считается нестабильной по верхней оценке для всех возможных пар.
    print("\nMost unstable pairs (smallest |mean[i] - mean[j]|):")
    pairs = []
    for i in range(n_osc):
        for j in range(i + 1, n_osc):
            diff = abs(means[i] - means[j])
            pairs.append((diff, i, j))
    pairs.sort()

    total_pairs = len(pairs)  # C(n_osc, 2)
    unstable = sum(1 for d, _, _ in pairs if d < 5)
    stable = total_pairs - unstable
    print(
        f"  margin < 5 counts  → {unstable} unstable pair comparisons  "
        f"({unstable}/{total_pairs} of all C({n_osc},2) pairs = {unstable/total_pairs*100:.1f}%)"
    )
    print(f"  margin ≥ 5 counts  → {stable} stable pairs")
    print(
        "  note: firmware uses only CONFIG_PUF_FINGERPRINT_BITS of these pairs;"
        " the percentage is an upper bound."
    )
    print()
    for diff, i, j in pairs[:20]:
        # Частота переключений: доля смежных снимков ВНУТРИ одной итерации,
        # в которых меняется знак сравнения osc[i] > osc[j]. Границы между
        # iterations (отдельные RAW-вызовы) не учитываются — измерения там
        # разделены передачей по UART, переход бита там не "колебание".
        flips = 0
        transitions = 0
        for iteration in iterations_data:
            if len(iteration) < 2:
                continue
            bits = [(r[i] > r[j]) for r in iteration]
            flips += sum(1 for a, b in zip(bits, bits[1:]) if a != b)
            transitions += len(bits) - 1
        denom = transitions if transitions > 0 else 1
        print(f"  [{i:02d},{j:02d}]  margin={diff:8.1f}  flip_rate={flips}/{denom}")


def main():
    """Разбирает аргументы командной строки и запускает сбор и анализ данных."""
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--port", help="Serial port (auto-detected if omitted)")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--iterations", type=int, default=10, help="Number of RAW commands to send (default: 10)")
    args = parser.parse_args()

    port = args.port or detect_port()
    all_runs = collect(port, args.baud, args.iterations)
    analyze(all_runs)


if __name__ == "__main__":
    main()
