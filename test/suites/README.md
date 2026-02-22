# suites

Canonical table-driven suite definitions.

- `workout_calculator/`: config for `main` application.

Each suite contains:

- `config.toml` (entry, includes `env.toml` + `tests.toml`)
- `env.toml` (environment / deployment paths)
- `tests.toml` (command table)

Path fields in suite TOML support:

- Relative paths (resolved from the TOML file directory)
- `${repo_root}` variable expansion

Schema validation is enforced before test execution starts.

Manual lint command:

- `python test/suites/lint_suites.py --suite workout_calculator`
