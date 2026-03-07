---
name: auto-build-troubleshooter
description: Execute C/C++ build commands, monitor and persist compiler output, classify compilation/linking/toolchain errors, auto-apply code fixes, and enforce bounded retry policy. Use when a user asks to run cmake+ninja/msbuild build loops, diagnose failed builds, auto-repair compile issues, run Python-assisted checks/tests, and stop safely after max retry count with escalation.
---

# Auto Build Troubleshooter

## Mandatory Constraints

These rules are mandatory and must not be skipped:

1. Default toolchain flow: `cmake + ninja/msbuild`.
2. On every failed build attempt, run `clean + full rebuild` for the next attempt.
3. Auto-modify code to fix detected root causes before the next rebuild.
4. Write fix logs for every attempt to `build/logs/fix-log.md`.
5. Max attempts: `3`.
6. If still failing after attempt 3, stop the task and ask user for help with concrete evidence.
7. Python runtime is available: prefer Python scripts for log parsing, patch support, regression checks, and test orchestration.

## Python Usage Policy

- Use Python helpers whenever deterministic execution is useful.
- Prefer commands like `python -m pytest`, custom Python smoke tests, or Python validation scripts after build success.
- Keep all Python-generated artifacts under `build/logs/` unless user specifies otherwise.

## Default Commands (Windows)

Prefer these defaults unless the user provides project-specific commands.

1. Configure (Ninja):
```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
```

2. Full rebuild (Ninja):
```powershell
cmake --build build --clean-first --parallel
```

3. Full rebuild (MSBuild generator):
```powershell
cmake --build build --config Debug --clean-first --parallel
```

4. Optional Python test command example:
```powershell
python -m pytest -q
```

## Enforced Workflow

1. Initialize
- Ensure `build/logs/` exists.
- Run configure if build files are missing/stale.

2. Attempt loop (`attempt = 1..3`)
- Run full rebuild with clean-first.
- Capture full output via `scripts/run_build_and_analyze.py` and save logs to `build/logs/`.
- On build success, run optional Python-assisted tests/checks.
- If success: log success and stop.
- If failed: identify first actionable root cause and patch code automatically.
- Append one fix record to `build/logs/fix-log.md` (template below).

3. Stop policy
- If attempt 3 still fails, stop immediately.
- Return escalation request with:
  - latest error category + evidence line
  - files changed in all attempts
  - what was tried and why it failed
  - explicit request for user decision/help

## Fix Log Template

Append this block for each failed attempt:

```markdown
## Attempt N - FAILED
- Timestamp: YYYY-MM-DD HH:MM:SS
- Build command: ...
- Test command: ...
- Report: build/logs/attempt-N-report.md
- Root cause category: ...
- Evidence: ...
- Files modified: ...
- Patch summary: ...
- Next hypothesis: ...
```

On success, append:

```markdown
## Attempt N - SUCCESS
- Timestamp: YYYY-MM-DD HH:MM:SS
- Build command: ...
- Test command: ...
- Report: build/logs/attempt-N-report.md
- Fix confirmed: ...
```

## Script Usage

Per attempt run:

```powershell
python skills/auto-build-troubleshooter/scripts/run_build_and_analyze.py `
  --command "cmake --build build --clean-first --parallel" `
  --test-command "python -m pytest -q" `
  --workdir "E:\CodexDev\qt-llm" `
  --log-path "build/logs/attempt-1-build.log" `
  --report-path "build/logs/attempt-1-report.md"
```

## Output Contract

When using this skill, always return:

1. Final state: `SUCCESS` or `STOPPED_AFTER_3_FAILURES`.
2. Attempt count used.
3. Log/report paths in `build/logs/`.
4. Fix log summary from `build/logs/fix-log.md`.
5. If stopped: explicit help request with decision options for the user.

## Diagnostic Heuristics

- Use `references/error-patterns.md` to map text patterns to root causes.
- Prioritize first compile error before downstream linker cascades.
- Resolve include/path/toolchain issues before source refactors.
- Treat `LNK2019` / `undefined reference` as link-stage issues first.

## Resources

### scripts/
- `scripts/run_build_and_analyze.py`: Execute build/test commands, stream output, save logs, classify errors, and emit Markdown summary.

### references/
- `references/error-patterns.md`: Common C/C++/Qt build error signatures and first-line remediation.