#!/usr/bin/env python3
"""Lightweight CI helper to enforce AGENTS.md guardrails."""
from __future__ import annotations

import argparse
import pathlib
import subprocess
import sys
from typing import Iterable

DOXYGEN_KEYWORDS = ("UCLASS", "USTRUCT", "UENUM", "UFUNCTION")
BLUEPRINT_PROPERTY_TOKENS = ("BlueprintCallable", "BlueprintPure", "BlueprintSetter", "BlueprintGetter", "BlueprintReadWrite")


def iter_code_files() -> Iterable[pathlib.Path]:
    project_root = pathlib.Path(__file__).resolve().parents[2]
    target = project_root / "Plugins" / "SuperManager" / "Source" / "SuperManager"
    if not target.exists():
        return []
    for ext in ("*.h", "*.hpp", "*.cpp"):
        yield from target.rglob(ext)


def has_doxygen(lines: list[str], index: int) -> bool:
    j = index - 1
    while j >= 0 and not lines[j].strip():
        j -= 1
    if j < 0:
        return False
    stripped = lines[j].lstrip()
    return stripped.startswith("/**")


def check_public_interface_comments(path: pathlib.Path) -> list[str]:
    warnings: list[str] = []
    try:
        text = path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return warnings
    lines = text.splitlines()
    for idx, line in enumerate(lines):
        line_stripped = line.strip()
        needs_comment = False
        if any(token in line_stripped for token in DOXYGEN_KEYWORDS):
            needs_comment = True
        elif "UPROPERTY" in line_stripped and any(token in line_stripped for token in BLUEPRINT_PROPERTY_TOKENS):
            needs_comment = True
        if needs_comment and not has_doxygen(lines, idx):
            warnings.append(f"{path}:{idx + 1} 缺少 Doxygen 注释")
    return warnings


def check_docs_uesource_clean(project_root: pathlib.Path) -> list[str]:
    warnings: list[str] = []
    git_dir = project_root / ".git"
    if not git_dir.exists():
        return warnings
    try:
        result = subprocess.run(
            ["git", "status", "--porcelain", "--", "Docs/UESource"],
            capture_output=True,
            text=True,
            check=False,
            cwd=project_root,
        )
    except FileNotFoundError:
        return warnings
    if result.stdout.strip():
        warnings.append("Docs/UESource 存在被修改的文件，违反不可变约束")
    return warnings


def run_checks() -> int:
    project_root = pathlib.Path(__file__).resolve().parents[2]
    warnings: list[str] = []

    for path in iter_code_files():
        warnings.extend(check_public_interface_comments(path))

    warnings.extend(check_docs_uesource_clean(project_root))

    if warnings:
        for message in warnings:
            print(f"[AGENTS] {message}")
        return 1

    print("[AGENTS] 所有检查通过")
    return 0


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description="Validate AGENTS guardrails")
    parser.parse_args(argv)
    return run_checks()


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
