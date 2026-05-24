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
            _eprint(f"\n[{i}/{n}] cold-boot: unplug USB, plug back in, then press Enter ...")
            try:
                input()
            except (EOFError, KeyboardInterrupt):
                sys.exit("\naborted")

            resolved_port = port or detect_port()
            _eprint(f"  reading from {resolved_port} ...")

            try:
                hex_line = _read_one_fingerprint(resolved_port, baud, timeout)
            except TimeoutError as e:
                sys.exit(f"  {e}; check that firmware is flashed and PUF command works")

            _eprint(f"  [{i}/{n}] captured {len(hex_line) * 4}-bit fingerprint")
            if save_fp:
                save_fp.write(hex_line + "\n")
                save_fp.flush()
            samples.append(parse_hex(hex_line))
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
