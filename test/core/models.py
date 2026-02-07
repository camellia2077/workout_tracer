# models.py
from dataclasses import dataclass, field
from typing import List, Optional

@dataclass
class PathsConfig:
    build_dir: str
    input_dir: str
    test_output_parent_dir: str

@dataclass
class NamesConfig:
    test_output_dir_name: str
    py_output_dir_name: str
    exe_name: str
    config_name: str
    db_name: str

@dataclass
class OptionsConfig:
    clean_on_start: bool = True

@dataclass
class CopyItem:
    type: str
    source_rel: str
    dest_rel: str = ""

@dataclass
class TestConfig:
    paths: PathsConfig
    names: NamesConfig
    options: OptionsConfig
    copy_items: List[CopyItem] = field(default_factory=list)

    @property
    def build_dir(self) -> str:
        return self.paths.build_dir

    @property
    def test_run_dir(self) -> str:
        import os
        return os.path.join(self.paths.test_output_parent_dir, self.names.test_output_dir_name)

    @property
    def exe_path(self) -> str:
        import os
        return os.path.join(self.test_run_dir, self.names.exe_name)

    @property
    def py_output_dir(self) -> str:
        import os
        return os.path.join(self.test_run_dir, self.names.py_output_dir_name)
