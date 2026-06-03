#!/usr/bin/env python3
"""Analyse act NDJSON logs stored in .act-logs/.

Usage:
    act-log.py report                   # summary table for the last run
    act-log.py report <board> <example> # full log for one board/example job
"""

import argparse
import json
import sys
from pathlib import Path

LOGS_DIR = Path(__file__).resolve().parents[2] / ".act-logs"
MAIN_LOG = LOGS_DIR / "act.ndjson"


def iter_ndjson(path: Path):
    with path.open() as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                yield json.loads(line)
            except json.JSONDecodeError:
                pass


def split_per_job(entries: list[dict]) -> None:
    """Write per-job NDJSON files from the full act log."""
    per_job: dict[str, list[dict]] = {}
    for entry in entries:
        matrix = entry.get("matrix") or {}
        board = matrix.get("board")
        example = matrix.get("example")
        if board and example:
            per_job.setdefault(f"{board}_{example}", []).append(entry)
    for key, job_entries in per_job.items():
        out = LOGS_DIR / f"{key}.ndjson"
        with out.open("w") as f:
            for e in job_entries:
                f.write(json.dumps(e) + "\n")


def build_summary(entries: list[dict]) -> dict[tuple[str, str], str]:
    build_results: dict[tuple[str, str], str] = {}
    job_seen: set[tuple[str, str]] = set()

    for entry in entries:
        if entry.get("jobID") != "examples":
            continue
        matrix = entry.get("matrix") or {}
        board = matrix.get("board")
        example = matrix.get("example")
        if not board or not example:
            continue
        key = (board, example)

        if entry.get("step") == "build examples" and entry.get("stepResult"):
            build_results[key] = entry["stepResult"]

        if entry.get("jobResult") is not None:
            job_seen.add(key)

    summary: dict[tuple[str, str], str] = {}
    for key in job_seen:
        if key in build_results:
            summary[key] = "OK" if build_results[key] == "success" else "FAIL"
        else:
            summary[key] = "INFRA"
    return summary


def print_summary(summary: dict[tuple[str, str], str]) -> bool:
    """Print the summary table. Returns True if any failures exist."""
    if not summary:
        print("No results found.", file=sys.stderr)
        return True

    board_w = max(len(b) for b, _ in summary) + 2
    example_w = max(len(e) for _, e in summary) + 2
    has_failures = False

    for (board, example), status in sorted(summary.items()):
        print(f"{status:<6} {board:<{board_w}} {example}")
        if status == "FAIL":
            has_failures = True

    return has_failures


def cmd_report(board: str | None, example: str | None) -> None:
    if not MAIN_LOG.exists():
        sys.exit(f"No act log at {MAIN_LOG} — run 'make ci-act' first.")

    entries = list(iter_ndjson(MAIN_LOG))

    if board and example:
        job_log = LOGS_DIR / f"{board}_{example}.ndjson"
        if not job_log.exists():
            split_per_job(entries)
        if not job_log.exists():
            sys.exit(f"No log for {board}/{example}.")
        for entry in iter_ndjson(job_log):
            msg = entry.get("msg", "")
            if msg:
                print(msg.rstrip("\n"))
        return

    split_per_job(entries)
    summary = build_summary(entries)
    has_failures = print_summary(summary)
    if has_failures:
        sys.exit(1)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = parser.add_subparsers(dest="command")
    report = sub.add_parser("report", help="Print summary or per-job log")
    report.add_argument("board", nargs="?", help="Board name (e.g. uno)")
    report.add_argument("example", nargs="?", help="Example name (e.g. hello)")

    args = parser.parse_args()
    if args.command == "report":
        if bool(args.board) != bool(args.example):
            parser.error("Provide both <board> and <example>, or neither.")
        cmd_report(args.board, args.example)
    else:
        parser.print_help()
        sys.exit(1)


if __name__ == "__main__":
    main()
