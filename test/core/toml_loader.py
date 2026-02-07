# toml_loader.py
import re
import os

def load_toml(file_path):
    """
    加载 TOML 文件。如果系统支持 tomllib (Python 3.11+), 则使用它。
    否则，使用一个简单的回退解析器。
    """
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"Config file not found: {file_path}")

    try:
        import tomllib
        with open(file_path, "rb") as f:
            return tomllib.load(f)
    except ImportError:
        # 回退解析逻辑
        return _simple_toml_parser(file_path)

def _simple_toml_parser(file_path):
    """
    一个非常简单的解析器，用于处理本项目中的 test_config.toml。
    不保证完全符合 TOML 规范，但足以处理基本的键值对、小节和数组。
    """
    config = {}
    current_section = config
    current_array_of_tables = None
    
    with open(file_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue

            # 处理数组 [[table_array]]
            if line.startswith("[[") and line.endswith("]]"):
                name = line[2:-2].strip()
                if name not in config:
                    config[name] = []
                new_table = {}
                config[name].append(new_table)
                current_section = new_table
                continue

            # 处理小节 [section]
            if line.startswith("[") and line.endswith("]"):
                name = line[1:-1].strip()
                config[name] = {}
                current_section = config[name]
                continue

            # 处理键值对 key = value
            if "=" in line:
                key, value = line.split("=", 1)
                key = key.strip()
                value = value.strip()

                # 简单解析字符串
                if (value.startswith("'") and value.endswith("'")) or (value.startswith('"') and value.endswith('"')):
                    value = value[1:-1]
                elif value.lower() == "true":
                    value = True
                elif value.lower() == "false":
                    value = False
                elif value.isdigit():
                    value = int(value)

                current_section[key] = value

    return config
