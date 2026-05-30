# Contributing to Infra

This project follows common patterns used by large C/C++ projects (OpenCV, LLVM, Boost).
Follow these guidelines to contribute effectively.

Developer workflow
- Use out-of-source builds. Prefer Ninja for local development to generate `compile_commands.json`:

```bash
cmake --preset debug
cmake --build --preset default
ctest --preset default
```

- Use `scripts/format.sh` or `scripts/format.ps1` to apply project formatting (`.clang-format`).
- Use `scripts/lint.sh` or `scripts/lint.ps1` to run `clang-tidy` against `compile_commands.json` (requires Ninja or other generator that supports `CMAKE_EXPORT_COMPILE_COMMANDS`).

Code style
- Follow `.clang-format` and `.clang-tidy` files in repository root.
- Keep public headers under `include/` and avoid exposing internal headers.

Branches & PRs
- Create feature branches per change. Rebase or squash commits to keep history clean.
- Provide a clear PR description, include change rationale and a short testing checklist.

Testing
- Add unit tests under `modules/<module>/tests` and register them with `infra_add_test` helper so they run under `ctest`.

CI
- The repository configures CI to run format, lint (Linux), build and tests. Fix issues locally before pushing.

Thanks for contributing! If you're new, open an issue describing your planned changes and the maintainers will help triage.
