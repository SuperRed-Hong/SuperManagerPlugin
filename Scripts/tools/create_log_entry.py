#!/usr/bin/env python3
"""Quick helper to create Docs/Logs entries that follow the mandated template."""
from __future__ import annotations

import argparse
import datetime as dt
import pathlib
import re
import sys


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Generate a Docs/Logs entry with the standard sections."
    )
    parser.add_argument(
        "--title",
        required=True,
        help="Short human readable title, e.g. 'Fix asset sync'.",
    )
    parser.add_argument("--issue", default="TODO", help="Problem summary.")
    parser.add_argument("--impact", default="TODO", help="Impacted scope.")
    parser.add_argument("--solution", default="TODO", help="Explain the fix.")
    parser.add_argument(
        "--files",
        default="TODO",
        help="Comma-separated list of touched files or modules.",
    )
    parser.add_argument(
        "--steps",
        action="append",
        help="Implementation steps (can repeat).",
    )
    parser.add_argument(
        "--tests",
        action="append",
        help="Verification steps (can repeat).",
    )
    parser.add_argument("--results", default="TODO", help="Test outcome.")
    parser.add_argument("--risks", default="TODO", help="Known risks.")
    parser.add_argument("--notes", default="TODO", help="Follow-up ideas.")
    parser.add_argument(
        "--problem-reminder",
        dest="problem_reminders",
        action="append",
        help="Problem reminders or user misconceptions you highlighted (can repeat).",
    )
    parser.add_argument(
        "--next-step",
        dest="next_steps",
        action="append",
        help="Next-step suggestions provided to the user (can repeat).",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Preview filename and content without writing.",
    )
    return parser


def slugify(text: str) -> str:
    slug = re.sub(r"[^a-z0-9]+", "_", text.lower()).strip("_")
    return slug or "log"


def ensure_logs_dir(project_root: pathlib.Path) -> pathlib.Path:
    logs_dir = project_root / "Docs" / "Logs"
    logs_dir.mkdir(parents=True, exist_ok=True)
    return logs_dir


def render_template(args: argparse.Namespace) -> str:
    steps = args.steps or ["TODO"]
    tests = args.tests or ["TODO"]
    problem_reminders = args.problem_reminders or ["TODO"]
    next_steps = args.next_steps or ["TODO"]
    steps_lines = "\n".join(f"{idx + 1}. {step}" for idx, step in enumerate(steps))
    tests_lines = "\n".join(f"- {test}" for test in tests)
    reminder_lines = "\n".join(f"- {item}" for item in problem_reminders)
    next_lines = "\n".join(f"- {step}" for step in next_steps)

    sections = (
        f"# {args.title}\n\n"
        f"## 问题描述\n"
        f"- 具体问题：{args.issue}\n"
        f"- 影响范围：{args.impact}\n\n"
        f"## 解决方案\n"
        f"- 技术方案：{args.solution}\n"
        f"- 修改的文件：{args.files}\n\n"
        f"## 实施步骤\n"
        f"{steps_lines}\n\n"
        f"## 测试验证\n"
        f"{tests_lines}\n"
        f"- 测试结果：{args.results}\n\n"
        f"## 注意事项\n"
        f"- 潜在风险：{args.risks}\n"
        f"- 后续优化：{args.notes}\n\n"
        f"## 问题提醒\n"
        f"{reminder_lines}\n\n"
        f"## 下一步建议\n"
        f"{next_lines}\n"
    )
    return sections


def main(argv: list[str]) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    project_root = pathlib.Path(__file__).resolve().parents[2]
    logs_dir = ensure_logs_dir(project_root)

    date_str = dt.datetime.now().strftime("%Y%m%d")
    filename = f"{date_str}_{slugify(args.title)}.md"
    target_path = logs_dir / filename

    content = render_template(args)

    if args.dry_run:
        print(f"[dry-run] would write {target_path}")
        print(content)
        return 0

    if target_path.exists():
        print(f"Error: {target_path} already exists", file=sys.stderr)
        return 1

    target_path.write_text(content, encoding="utf-8")
    print(f"Created {target_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
