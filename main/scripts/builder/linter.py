# scripts/builder/linter.py

import shutil
import subprocess
import sys
from builder.common import Color, print_header
from workflow.task_splitter import process_tidy_results

def run_incremental_lint(build_dir, project_root):
    """
    Run Clang-Tidy analysis on all source files and generate task logs.
    
    NOTE: Auto-fix functionality has been removed. This function only performs
    static analysis and generates task logs. Code modifications are handled by
    AI agents to ensure context-aware, project-compliant refactoring.
    """
    print_header("Running Incremental Clang-Tidy (Check Only)")
    
    src_dir = project_root / "src"
    cpp_files = sorted(list(src_dir.rglob("*.cpp")))
    
    if not cpp_files:
        print(f"{Color.FAIL}No source files found in {src_dir}{Color.ENDC}")
        return

    log_file = build_dir / "clang-tidy.log"
    print(f"Logging full output to: {log_file}")
    
    clang_tidy = shutil.which("clang-tidy")
    if not clang_tidy:
        print(f"{Color.FAIL}Error: clang-tidy not found in PATH.{Color.ENDC}")
        return

    issues_found = False
    collected_results = []
    
    with open(log_file, "w", encoding='utf-8') as f:
        for i, cpp_file in enumerate(cpp_files):
            rel_path = cpp_file.relative_to(project_root)
            display_path = str(rel_path).replace('\\', '/')
            
            print(f"[{i+1}/{len(cpp_files)}] Linting {display_path}...{' ' * 20}", end='\r', flush=True)
            
            cmd = [clang_tidy, "-p", str(build_dir), str(cpp_file), "--quiet"]
                
            result = subprocess.run(cmd, cwd=project_root, capture_output=True, text=True, encoding='utf-8')
            
            combined_output = result.stdout + result.stderr
            if combined_output:
                issues_found = True
                f.write(combined_output)
                collected_results.append({
                    "file": str(cpp_file),
                    "output": combined_output
                })

    # Process task splitting
    tasks_dir = build_dir / "tasks"
    summary_md = build_dir / "tidy_summary.md"
    task_count = process_tidy_results(collected_results, tasks_dir, summary_md, project_root)

    print(f"\n{Color.OKGREEN if not issues_found else Color.HEADER}Clang-tidy completed. {task_count} tasks generated.{Color.ENDC}")
    if task_count > 0:
        print(f"Summary available at: {summary_md}")
