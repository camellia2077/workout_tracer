# config_manager.py
import os
from toml_loader import load_toml
from models import TestConfig, PathsConfig, NamesConfig, OptionsConfig, CopyItem

class ConfigManager:
    """负责加载 TOML 配置文件并解析为 TestConfig 数据模型。"""
    
    @staticmethod
    def load(file_path: str) -> TestConfig:
        data = load_toml(file_path)
        
        paths = PathsConfig(**data['paths'])
        names = NamesConfig(**data['names'])
        options = OptionsConfig(**data.get('options', {}))
        
        copy_items = []
        for item in data.get('copy_items', []):
            copy_items.append(CopyItem(**item))
            
        return TestConfig(
            paths=paths,
            names=names,
            options=options,
            copy_items=copy_items
        )
