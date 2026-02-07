# scripts/workflow/task_splitter.py

import re
import os
from collections import defaultdict
from pathlib import Path
from typing import List, Dict, Tuple

# Pattern to match warning lines with file path
# Example: C:/path/to/file.hpp:42:8: warning: message [check-name]
WARNING_LINE_PATTERN = re.compile(
    r"^([A-Za-z]:)?([^:]+):(\d+):(\d+):\s+warning:\s+(.+)\s+\[([^\]]+)\]"
)

def _generate_summary(warnings: List[Dict]) -> str:
    """Generate a summary header for the task."""
    if not warnings:
        return ""
    
    file_counts: Dict[str, int] = defaultdict(int)
    check_counts: Dict[str, int] = defaultdict(int)
    
    for w in warnings:
        filename = Path(w["file"]).name
        file_counts[filename] += 1
        check_counts[w["check"]] += 1
    
    lines = ["=== SUMMARY ==="]
    file_parts = [f"{f}({c})" for f, c in sorted(file_counts.items(), key=lambda x: -x[1])]
    lines.append(f"Files: {', '.join(file_parts)}")
    
    check_parts = [f"{c}({n})" for c, n in sorted(check_counts.items(), key=lambda x: -x[1])]
    lines.append(f"Types: {', '.join(check_parts)}")
    lines.append("=" * 15 + "\n")
    
    return "\n".join(lines)

def process_tidy_results(results: List[Dict], tasks_dir: Path, summary_md: Path, project_root: Path):
    """
    Process collected tidy results, create task files and a summary table.
    """
    tasks_dir.mkdir(parents=True, exist_ok=True)
    
    # Clean old tasks
    for f in tasks_dir.glob("task_*.log"):
        try: f.unlink()
        except: pass

    processed_tasks = []
    
    for res in results:
        file_path = res["file"]
        output = res["output"]
        
        # Extract warnings from output
        warnings = []
        for line in output.splitlines():
            match = WARNING_LINE_PATTERN.match(line.strip())
            if match:
                drive = match.group(1) or ""
                path = drive + match.group(2)
                warnings.append({
                    "file": path,
                    "check": match.group(6),
                    "message": match.group(5)
                })
        
        if not warnings:
            continue
            
        summary = _generate_summary(warnings)
        content = f"File: {file_path}\n" + "=" * 60 + "\n" + summary + output
        
        processed_tasks.append({
            "target_file": file_path,
            "content": content,
            "warning_count": len(warnings),
            "check_types": set(w["check"] for w in warnings)
        })

    # Sort tasks: files with most warnings first
    processed_tasks.sort(key=lambda x: x["warning_count"], reverse=True)

    # Write task files
    for idx, task in enumerate(processed_tasks, start=1):
        task_id = f"{idx:03d}"
        task_file = tasks_dir / f"task_{task_id}.log"
        task_file.write_text(task["content"], encoding="utf-8")
        task["id"] = task_id

    # Generate Markdown Summary
    if processed_tasks:
        md_lines = ["# Clang-Tidy Tasks Summary\n", "| ID | File | Warnings | Check Types |", "| :-- | :--- | :--- | :--- |"]
        for t in processed_tasks:
            try:
                rel_path = os.path.relpath(t["target_file"], start=project_root).replace('\\', '/')
            except:
                rel_path = t["target_file"]
            checks = ", ".join(sorted(t["check_types"]))
            md_lines.append(f"| {t['id']} | {rel_path} | {t['warning_count']} | {checks} |")
        
        summary_md.write_text("\n".join(md_lines), encoding="utf-8")
    else:
        summary_md.write_text("# Clang-Tidy Tasks Summary\n\nNo issues found! 🎉", encoding="utf-8")

    return len(processed_tasks)
