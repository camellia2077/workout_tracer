from __future__ import annotations

import argparse
import sys
from pathlib import Path

from .models import GeneratorConfig
from .service import default_paths, generate_records


def build_parser() -> argparse.ArgumentParser:
    default_mapping, default_output_root = default_paths()
    parser = argparse.ArgumentParser(
        description="Generate monthly workout records (YYYY-MM.txt)."
    )
    parser.add_argument("--year", required=True, type=int, help="Year in YYYY.")
    parser.add_argument("--month", required=True, type=int, help="Month in 1-12.")
    parser.add_argument(
        "--active-days",
        type=int,
        default=None,
        help=(
            "Limit generated training days to first N sessions from "
            "Mon/Wed/Fri sequence (default: all available)."
        ),
    )
    parser.add_argument(
        "--mapping-path",
        default=str(default_mapping),
        help=f"Path to mapping TOML (default: {default_mapping}).",
    )
    parser.add_argument(
        "--output-root",
        default=str(default_output_root),
        help=f"Root output directory (default: {default_output_root}).",
    )
    return parser


def run(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    try:
        config = GeneratorConfig(
            year=args.year,
            month=args.month,
            mapping_path=Path(args.mapping_path),
            output_root=Path(args.output_root),
            active_days=args.active_days,
        )
        out_file = generate_records(config)
    except (FileNotFoundError, ValueError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 2
    except OSError as exc:
        print(f"Error: failed to write output: {exc}", file=sys.stderr)
        return 3

    print(f"Generated: {out_file}")
    return 0
