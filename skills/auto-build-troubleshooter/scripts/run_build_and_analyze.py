#!/usr/bin/env python3
"""Run build command(s), persist logs, and summarize likely root causes."""

from __future__ import annotations

import argparse
import datetime as dt
import os
import re
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import List, Sequence


@dataclass(frozen=True)
class ErrorPattern:
    category: str
    regex: re.Pattern[str]
    hint: str


ERROR_PATTERNS: Sequence[ErrorPattern] = (
    ErrorPattern(
        category="missing-include-or-source",
        regex=re.compile(r"(fatal error|error)\b.*(No such file or directory|Cannot open include file)", re.IGNORECASE),
        hint="检查 include 路径、头文件名称、文件是否纳入工程，以及大小写是否一致。",
    ),
    ErrorPattern(
        category="undeclared-identifier",
        regex=re.compile(r"(was not declared in this scope|undeclared identifier|C2065)", re.IGNORECASE),
        hint="检查命名空间、头文件包含、变量定义顺序，确认符号在当前作用域可见。",
    ),
    ErrorPattern(
        category="type-mismatch-or-signature",
        regex=re.compile(r"(no matching function for call to|cannot convert|invalid conversion|C2664|C2440)", re.IGNORECASE),
        hint="对照函数签名核对参数类型、const/引用修饰及重载选择。",
    ),
    ErrorPattern(
        category="syntax-error",
        regex=re.compile(r"(expected .+ before|expected [;)}]|syntax error|C2143|C2059)", re.IGNORECASE),
        hint="优先检查首个语法报错附近：括号、分号、模板尖括号配对及宏展开。",
    ),
    ErrorPattern(
        category="linker-unresolved-symbol",
        regex=re.compile(r"(undefined reference to|unresolved external symbol|LNK2019|LNK2001)", re.IGNORECASE),
        hint="确认实现文件参与链接、库已链接、函数签名完全一致。",
    ),
    ErrorPattern(
        category="duplicate-symbol",
        regex=re.compile(r"(multiple definition of|already defined in|LNK2005)", re.IGNORECASE),
        hint="排查重复定义：全局变量/非 inline 函数在多个翻译单元重复实现。",
    ),
    ErrorPattern(
        category="qt-moc-uic-rcc",
        regex=re.compile(r"(moc_|uic_|rcc_|undefined reference to `vtable|Q_OBJECT)", re.IGNORECASE),
        hint="检查 Qt 元对象相关：`Q_OBJECT` 后是否重新运行 qmake/cmake，头文件是否在构建系统中。",
    ),
    ErrorPattern(
        category="toolchain-or-command",
        regex=re.compile(r"(is not recognized as an internal or external command|No rule to make target|ninja: error|CMake Error|qmake: could not)", re.IGNORECASE),
        hint="检查构建工具是否安装并在 PATH 中，确认生成步骤和构建目录一致。",
    ),
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Run build command(s), stream output, save logs, and diagnose common failures."
    )
    parser.add_argument("--command", required=True, help="Primary build command to execute.")
    parser.add_argument("--workdir", default=os.getcwd(), help="Working directory for commands.")
    parser.add_argument("--log-path", default="build/logs/latest-build.log", help="Path to save raw logs.")
    parser.add_argument(
        "--report-path",
        default="build/logs/latest-report.md",
        help="Path to save Markdown diagnostic report.",
    )
    parser.add_argument(
        "--test-command",
        default="",
        help="Optional test command executed only when build succeeds.",
    )
    parser.add_argument(
        "--max-evidence",
        type=int,
        default=3,
        help="Maximum evidence lines to keep per category.",
    )
    return parser.parse_args()


def run_command(command: str, workdir: Path, heading: str) -> tuple[int, List[str]]:
    print(heading)
    process = subprocess.Popen(
        command,
        cwd=str(workdir),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        shell=True,
        universal_newlines=True,
        encoding="utf-8",
        errors="replace",
    )

    assert process.stdout is not None
    lines: List[str] = []
    for raw_line in process.stdout:
        line = raw_line.rstrip("\n")
        lines.append(line)
        print(line)

    process.wait()
    return process.returncode, lines


def classify(lines: Sequence[str], max_evidence: int) -> dict[str, dict[str, object]]:
    result: dict[str, dict[str, object]] = {}
    for pattern in ERROR_PATTERNS:
        result[pattern.category] = {"count": 0, "hint": pattern.hint, "evidence": []}

    uncategorized: List[str] = []
    for line in lines:
        matched = False
        for pattern in ERROR_PATTERNS:
            if pattern.regex.search(line):
                entry = result[pattern.category]
                entry["count"] = int(entry["count"]) + 1
                evidence = entry["evidence"]
                assert isinstance(evidence, list)
                if len(evidence) < max_evidence:
                    evidence.append(line.strip())
                matched = True
                break
        if not matched and re.search(r"\b(error|fatal|undefined reference|LNK\d+|FAILED)\b", line, re.IGNORECASE):
            uncategorized.append(line.strip())

    if uncategorized:
        result["uncategorized-error"] = {
            "count": len(uncategorized),
            "hint": "查看完整日志中的首个 error/fatal 位置，补充匹配规则后再重跑分析。",
            "evidence": uncategorized[:max_evidence],
        }
    return result


def write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def build_report(
    build_command: str,
    test_command: str,
    workdir: Path,
    build_exit: int,
    test_exit: int | None,
    final_exit: int,
    log_path: Path,
    summary: dict[str, dict[str, object]],
) -> str:
    now = dt.datetime.now().isoformat(timespec="seconds")
    status = "SUCCESS" if final_exit == 0 else "FAILED"

    lines = [
        "# Build Diagnostic Report",
        "",
        f"- Timestamp: `{now}`",
        f"- Build command: `{build_command}`",
        f"- Workdir: `{workdir}`",
        f"- Final status: `{status}`",
        f"- Build exit code: `{build_exit}`",
    ]

    if test_command:
        lines.append(f"- Test command: `{test_command}`")
        lines.append(f"- Test exit code: `{test_exit}`")

    lines.extend(
        [
            f"- Final exit code: `{final_exit}`",
            f"- Log file: `{log_path}`",
            "",
            "## Error Categories",
        ]
    )

    active_categories = [
        (category, data) for category, data in summary.items() if int(data["count"]) > 0
    ]

    if not active_categories:
        lines.append("- No matched error categories.")
    else:
        for category, data in active_categories:
            count = int(data["count"])
            hint = str(data["hint"])
            lines.append(f"- `{category}`: {count}")
            lines.append(f"  - Hint: {hint}")
            evidence = data["evidence"]
            assert isinstance(evidence, list)
            for item in evidence:
                lines.append(f"  - Evidence: `{item}`")

    lines.extend(
        [
            "",
            "## Suggested Next Step",
            "- Fix the first category above with the earliest evidence line.",
            "- Re-run the same command set and compare report diffs.",
        ]
    )

    return "\n".join(lines) + "\n"


def main() -> int:
    args = parse_args()
    build_command = args.command
    test_command = args.test_command.strip()
    workdir = Path(args.workdir).resolve()
    log_path = Path(args.log_path)
    report_path = Path(args.report_path)

    if not workdir.exists():
        print(f"[ERROR] Workdir not found: {workdir}", file=sys.stderr)
        return 2

    build_exit, build_output = run_command(build_command, workdir, "=== Build Output ===")

    test_exit: int | None = None
    test_output: List[str] = []
    if build_exit == 0 and test_command:
        test_exit, test_output = run_command(test_command, workdir, "=== Test Output ===")

    final_exit = test_exit if test_exit is not None else build_exit

    raw_lines = [
        "=== BUILD ===",
        *build_output,
    ]
    if test_command:
        raw_lines.extend([
            "",
            "=== TEST ===",
            *(test_output if test_output else ["(skipped: build failed)"]),
        ])
    raw_log = "\n".join(raw_lines) + "\n"
    write_text(log_path, raw_log)

    summary = classify(build_output + test_output, max_evidence=args.max_evidence)
    report = build_report(
        build_command=build_command,
        test_command=test_command,
        workdir=workdir,
        build_exit=build_exit,
        test_exit=test_exit,
        final_exit=final_exit,
        log_path=log_path.resolve(),
        summary=summary,
    )
    write_text(report_path, report)

    print("\n=== Build Summary ===")
    print(f"Build exit: {build_exit}")
    if test_command:
        print(f"Test exit: {test_exit}")
    print(f"Final status: {'SUCCESS' if final_exit == 0 else 'FAILED'} (exit={final_exit})")
    print(f"Log: {log_path.resolve()}")
    print(f"Report: {report_path.resolve()}")

    active_categories = [
        (category, data) for category, data in summary.items() if int(data["count"]) > 0
    ]
    if active_categories:
        print("Detected categories:")
        for category, data in active_categories:
            print(f"- {category}: {data['count']}")
    else:
        print("Detected categories: none")

    return final_exit


if __name__ == "__main__":
    raise SystemExit(main())