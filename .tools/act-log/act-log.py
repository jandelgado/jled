#!/usr/bin/env python3
"""Analyse act NDJSON logs stored in .act-logs/.

Usage:
    act-log.py report          # summary table for the last run
    act-log.py report <unit>   # full log for one unit (see below)

A "unit" is one row of the summary table: a matrix board of the 'examples' job
(e.g. 'uno', 'nucleo_f401re_mbed'), or a jobID for any job without a board.
"""

import argparse
import json
import sys
from pathlib import Path

LOGS_DIR = Path(__file__).resolve().parents[2] / ".act-logs"
MAIN_LOG = LOGS_DIR / "act.ndjson"

# Groups emitted by setup actions (e.g. actions/setup-python) — not example names
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


def unit_of(entry: dict) -> str | None:
    """The summary row an entry belongs to.

    Matrix jobs fan out per board, so the board name identifies the unit.
    Non-matrix jobs (the single-build examples) have no board, so the jobID
    identifies the unit. Entries with neither (should not happen) are ignored.
    """
    board = (entry.get("matrix") or {}).get("board")
    return board or entry.get("jobID")


def split_per_job(entries: list[dict]) -> None:
    """Write one NDJSON file per unit from the full act log."""
    per_unit: dict[str, list[dict]] = {}
    for entry in entries:
        unit = unit_of(entry)
        if unit:
            per_unit.setdefault(unit, []).append(entry)
    for unit, unit_entries in per_unit.items():
        out = LOGS_DIR / f"{unit}.ndjson"
        with out.open("w") as f:
            for e in unit_entries:
                f.write(json.dumps(e) + "\n")


def build_summary(entries: list[dict]) -> dict[tuple[str, str], str]:
    """Return {(unit, example): 'OK'|'FAIL'|'INFRA'}.

    Two kinds of unit are reported uniformly, without hardcoding any job or
    example name:

    * Matrix boards build many examples per job, each wrapped in a ::group::
      workflow command. Every group becomes a row; a ::error:: inside it marks
      it FAIL. A matrix board that ran but emitted no build groups is INFRA
      (it never reached the build step, i.e. an act issue, not a code bug).
    * Single-build jobs emit no build groups, so the whole job is one row whose
      status comes from step results: FAIL if any step failed, else OK. A job
      scheduled but producing no step result at all is INFRA.
    """
    # per-unit tracking while scanning the interleaved parallel log
    current: dict[str, str] = {}       # unit -> active group (example) name
    group_failed: dict[str, bool] = {}  # unit -> whether current group errored
    grouped: set[str] = set()          # units that produced build groups
    is_matrix: dict[str, bool] = {}    # unit -> came from a board matrix
    step_failed: dict[str, bool] = {}  # unit -> any step failed (non-grouped)
    results: dict[tuple[str, str], str] = {}

    for entry in entries:
        unit = unit_of(entry)
        if not unit:
            continue
        is_matrix[unit] = is_matrix.get(unit, False) or \
            bool((entry.get("matrix") or {}).get("board"))

        result = entry.get("stepResult")
        if result == "failure":
            step_failed[unit] = True
        elif result is not None:
            step_failed.setdefault(unit, False)

        cmd = entry.get("command")
        arg = entry.get("arg", "")
        if cmd == "group" and arg and arg not in _SKIP_GROUPS:
            current[unit] = arg
            group_failed[unit] = False
        elif cmd == "error" and unit in current:
            group_failed[unit] = True
        elif cmd == "endgroup" and current.get(unit):
            example = current.pop(unit)
            results[(unit, example)] = "FAIL" if group_failed.pop(unit, False) else "OK"
            grouped.add(unit)

    # units without build groups collapse to a single status row
    for unit in is_matrix:
        if unit in grouped:
            continue
        if is_matrix[unit] or unit not in step_failed:
            results[(unit, "")] = "INFRA"
        else:
            results[(unit, "")] = "FAIL" if step_failed[unit] else "OK"

    return results


def print_summary(summary: dict[tuple[str, str], str]) -> bool:
    """Print the summary table. Returns True if any failures exist."""
    if not summary:
        print("No results found.", file=sys.stderr)
        return True

    unit_w = max(len(u) for u, _ in summary) + 2
    has_failures = False

    for (unit, example), status in sorted(summary.items()):
        if example:
            label = example
        elif status == "INFRA":
            label = "(infra)"
        else:
            label = "-"
        print(f"{status:<6} {unit:<{unit_w}} {label}")
        if status in ("FAIL", "INFRA"):
            has_failures = True

    return has_failures


def cmd_report(unit: str | None) -> None:
    if not MAIN_LOG.exists():
        sys.exit(f"No act log at {MAIN_LOG} — run 'make ci-act' first.")

    entries = list(iter_ndjson(MAIN_LOG))

    if unit:
        unit_log = LOGS_DIR / f"{unit}.ndjson"
        if not unit_log.exists():
            split_per_job(entries)
        if not unit_log.exists():
            sys.exit(f"No log for unit '{unit}'.")
        for entry in iter_ndjson(unit_log):
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
    report = sub.add_parser("report", help="Print summary or per-unit log")
    report.add_argument("unit", nargs="?",
                        help="Board or jobID (e.g. uno, nucleo_f401re_mbed)")

    args = parser.parse_args()
    if args.command == "report":
        cmd_report(args.unit)
    else:
        parser.print_help()
        sys.exit(1)


if __name__ == "__main__":
    main()
