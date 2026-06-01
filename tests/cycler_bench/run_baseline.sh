#!/bin/bash
# Run cycler_bench N times and emit a Markdown table of medians.
# Usage: run_baseline.sh <binary> <runs> <scale> [extra cycler_bench args...]
#   binary  path to cycler_bench executable
#   runs    number of runs to take the median of (default 3)
#   scale   --scale value to pass to cycler_bench  (default 1)
#   extra   any further args are forwarded verbatim to cycler_bench
#           (e.g. --reader-threads 3 --reader-stage 1)
set -euo pipefail
BIN=${1:-$(dirname "$0")/../../cmake-build-debug/bin/cycler_bench}
RUNS=${2:-3}
SCALE=${3:-1}
shift $(( $# < 3 ? $# : 3 ))
EXTRA=("$@")

if [[ ! -x "$BIN" ]]; then
  echo "binary not executable: $BIN" >&2
  exit 1
fi

LIBDIR=$(cd "$(dirname "$BIN")/../lib" && pwd)
export LD_LIBRARY_PATH="$LIBDIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

for i in $(seq 1 "$RUNS"); do
  "$BIN" --scale "$SCALE" "${EXTRA[@]}" > "$TMP/run_$i.txt"
done

python3 - "$TMP" "$RUNS" <<'PY'
import sys, os, statistics
tmp, runs = sys.argv[1], int(sys.argv[2])
runs_data = []
for i in range(1, runs + 1):
    with open(os.path.join(tmp, f"run_{i}.txt")) as f:
        runs_data.append([l.rstrip() for l in f])

print("| workload                       | depth | branch |     iters | ns/op   |")
print("|--------------------------------|------:|-------:|----------:|--------:|")
for line_idx, l in enumerate(runs_data[0]):
    if "|" not in l or l.startswith("|---") or "workload" in l.lower():
        continue
    cells = [c.strip() for c in l.strip().strip("|").split("|")]
    if len(cells) < 5:
        continue
    name, depth, branch, iters, _ = cells
    nums = []
    for r in runs_data:
        nums.append(float(r[line_idx].strip().strip("|").split("|")[4].strip()))
    med = statistics.median(nums)
    print(f"| {name:<30} | {depth:>5} | {branch:>6} | {iters:>9} | {med:7.1f} |")
PY
