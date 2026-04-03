from __future__ import annotations

import tomllib
from pathlib import Path


def load_exercise_keys(mapping_path: Path) -> list[str]:
    if not mapping_path.exists():
        raise FileNotFoundError(f"Mapping file not found: {mapping_path}")
    if not mapping_path.is_file():
        raise ValueError(f"Mapping path is not a file: {mapping_path}")

    try:
        with mapping_path.open("rb") as f:
            parsed = tomllib.load(f)
    except tomllib.TOMLDecodeError as exc:
        raise ValueError(f"Failed to parse TOML: {mapping_path}: {exc}") from exc

    exercise_keys = [k for k, v in parsed.items() if isinstance(v, dict)]
    if not exercise_keys:
        raise ValueError(f"Mapping is empty or invalid: {mapping_path}")
    return exercise_keys


def output_file_path(output_root: Path, year: int, month: int) -> Path:
    year_dir = output_root / f"{year:04d}"
    return year_dir / f"{year:04d}-{month:02d}.txt"


def write_output_file(output_root: Path, year: int, month: int, content: str) -> Path:
    out_file = output_file_path(output_root, year, month)
    out_file.parent.mkdir(parents=True, exist_ok=True)
    out_file.write_text(content, encoding="utf-8")
    return out_file
