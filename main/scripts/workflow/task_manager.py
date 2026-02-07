# scripts/workflow/task_manager.py

import os
from pathlib import Path
from typing import List
from builder.common import Color

def list_tasks(tasks_dir: Path):
    """List all available task logs."""
    if not tasks_dir.exists():
        print(f"{Color.WARNING}No tasks directory found.{Color.ENDC}")
        return
    
    logs = sorted(tasks_dir.glob("task_*.log"))
    if not logs:
        print(f"{Color.OKGREEN}No pending tasks found. Project is clean! 🎉{Color.ENDC}")
        return

    print(f"\n{Color.HEADER}Pending Tasks in {tasks_dir.name}:{Color.ENDC}")
    for log in logs:
        print(f"  - {log.name}")

def cleanup_task_logs(tasks_dir: Path, task_ids: List[str]) -> int:
    """Delete task logs matching the provided IDs."""
    if not tasks_dir.exists():
        return 0

    deleted_count = 0
    for tid in task_ids:
        # Handle both "001" and "1"
        pattern = tid.zfill(3)
        matches = list(tasks_dir.glob(f"task_{pattern}*.log"))
        
        for f in matches:
            try:
                f.unlink()
                print(f"  {Color.OKBLUE}[DELETED]{Color.ENDC} {f.name}")
                deleted_count += 1
            except Exception as e:
                print(f"  {Color.FAIL}[ERROR]{Color.ENDC} Failed to delete {f.name}: {e}")
                
    return deleted_count
