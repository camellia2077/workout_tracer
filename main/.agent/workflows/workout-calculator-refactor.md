---
description: Safe Clang-Tidy Refactoring Workflow
---

This workflow guides the process of applying Clang-Tidy fixes and verifying them using the fast build pipeline for the `workout_calculator` project.

1. **Select Refactoring Tasks (Batch Strategy)**:
   - Check the `workout_calculator/main/build_tidy/tasks/` directory.
   - **Bulk Trivial Strategy**: For highly uniform, low-risk changes (e.g., `modernize-use-trailing-return-type` or `readability-identifier-length` for simple loop counters), select **5-10 task logs**.
   - **Mixed Batch Strategy**: For mixed cosmetic or readability changes, select **2-4 task logs**.
   - **Complex Strategy**: For logic changes, header renames, or significant structural modifications, select **ONLY 1 task log** to process atomically.
   - **C++23 Style Guidelines (Strict)**:
     - **Types/Funcs**: `PascalCase` (e.g., `Validate()`, NOT `validate()`).
     - **Vars/Params**: `snake_case` (MIN_LEN: 3).
     - **Members**: `snake_case_` (e.g., `is_valid_`).
     - **Consts**: `kPascalCase` (e.g., `kMaxRetry`).

2. **Impact Analysis**:
   - Before renaming any function or class found in the task logs, **MUST** search for all call sites using `grep` or `ripgrep`.
   - Identify all affected layers (infrastructure, application, domain).

3. **Code Refactoring & Safety**:
   - Apply suggested changes based on the selected task logs.
   - **Edit Safety**: If a tool fails due to "content not found", re-read the file via `view_file` to update context.

4. **Verification**:
   - **Build Verification**:
     - **Command**:
       // turbo
       `C:\msys64\msys2_shell.cmd -ucrt64 -defterm -no-start -where . -c "./workout_calculator/main/scripts/build_fast.sh"`
     - Ensure the build completes with "Process finished!" and exit code 0.

5. **Cleanup**:
   - Only if verification passes, use the `refactor.py` script to remove processed logs.
   - **Command**:
     // turbo
     `C:\msys64\msys2_shell.cmd -ucrt64 -defterm -no-start -where . -c "python ./workout_calculator/main/scripts/refactor.py clean <ID1> <ID2> ..."`
   - *Example*: `python ./workout_calculator/main/scripts/refactor.py clean 001 002`
