#!/bin/sh
# Hand-calculated traces and metrics for an identical-priority, identical-burst tie.
set -eu
cd "$(dirname "$0")/../src/c"
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT TERM

cat > "$work/schedule.txt" <<'SCHEDULE'
X, 5, 3
Y, 5, 3
Z, 1, 1
SCHEDULE

check_algorithm() {
  "./$1" "$work/schedule.txt" "$2" > "$work/out"
  awk '/^Running task = / { print; next }
       /^Task +tid / { metrics = 1; next }
       metrics && /^$/ { metrics = 0 }
       metrics { $1 = $1; print }
       /^Schedule summary: / { print }' "$work/out" > "$work/actual"
  if ! diff -u "$work/expected" "$work/actual"; then
    echo "FAIL: $1 q=$2 exact trace and per-task metrics" >&2
    exit 1
  fi
}
# Rows have name, TID, priority, burst, waiting, turnaround, response.
cat > "$work/expected" <<'EXPECTED'
Running task = [X] [5] [3] for 3 units.  (t = 0..3)
Running task = [Y] [5] [3] for 3 units.  (t = 3..6)
Running task = [Z] [1] [1] for 1 units.  (t = 6..7)
X 1 5 3 0 3 0
Y 2 5 3 3 6 3
Z 3 1 1 6 7 6
Schedule summary: tasks=3 end=7
EXPECTED
check_algorithm fcfs 10
check_algorithm priority 10
check_algorithm rr 1000
check_algorithm priority_rr 1000

cat > "$work/expected" <<'EXPECTED'
Running task = [Z] [1] [1] for 1 units.  (t = 0..1)
Running task = [X] [5] [3] for 3 units.  (t = 1..4)
Running task = [Y] [5] [3] for 3 units.  (t = 4..7)
X 1 5 3 1 4 1
Y 2 5 3 4 7 4
Z 3 1 1 0 1 0
Schedule summary: tasks=3 end=7
EXPECTED
check_algorithm sjf 10

cat > "$work/expected" <<'EXPECTED'
Running task = [X] [5] [3] for 1 units.  (t = 0..1)
Running task = [Y] [5] [3] for 1 units.  (t = 1..2)
Running task = [Z] [1] [1] for 1 units.  (t = 2..3)
Running task = [X] [5] [2] for 1 units.  (t = 3..4)
Running task = [Y] [5] [2] for 1 units.  (t = 4..5)
Running task = [X] [5] [1] for 1 units.  (t = 5..6)
Running task = [Y] [5] [1] for 1 units.  (t = 6..7)
X 1 5 3 3 6 0
Y 2 5 3 4 7 1
Z 3 1 1 2 3 2
Schedule summary: tasks=3 end=7
EXPECTED
check_algorithm rr 1

cat > "$work/expected" <<'EXPECTED'
Running task = [X] [5] [3] for 1 units.  (t = 0..1)
Running task = [Y] [5] [3] for 1 units.  (t = 1..2)
Running task = [X] [5] [2] for 1 units.  (t = 2..3)
Running task = [Y] [5] [2] for 1 units.  (t = 3..4)
Running task = [X] [5] [1] for 1 units.  (t = 4..5)
Running task = [Y] [5] [1] for 1 units.  (t = 5..6)
Running task = [Z] [1] [1] for 1 units.  (t = 6..7)
X 1 5 3 2 5 0
Y 2 5 3 3 6 1
Z 3 1 1 6 7 6
Schedule summary: tasks=3 end=7
EXPECTED
check_algorithm priority_rr 1
echo 'PASS: 7 hand-calculated traces and per-task metric tables'
