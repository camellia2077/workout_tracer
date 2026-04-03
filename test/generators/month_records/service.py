from __future__ import annotations

from pathlib import Path

from .io_layer import load_exercise_keys, write_output_file
from .logic import build_content, validate_year_month
from .models import GeneratorConfig


def default_paths() -> tuple[Path, Path]:
    repo_root = Path(__file__).resolve().parents[3]
    mapping_path = repo_root / "apps" / "config" / "mapping.toml"
    output_root = repo_root / "test" / "data"
    return mapping_path, output_root


def generate_records(config: GeneratorConfig) -> Path:
    validate_year_month(config.year, config.month)
    exercise_keys = load_exercise_keys(config.mapping_path)
    content = build_content(
        config.year, config.month, exercise_keys, active_days=config.active_days
    )
    return write_output_file(config.output_root, config.year, config.month, content)
