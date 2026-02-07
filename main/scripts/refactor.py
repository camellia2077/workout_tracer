import sys
from pathlib import Path
from builder.orchestrator import execute_refactor_workflow

def main():
    project_root = Path(__file__).parent.parent.resolve()
    execute_refactor_workflow(project_root)

if __name__ == "__main__":
    main()
