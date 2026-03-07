# Common Build Error Patterns

Use this table as quick triage guidance after running the analyzer script.

## Compiler and Parser Errors

- `fatal error: ... No such file or directory`
  - Cause: Missing header/source path or wrong filename case.
  - First action: Check include directories and project file list.

- `... was not declared in this scope` / `undeclared identifier`
  - Cause: Missing declarations, namespace issues, include ordering.
  - First action: Add/verify header include and qualified name.

- `no matching function for call to` / `cannot convert`
  - Cause: Argument type mismatch or wrong overload.
  - First action: Compare callsite types with target signature.

- `syntax error` / `expected ...`
  - Cause: Broken token sequence, macro side effects, missing delimiters.
  - First action: Fix earliest syntax error before any later error.

## Linker Errors

- `undefined reference to` / `LNK2019`
  - Cause: Missing implementation or library linkage.
  - First action: Ensure object/library is part of the link step.

- `multiple definition` / `LNK2005`
  - Cause: Duplicate symbol definitions across translation units.
  - First action: Keep one definition, move shared declarations to headers.

## Qt-Specific Signals

- `undefined reference to vtable for ...`
  - Cause: Qt Meta-Object data not generated/linked.
  - First action: Confirm `Q_OBJECT` class header is tracked by build system and rerun qmake/cmake.

- `moc_*.cpp` / `uic` / `rcc` failures
  - Cause: Meta-object/UI/resource generation mismatch.
  - First action: Clean generated files and regenerate build files.

## Toolchain and Command Issues

- `is not recognized as an internal or external command`
  - Cause: Tool missing in PATH.
  - First action: Add compiler/build tool to PATH and reopen shell.

- `No rule to make target`
  - Cause: Stale build files or wrong build directory.
  - First action: Regenerate build files in a clean build dir.

- `CMake Error` / `ninja: error`
  - Cause: Configure/build mismatch or invalid target dependency.
  - First action: Reconfigure from scratch and validate generator/toolchain pair.