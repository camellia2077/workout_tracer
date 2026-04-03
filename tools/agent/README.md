# Agent Toolchain Architecture

This directory contains the automation scripts for Project Agent. It is designed with a layered architecture to ensure maintainability and predictability for both humans and AI agents.

## Directory Structure

- `core/`: **Infrastructure Layer**. Handles environment setup, process execution (with real-time feedback), and application registration.
- `services/`: **Logic Layer**. Pure functions for parsing diagnostics, planning rename candidates, and driving `clangd` LSP edits.
- `commands/`: **Workflow Layer**. Orchestrates multi-step processes like `configure`, `build`, `tidy`, `clean`, and rename automation (`rename-plan`, `rename-apply`, `rename-audit`).

## Key Design Principles

1. **Real-time Feedback**: All intensive processes (CMake, Clang-Tidy) must provide line-buffered real-time output to prevent perceived deadlocks.
2. **Context-Aware**: The `Context` object centralizes path management and environment setup.
3. **AI-Friendly**: Clearly defined boundaries between "How to run" (Core) and "What to do" (Commands) reduce the cognitive load for LLMs performing refactoring tasks.

## Usage

All commands are accessed via the root `tools/run.py`.

```bash
# Example: Run tidy for time_tracer
python tools/run.py tidy --app time_tracer

# Android commands from the repo root
python tools/run.py android assemble-debug
python tools/run.py android assemble-release
python tools/run.py android native-debug
python tools/run.py android native-release
python tools/run.py android test-debug

# Configure and build are split commands
python tools/run.py configure --app time_tracer
python tools/run.py build --app time_tracer
python tools/run.py format --app time_tracer

# Process cleanup is opt-in (single-project serial workflow is faster by default)
python tools/run.py build --app time_tracer --kill-build-procs

# Tune tidy parallelism
python tools/run.py tidy --app time_tracer --jobs 16 --parse-workers 8

# Auto loop for rename-only tasks with periodic verify
python tools/run.py tidy-loop --app time_tracer --n 10 --test-every 3 --concise
python tools/run.py tidy-loop --app time_tracer --all --test-every 3 --concise

# Build and apply rename plan for naming warnings
python tools/run.py rename-plan --app time_tracer
python tools/run.py rename-apply --app time_tracer
python tools/run.py rename-audit --app time_tracer
```
