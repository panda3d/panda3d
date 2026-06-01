#!/usr/bin/env python3
"""Runs the scenegraph_stress binary N times and summarizes pass / deadlock
/ crash / skipped counts.  Used as the ctest command so the binary itself
stays focused on running one scenario.

Exit codes match the binary's:
  0   every run passed (or all were skipped)
  1   any run crashed or any run hit the watchdog deadlock
 77   every run skipped (no graphics pipe available)
"""

import argparse
import subprocess
import sys


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--binary", required=True,
                   help="Path to the scenegraph_stress executable.")
    p.add_argument("--runs", type=int, default=1,
                   help="Number of independent scenario runs (default: 1).")
    p.add_argument("scenario_args", nargs=argparse.REMAINDER,
                   help="Arguments forwarded to each scenario invocation.  "
                        "Prefix with '--' so argparse doesn't try to consume them.")
    args = p.parse_args()

    # argparse keeps the leading "--" in REMAINDER; strip it.
    scenario_args = args.scenario_args
    if scenario_args and scenario_args[0] == "--":
        scenario_args = scenario_args[1:]

    counts = {"pass": 0, "deadlock": 0, "crash": 0, "skipped": 0}
    for r in range(1, args.runs + 1):
        if args.runs > 1:
            print(f"[stress] === run {r} of {args.runs} ===",
                  file=sys.stderr, flush=True)
        rc = subprocess.run([args.binary] + scenario_args).returncode
        if rc == 0:    counts["pass"] += 1
        elif rc == 2:  counts["deadlock"] += 1
        elif rc == 77: counts["skipped"] += 1
        else:          counts["crash"] += 1

    if args.runs > 1:
        print(f"[stress] SUMMARY  runs={args.runs}  "
              f"pass={counts['pass']}  deadlock={counts['deadlock']}  "
              f"crash={counts['crash']}  skipped={counts['skipped']}",
              file=sys.stderr)

    if counts["skipped"] == args.runs:
        sys.exit(77)
    if counts["deadlock"] or counts["crash"]:
        sys.exit(1)
    sys.exit(0)


if __name__ == "__main__":
    main()
