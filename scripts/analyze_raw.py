#!/usr/bin/env python3
"""Collect and analyze raw oscillator counts from the board.

Usage:
    python3 scripts/analyze_raw.py [--port PORT] [--runs N]

Sends the RAW command N times and prints:
- Per-oscillator mean and std deviation across runs
- Pairs with smallest count difference (most unstable bits)
- Predicted stable vs unstable bit count
"""

import argparse
import glob
import re
import sys
import time

_RUN_RE = re.compile(r"^RUN \d+: (.+)$")


def eprint(*a, **kw):
    print(*a, file=sys.stderr, **kw)


def detect_port() -> str:
    candidates = (
        glob.glob("/dev/cu.usbmodem*")
        + glob.glob("/dev/cu.SLAB_USBtoUART*")
        + glob.glob("/dev/ttyUSB*")
        + glob.glob("/dev/ttyACM*")
    )
    if not candidates:
        sys.exit("No ESP32 port found.")
    return candidates[0]


def collect(port: str, baud: int, iterations: int) -> list[list[int]]:
    try:
        import serial
    except ImportError:
        sys.exit("pip install pyserial")

    eprint(f"Opening {port} at {baud} baud ...")
    all_runs: list[list[int]] = []

    with serial.Serial(port, baud, timeout=2) as ser:
        ser.reset_input_buffer()

        for iteration in range(iterations):
            ser.write(b"RAW\n")
            ser.flush()
            eprint(f"  iteration {iteration + 1}/{iterations} ...")

            inside = False
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
                    break
                if inside:
                    m = _RUN_RE.match(line)
                    if m:
                        counts = list(map(int, m.group(1).split()))
                        all_runs.append(counts)

    return all_runs


def analyze(all_runs: list[list[int]]) -> None:
    if not all_runs:
        sys.exit("No data collected.")

    n_osc = len(all_runs[0])
    n_runs = len(all_runs)
    print(f"Oscillators: {n_osc}, total run snapshots: {n_runs}\n")

    # Per-oscillator stats
    means = []
    stds = []
    for i in range(n_osc):
        vals = [r[i] for r in all_runs]
        mean = sum(vals) / len(vals)
        std = (sum((v - mean) ** 2 for v in vals) / len(vals)) ** 0.5
        means.append(mean)
        stds.append(std)

    print("Oscillator  mean       std      cv(%)")
    for i in range(n_osc):
        cv = (stds[i] / means[i] * 100) if means[i] else 0
        print(f"  osc[{i:02d}]  {means[i]:10.1f}  {stds[i]:8.2f}  {cv:6.3f}%")

    # Pairwise margin analysis (first run snapshot per iteration)
    # Use means to compute expected bit stability
    print("\nMost unstable pairs (smallest |mean[i] - mean[j]|):")
    pairs = []
    for i in range(n_osc):
        for j in range(i + 1, n_osc):
            diff = abs(means[i] - means[j])
            pairs.append((diff, i, j))
    pairs.sort()

    unstable = sum(1 for d, _, _ in pairs if d < 5)
    stable = len(pairs) - unstable
    print(f"  margin < 5 counts  → {unstable} unstable pairs  ({unstable}/{len(pairs)} = {unstable/len(pairs)*100:.1f}%)")
    print(f"  margin ≥ 5 counts  → {stable} stable pairs")
    print()
    for diff, i, j in pairs[:20]:
        # Flip rate: fraction of run snapshots where osc[i] > osc[j] flips
        bits = [(r[i] > r[j]) for r in all_runs]
        flips = sum(1 for a, b in zip(bits, bits[1:]) if a != b)
        print(f"  [{i:02d},{j:02d}]  margin={diff:8.1f}  flip_rate={flips}/{n_runs-1}")


def main():
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
