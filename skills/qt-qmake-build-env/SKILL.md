---
name: qt-qmake-build-env
description: Set up and use a Qt 5.15.2 + MSVC2019/2022 qmake build environment on Windows. Use when compiling, rebuilding, or running tests for this repository (or similar Qt/qmake projects) requires entering qtenv2.bat and then vcvarsall.bat so qmake, cl, and nmake are all available.
---

# Qt Qmake Build Env

Prepare a working Windows build shell for Qt/qmake projects, then run qmake and nmake reliably.

## Use This Workflow

1. Bootstrap Qt environment.
2. Bootstrap MSVC compiler environment.
3. Verify required tools (`qmake`, `cl`, `nmake`).
4. Run qmake in an out-of-source build directory.
5. Build with nmake.

## Commands

Use one-shot execution (`/C`) when running a single build command:

```bat
C:\Windows\System32\cmd.exe /A /Q /C "call E:\Qt\5.15.2\msvc2019_64\bin\qtenv2.bat && call \"C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat\" x64 && qmake -v && where cl && where nmake"
```

Use interactive shell (`/K`) when keeping the environment for multiple commands:

```bat
C:\Windows\System32\cmd.exe /A /Q /K E:\Qt\5.15.2\msvc2019_64\bin\qtenv2.bat
```

Then run in that shell:

```bat
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
```

## Repository Build (qt-llm)

From repository root `E:\CodexDev\qt-llm`:

```bat
if not exist build\codex-qmake-test mkdir build\codex-qmake-test
cd /d build\codex-qmake-test
qmake ..\..\qt-llm.pro -spec win32-msvc "CONFIG+=debug"
nmake /NOLOGO
```

## Validation Rules

1. Require `qmake -v` to show Qt `5.15.2` path before build.
2. Require `where cl` and `where nmake` to return paths before build.
3. If `qtenv2.bat` prints `Remember to call vcvarsall.bat`, always run `vcvarsall.bat x64`.
4. Use out-of-source build directory; do not generate Makefiles in source tree.

## Fallbacks

1. If VS2022 path does not exist, try VS2019:

```bat
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
```

2. If tool check fails, stop and report which tool is missing (`qmake`, `cl`, or `nmake`).
