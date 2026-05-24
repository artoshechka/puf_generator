#!/usr/bin/env python3
"""Collect and/or analyse single-device PUF metrics.

Computes the four metrics implemented in components/puf_metrics
(HammingDistance, FractionalHD, IntraHD, Uniformity) on a series of
cold-boot fingerprints from one device. InterHD is intentionally
excluded — analysis is scoped to a single device.

Two input modes:

  1. --collect N
        Drives the collection loop interactively. For each of the N
        iterations: prompts to power-cycle the board (unplug / replug),
        sends a PUF command over serial, captures the hex response.
        With --save PATH also writes the captured hex to a file.

  2. positional argument (file path or '-' for stdin)
        Reads pre-captured hex fingerprints, one per line. Empty lines
        and lines starting with '#' are ignored.

All fingerprints must have the same byte length.
"""

from __future__ import annotations

import argparse
import math
import os
import re
import statistics
import sys
import time


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


def _format_hex(fp: bytes, group: int = 8) -> str:
    h = fp.hex()
    return " ".join(h[i:i + group] for i in range(0, len(h), group))


def _bits_of(fp: bytes) -> list[int]:
    out: list[int] = []
    for byte in fp:
        for k in range(7, -1, -1):
            out.append((byte >> k) & 1)
    return out


def _bits_to_bytes(bits: list[int]) -> bytes:
    out = bytearray(len(bits) // 8)
    for idx, bit in enumerate(bits):
        if bit:
            out[idx // 8] |= 1 << (7 - (idx % 8))
    return bytes(out)


def bit_stability(samples: list[bytes]) -> tuple[list[int], list[float]]:
    """Return (per-bit ones-count, per-bit p(1)) across all samples."""
    n = len(samples)
    bit_len = len(samples[0]) * 8
    ones = [0] * bit_len
    for s in samples:
        for i, b in enumerate(_bits_of(s)):
            ones[i] += b
    probs = [c / n for c in ones]
    return ones, probs


def majority_vote(samples: list[bytes]) -> bytes:
    """Per-bit majority. Ties (only with even N) resolve to 0."""
    n = len(samples)
    ones, _ = bit_stability(samples)
    bits = [1 if c * 2 > n else 0 for c in ones]
    return _bits_to_bytes(bits)


def min_entropy_estimate(probs: list[float]) -> float:
    """Lower bound on per-bit min-entropy, summed across the fingerprint.

    H_min(bit i) = -log2(max(p_i, 1 - p_i)). Sum is a rough upper bound on
    full-fingerprint min-entropy assuming independent bits; treats sampled
    frequencies as ground truth, which is unreliable for small N.
    """
    total = 0.0
    for p in probs:
        q = max(p, 1.0 - p)
        if q <= 0.0:
            return float("inf")  # impossible given counts; defensive
        total += -math.log2(q)
    return total


def print_report(samples: list[bytes], verbose: bool = False) -> None:
    n = len(samples)
    byte_len = len(samples[0])
    bit_len = byte_len * 8

    print(f"Samples:        {n} fingerprints, {bit_len} bits ({byte_len} bytes) each")
    print()

    print("Captured fingerprints:")
    for i, s in enumerate(samples, 1):
        print(f"  [{i:>2}]  {_format_hex(s)}")
    print()

    unis = [uniformity(s) for s in samples]
    print("Per-sample Uniformity:")
    for i, u in enumerate(unis, 1):
        print(f"  [{i:>2}]  {u:.4f}")
    print(
        f"  -> mean={statistics.fmean(unis):.4f}  "
        f"min={min(unis):.4f}  max={max(unis):.4f}  (ideal 0.5)"
    )
    print()

    if n >= 2:
        mean_hd, min_hd, max_hd, pairs = intra_hd(samples)
        print("Pairwise FractionalHD:")
        header = "        " + "  ".join(f"{j:>6}" for j in range(1, n + 1))
        print(header)
        for i in range(n):
            cells = []
            for j in range(n):
                if j <= i:
                    cells.append("     -")
                else:
                    cells.append(f"{fractional_hd(samples[i], samples[j]):.4f}")
            print(f"  [{i+1:>2}]  " + "  ".join(cells))
        print()

        print("Pairwise HammingDistance (bits):")
        print(header)
        for i in range(n):
            cells = []
            for j in range(n):
                if j <= i:
                    cells.append("     -")
                else:
                    cells.append(f"{hamming_distance(samples[i], samples[j]):>6}")
            print(f"  [{i+1:>2}]  " + "  ".join(cells))
        print()

        print(
            f"IntraHD:        mean={mean_hd:.4f}  min={min_hd:.4f}  max={max_hd:.4f}  "
            f"(pairs={len(pairs)}, ideal 0.0, acceptable <0.05)"
        )
        print()

        ones, probs = bit_stability(samples)
        stable = sum(1 for c in ones if c == 0 or c == n)
        unstable = bit_len - stable
        print("Bit stability across samples:")
        print(f"  total bits:    {bit_len}")
        print(f"  stable bits:   {stable:>4} ({stable / bit_len * 100:>5.2f}%)  always 0 or always 1")
        print(f"  unstable bits: {unstable:>4} ({unstable / bit_len * 100:>5.2f}%)")
        print()

        hmin = min_entropy_estimate(probs)
        suffix = "" if n >= 20 else f"  (rough, N={n}; >=20 recommended)"
        print(f"Min-entropy:    H_inf >= {hmin:.1f} bits / {bit_len}{suffix}")
        print()

        ref = majority_vote(samples)
        print(f"Majority-vote reference (bitwise):")
        print(f"        {_format_hex(ref)}")
    else:
        print("IntraHD:        n/a (need >= 2 samples)")
        print("Bit stability:  n/a (need >= 2 samples)")
        print("Min-entropy:    n/a (need >= 2 samples)")


_MIN_HEX_CHARS = 16
_HEX_RE = re.compile(rf"^[0-9a-f]{{{_MIN_HEX_CHARS},}}$")


def _eprint(*args, **kwargs) -> None:
    print(*args, file=sys.stderr, **kwargs)


def _read_one_fingerprint(port: str, baud: int, timeout: float) -> str:
    """Send PUF over serial, return the first hex line matching _HEX_RE."""
    try:
        import serial  # pyserial
    except ImportError:
        sys.exit("pyserial is required: pip install pyserial")

    with serial.Serial(port, baud, timeout=1) as ser:
        ser.reset_input_buffer()
        ser.write(b"PUF\n")
        ser.flush()

        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            try:
                raw = ser.readline()
            except Exception as e:
                _eprint(f"  serial error: {e}, retrying ...")
                time.sleep(0.5)
                ser.reset_input_buffer()
                continue
            line = raw.decode("ascii", errors="ignore").strip()
            if not line:
                continue
            if _HEX_RE.match(line):
                return line
            _eprint(f"  skip: {line[:80]}")

    raise TimeoutError("timed out waiting for fingerprint")


def collect_samples(n: int, port: str | None, baud: int, timeout: float, save_path: str | None) -> list[bytes]:
    """Interactive collection of N cold-boot fingerprints."""
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from _serial import detect_port  # noqa: WPS433

    samples: list[bytes] = []
    save_fp = open(save_path, "w", encoding="utf-8") if save_path else None
    try:
        for i in range(1, n + 1):
            while True:
                _eprint(f"\n[{i}/{n}] cold-boot: unplug USB, plug back in, then press Enter ...")
                try:
                    input()
                except (EOFError, KeyboardInterrupt):
                    sys.exit("\naborted")

                try:
                    resolved_port = port or detect_port()
                except SystemExit as e:
                    _eprint(str(e))
                    _eprint("  port not found; try again")
                    continue
                _eprint(f"  reading from {resolved_port} ...")

                try:
                    hex_line = _read_one_fingerprint(resolved_port, baud, timeout)
                except TimeoutError as e:
                    _eprint(f"  {e}; try again")
                    continue

                _eprint(f"  [{i}/{n}] captured {len(hex_line) * 4}-bit fingerprint")
                if save_fp:
                    save_fp.write(hex_line + "\n")
                    save_fp.flush()
                samples.append(parse_hex(hex_line))
                break
    finally:
        if save_fp:
            save_fp.close()

    return samples


def main() -> None:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "input",
        nargs="?",
        default=None,
        help="Path to file with hex fingerprints, one per line. '-' = stdin. Ignored with --collect.",
    )
    parser.add_argument(
        "-c", "--collect",
        type=int,
        metavar="N",
        help="Interactively collect N cold-boot fingerprints from the board.",
    )
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Print per-sample Uniformity and full pairwise HD matrix.",
    )
    parser.add_argument(
        "--save",
        metavar="PATH",
        help="With --collect, also write captured hex fingerprints to this file.",
    )
    # Lazy import: default_baud reads sdkconfig, only needed when collecting.
    parser.add_argument("--port", help="Serial port (auto-detected if omitted, only with --collect)")
    parser.add_argument("--baud", type=int, default=0, help="Baud rate (default from sdkconfig)")
    parser.add_argument("--timeout", type=float, default=15.0, help="Seconds to wait per fingerprint (default 15)")
    args = parser.parse_args()

    if args.collect is not None:
        if args.collect < 1:
            sys.exit("--collect N must be >= 1")
        if args.input is not None:
            _eprint("warning: positional input argument ignored when --collect is set")

        baud = args.baud
        if baud == 0:
            sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
            from _serial import default_baud  # noqa: WPS433
            baud = default_baud()

        samples = collect_samples(args.collect, args.port, baud, args.timeout, args.save)
    else:
        samples = load_samples(args.input or "-")

    print()
    print_report(samples, args.verbose)


if __name__ == "__main__":
    main()
