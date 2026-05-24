#!/usr/bin/env python3
"""Compute single-device PUF metrics from a list of cold-boot fingerprints.

Mirrors the four metrics implemented in components/puf_metrics
(HammingDistance, FractionalHD, IntraHD, Uniformity) — but in Python,
so they can be evaluated on a series of captured hex strings without
re-running anything on the board.

InterHD is intentionally excluded: this script analyses ONE device's
samples. For inter-device comparisons collect fingerprints separately
and diff them manually.

Input:
    A text file (or stdin) with one hex fingerprint per line. Empty lines
    and lines starting with '#' are ignored. All fingerprints must have
    the same byte length.

Typical workflow:

    # Collect N cold-boot fingerprints (power-cycle between each read).
    : > /tmp/puf_samples.hex
    for i in 1 2 3 4 5; do
        echo "==> cold-boot #$i: unplug the board, replug, press Enter"
        read
        make puf >> /tmp/puf_samples.hex
    done

    # Analyse.
    python3 scripts/analyze_metrics.py /tmp/puf_samples.hex
    python3 scripts/analyze_metrics.py /tmp/puf_samples.hex --verbose
"""

import argparse
import statistics
import sys


def _popcount(byte: int) -> int:
    return bin(byte).count("1")


def hamming_distance(a: bytes, b: bytes) -> int:
    if len(a) != len(b):
        raise ValueError(f"length mismatch: {len(a)} vs {len(b)}")
    return sum(_popcount(x ^ y) for x, y in zip(a, b))


def fractional_hd(a: bytes, b: bytes) -> float:
    return hamming_distance(a, b) / (len(a) * 8)


def uniformity(fp: bytes) -> float:
    if not fp:
        return 0.0
    return sum(_popcount(b) for b in fp) / (len(fp) * 8)


def intra_hd(samples: list[bytes]) -> tuple[float, float, float, list[float]]:
    """Return (mean, min, max, all_pairwise_FHDs)."""
    if len(samples) < 2:
        raise ValueError("need at least 2 samples")
    pairs = [
        fractional_hd(samples[i], samples[j])
        for i in range(len(samples))
        for j in range(i + 1, len(samples))
    ]
    return statistics.fmean(pairs), min(pairs), max(pairs), pairs


def parse_hex(line: str) -> bytes:
    line = line.strip().lower()
    if len(line) % 2 != 0:
        raise ValueError(f"odd-length hex string: {line[:40]}...")
    try:
        return bytes.fromhex(line)
    except ValueError as e:
        raise ValueError(f"invalid hex: {e}") from e


def load_samples(path: str) -> list[bytes]:
    if path == "-":
        text = sys.stdin.read()
    else:
        with open(path, encoding="utf-8") as f:
            text = f.read()

    samples: list[bytes] = []
    for n, raw in enumerate(text.splitlines(), 1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        try:
            samples.append(parse_hex(line))
        except ValueError as e:
            sys.exit(f"line {n}: {e}")

    if not samples:
        sys.exit("no fingerprints found in input")

    lengths = {len(s) for s in samples}
    if len(lengths) > 1:
        sys.exit(f"fingerprints have mixed byte lengths: {sorted(lengths)}")

    return samples


def print_report(samples: list[bytes], verbose: bool) -> None:
    n = len(samples)
    byte_len = len(samples[0])
    bit_len = byte_len * 8

    print(f"Samples:    {n} fingerprints, {bit_len} bits ({byte_len} bytes) each")

    unis = [uniformity(s) for s in samples]
    print(
        f"Uniformity: mean={statistics.fmean(unis):.4f}  "
        f"min={min(unis):.4f}  max={max(unis):.4f}  (ideal 0.5)"
    )

    if n >= 2:
        mean_hd, min_hd, max_hd, pairs = intra_hd(samples)
        print(
            f"IntraHD:    mean={mean_hd:.4f}  min={min_hd:.4f}  max={max_hd:.4f}  "
            f"(pairs={len(pairs)}, ideal 0.0, acceptable <0.05)"
        )
    else:
        print("IntraHD:    n/a (need >= 2 samples)")
        pairs = []

    if not verbose:
        return

    print()
    print("Per-sample Uniformity:")
    for i, u in enumerate(unis, 1):
        print(f"  [{i:>2}]  {u:.4f}")

    if n >= 2:
        print()
        print("Pairwise FractionalHD / HammingDistance (bits):")
        header = "       " + "  ".join(f"{j:>6}" for j in range(1, n + 1))
        print(header)
        for i in range(n):
            row_cells = []
            for j in range(n):
                if j <= i:
                    row_cells.append("     -")
                else:
                    fhd = fractional_hd(samples[i], samples[j])
                    row_cells.append(f"{fhd:.4f}")
            print(f"  [{i+1:>2}]  " + "  ".join(row_cells))

        print()
        print("Pairwise HammingDistance (bits):")
        print(header)
        for i in range(n):
            row_cells = []
            for j in range(n):
                if j <= i:
                    row_cells.append("     -")
                else:
                    hd = hamming_distance(samples[i], samples[j])
                    row_cells.append(f"{hd:>6}")
            print(f"  [{i+1:>2}]  " + "  ".join(row_cells))


def main() -> None:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "input",
        nargs="?",
        default="-",
        help="Path to file with hex fingerprints, one per line. '-' or omitted = stdin.",
    )
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Print per-sample Uniformity and full pairwise HD matrix.",
    )
    args = parser.parse_args()

    samples = load_samples(args.input)
    print_report(samples, args.verbose)


if __name__ == "__main__":
    main()
