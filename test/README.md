# test

Unified executable test workspace.

## Canonical structure

- `framework/`: shared Python testing framework and runner internals.
  - `framework/core/`: reusable testing engine package.
  - `framework/suite_runner.py`: suite execution adapter.
- `compat/`: legacy wrappers forwarding to unified runner.
  - `compat/workout_calculator/`: `run.bat`, `run_fast.bat`, `run_agent.bat`
- `suites/workout_calculator/`: table-driven suite config for `main` app.
- `data/`: shared input data.
- `run.py`: unified entrypoint (`--suite workout_calculator`).
- `run_workout_calculator.bat`: scenario launcher.

## Output layering

Suite writes into:

- `../output/tests/workout_calculator/workspace`: copied binaries and runtime workspace.
- `../output/tests/workout_calculator/logs`: per-case logs + python output log.
- `../output/tests/workout_calculator/artifacts`: generated report/output files.
- `../output/tests/workout_calculator/result.json`: machine-readable summary.

## Config path variables

Suite TOML files support `${repo_root}` and relative paths.

## Quick start

From `workout_calculator/test`:

- `python run.py --suite workout_calculator --build-dir build_agent --concise`
- `python run.py --suite workout_calculator --with-build --build-dir build_agent --concise`
- `run_workout_calculator.bat --concise`
- `run_workout_calculator.bat -b build_fast --with-build --concise`
