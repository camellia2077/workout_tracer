# scripts/workflow/log_analyzer.py

import json
import re
import os
from pathlib import Path
from typing import List, Dict

def analyze_tasks(tasks_dir: Path) -> List[Dict]:
    """
    Parse task logs in tasks_dir and return a structured summary.
    Identical to time_tracer strategy.
    """
    if not tasks_dir.exists():
        return []

    summary = []
    file_pattern = re.compile(r"File: (.*)")
    warning_pattern = re.compile(r".*:\d+:\d+: warning: (.*) \[(.*)\]")
    
    log_files = sorted(tasks_dir.glob("task_*.log"))
    
    for log_file in log_files:
        task_id = log_file.stem.split("_")[1]
        try:
            content = log_file.read_text(encoding="utf-8", errors="ignore")
        except Exception:
            continue
            
        file_path = ""
        file_match = file_pattern.search(content)
        if file_match:
            file_path = file_match.group(1).strip()
            
        warnings = []
        for match in warning_pattern.finditer(content):
            msg, w_type = match.groups()
            warning_entry = {"message": msg.strip(), "type": w_type.strip()}
            if warning_entry not in warnings:
                warnings.append(warning_entry)
                
        summary.append({
            "task_id": task_id,
            "log_file": log_file.name,
            "target_file": file_path,
            "warnings": warnings
        })
    return summary

def save_json_summary(summary: List[Dict], output_path: Path):
    """Save the parsed tasks summary to a JSON file."""
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(summary, f, indent=2)

def generate_markdown_summary(summary: List[Dict], output_path: Path, project_root: Path):
    """Generate a Markdown table summary."""
    md_content = "# Clang-Tidy Tasks Summary\n\n"
    md_content += "| ID | File | Warning Types |\n"
    md_content += "| --- | --- | --- |\n"
    
    for item in summary:
        w_types = ", ".join(sorted(set(w["type"] for w in item["warnings"])))
        target = item["target_file"]
        try:
            rel_path = os.path.relpath(target, start=project_root).replace('\\', '/')
        except:
            rel_path = target
            
        md_content += f"| {item['task_id']} | {rel_path} | {w_types} |\n"
        
    output_path.write_text(md_content, encoding="utf-8")
