#!/usr/bin/env python3
"""Analyse act NDJSON logs stored in .act-logs/.

Usage:
    act-log.py report           # summary table for the last run
    act-log.py report <board>   # full log for one board job
"""

import argparse
import json
import sys
from pathlib import Path

LOGS_DIR = Path(__file__).resolve().parents[2] / ".act-logs"
MAIN_LOG = LOGS_DIR / "act.ndjson"

# Groups emitted by setup steps — not example names
_SKIP_GROUPS = {"Installed versions"}


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
    """Write per-board NDJSON files from the full act log."""
    per_board: dict[str, list[dict]] = {}
    for entry in entries:
        board = (entry.get("matrix") or {}).get("board")
        if board:
            per_board.setdefault(board, []).append(entry)
    for board, job_entries in per_board.items():
        out = LOGS_DIR / f"{board}.ndjson"
        with out.open("w") as f:
            for e in job_entries:
                f.write(json.dumps(e) + "\n")


def build_summary(entries: list[dict]) -> dict[tuple[str, str], str]:
    """Return {(board, example): 'OK'|'FAIL'|'INFRA'}.

    Examples are discovered from ::group:: workflow commands emitted by the
    build loop inside the 'build examples' step.  Boards whose job never
    reached that step are reported as INFRA.
    """
    # per-board tracking while scanning the interleaved parallel log
    current: dict[str, str] = {}   # board -> active example name
    failed:  dict[str, bool] = {}  # board -> whether current example errored
    results: dict[tuple[str, str], str] = {}
    job_seen: set[str] = set()

    for entry in entries:
        if entry.get("jobID") != "examples":
            continue
        board = (entry.get("matrix") or {}).get("board")
        if not board:
            continue

        if entry.get("jobResult") is not None:
            job_seen.add(board)

        if entry.get("step") != "build examples":
            continue

        cmd = entry.get("command")
        arg = entry.get("arg", "")

        if cmd == "group" and arg and arg not in _SKIP_GROUPS:
            current[board] = arg
            failed[board] = False
        elif cmd == "error" and board in current:
            failed[board] = True
        elif cmd == "endgroup" and current.get(board):
            example = current.pop(board)
            results[(board, example)] = "FAIL" if failed.pop(board, False) else "OK"

    # boards whose job ran but produced no example results → INFRA
    for board in job_seen:
        if not any(b == board for b, _ in results):
            results[(board, "")] = "INFRA"

    return results


def print_summary(summary: dict[tuple[str, str], str]) -> bool:
    """Print the summary table. Returns True if any failures exist."""
    if not summary:
        print("No results found.", file=sys.stderr)
        return True

    board_w = max(len(b) for b, _ in summary) + 2
    example_w = max((len(e) for _, e in summary if e), default=0) + 2
    has_failures = False

    for (board, example), status in sorted(summary.items()):
        label = example if example else "(infra)"
        print(f"{status:<6} {board:<{board_w}} {label}")
        if status in ("FAIL", "INFRA"):
            has_failures = True

    return has_failures


def cmd_report(board: str | None) -> None:
    if not MAIN_LOG.exists():
        sys.exit(f"No act log at {MAIN_LOG} — run 'make ci-act' first.")

    entries = list(iter_ndjson(MAIN_LOG))

    if board:
        job_log = LOGS_DIR / f"{board}.ndjson"
        if not job_log.exists():
            split_per_job(entries)
        if not job_log.exists():
            sys.exit(f"No log for board '{board}'.")
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
    report = sub.add_parser("report", help="Print summary or per-board log")
    report.add_argument("board", nargs="?", help="Board name (e.g. uno)")

    args = parser.parse_args()
    if args.command == "report":
        cmd_report(args.board)
    else:
        parser.print_help()
        sys.exit(1)


if __name__ == "__main__":
    main()
