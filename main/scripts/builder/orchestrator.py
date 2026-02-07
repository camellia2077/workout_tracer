# scripts/builder/orchestrator.py

import sys
import subprocess
from pathlib import Path
from builder.common import Color, print_header
from workflow import task_manager, log_analyzer

class RefactorOrchestrator:
    """
    Orchestrates the Clang-Tidy refactoring workflow.
    Provides methods to analyze issues, list tasks, and clean up.
    """
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.build_dir = project_root / "build_tidy"
        self.tasks_dir = self.build_dir / "tasks"
        self.summary_md = self.build_dir / "tidy_summary.md"
        self.build_py = project_root / "scripts" / "build.py"

    def analyze(self):
        """
        Run Clang-Tidy analysis and generate tasks.
        
        NOTE: Auto-fix functionality has been removed. Code modifications are now
        handled by AI agents using the task logs as input. This ensures context-aware,
        project-compliant refactoring that respects our C++23 style guidelines.
        """
        print_header("Starting Workflow: Analysis")
        
        cmd = [sys.executable, str(self.build_py), "--lint"]
            
        try:
            subprocess.run(cmd, cwd=self.project_root, check=True)
            print(f"\n{Color.OKGREEN}Workflow step completed successfully.{Color.ENDC}")
        except subprocess.CalledProcessError as e:
            print(f"\n{Color.FAIL}Workflow step failed with exit code {e.returncode}.{Color.ENDC}")

    def list_tasks(self):
        """List current pending tasks."""
        task_manager.list_tasks(self.tasks_dir)
        if self.summary_md.exists():
            print(f"\nRefer to summary for details: {self.summary_md}")

    def clean_tasks(self, task_ids: list):
        """Remove specific task logs."""
        count = task_manager.cleanup_task_logs(self.tasks_dir, task_ids)
        print(f"\n{Color.OKGREEN}Cleanup finished. Removed {count} tasks.{Color.ENDC}")

import argparse

def execute_refactor_workflow(project_root: Path):
    """
    CLI entry point for the refactoring workflow.
    """
    orchestrator = RefactorOrchestrator(project_root)

    parser = argparse.ArgumentParser(description="AI-Assisted Refactoring Workflow CLI")
    subparsers = parser.add_subparsers(dest="command", help="Command to execute")

    # Analyze (removed --fix option)
    subparsers.add_parser("analyze", help="Run Clang-Tidy analysis and generate tasks")

    # List
    subparsers.add_parser("list", help="List all pending tasks")

    # Clean
    clean_parser = subparsers.add_parser("clean", help="Remove completed task logs")
    clean_parser.add_argument("ids", nargs="+", help="Task IDs to remove (e.g., 001 002)")

    args = parser.parse_args()

    if args.command == "analyze":
        orchestrator.analyze()
    elif args.command == "list":
        orchestrator.list_tasks()
    elif args.command == "clean":
        orchestrator.clean_tasks(args.ids)
    else:
        parser.print_help()
