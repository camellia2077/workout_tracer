import os
import subprocess
import sys
from pathlib import Path

def run_command(command, cwd):
    """Executes a command in a specified directory and checks for errors."""
    print(f"--- Running command: {' '.join(command)} in '{cwd}'")
    try:
        # shell=True is generally not recommended, but can be necessary on Windows
        # for commands like 'cmake' if they are not directly in the system PATH
        # in a way Python's subprocess can find them. For MSYS2/MinGW, it's often fine.
        is_windows = sys.platform.startswith('win')
        subprocess.run(command, cwd=cwd, check=True, shell=is_windows)
    except (subprocess.CalledProcessError, FileNotFoundError) as e:
        print(f"!!! Error executing command: {' '.join(command)}", file=sys.stderr)
        print(f"!!! Error: {e}", file=sys.stderr)
        sys.exit(1)

def main():
    """Main function to configure and build the project."""
    # The script's directory is the project's root directory
    project_root = Path(__file__).parent.resolve()
    os.chdir(project_root)
    print(f"Switched to script directory: {project_root}")

    build_dir = project_root / "build"

    # 1. Prepare build directory
    print(f"\n--- Preparing build directory ---")
    if not build_dir.exists():
        print(f"Build directory '{build_dir.name}' not found. Creating it...")
        build_dir.mkdir()
    else:
        print(f"Using existing build directory '{build_dir.name}'.")

    # 2. Configure project with CMake
    print(f"\n--- Configuring project with CMake+Ninja for Release build ---")
    cmake_configure_command = [
        "cmake",
        "..",
        "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE=Release"
    ]
    run_command(cmake_configure_command, cwd=build_dir)

    # 3. Compile project
    print(f"\n--- Compiling project in Release mode with Ninja ---")
    cmake_build_command = [
        "cmake",
        "--build", ".",
        "--config", "Release"
    ]
    run_command(cmake_build_command, cwd=build_dir)

    print(f"\n--- Build process finished successfully! ---")
    print(f"Executables are located at: {build_dir}")

if __name__ == "__main__":
    main()
