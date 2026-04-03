from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class GeneratorConfig:
    year: int
    month: int
    mapping_path: Path
    output_root: Path
    active_days: int | None = None
