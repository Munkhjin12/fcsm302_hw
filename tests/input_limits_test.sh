#!/bin/sh
set -eu
cd "$(dirname "$0")/../src/c"
make -s all
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT TERM

expect_rejection() {
  if ./fcfs "$1" >"$work/out" 2>"$work/err"; then
    echo "FAIL: $1 accepted" >&2
    exit 1
  fi
  if ! grep -Fq "$2" "$work/err" || [ -s "$work/out" ]; then
    echo "FAIL: expected '$2' and no schedule for $1" >&2
    cat "$work/err" >&2
    exit 1
  fi
}

# One task, maximum burst and quantum, CRLF, and a final line without LF.
printf '# input comment\r\n\r\nA, 1, 1000' > "$work/bounds.txt"
./rr "$work/bounds.txt" 1000 > "$work/out"
grep -Fxq 'Schedule summary: tasks=1 end=1000' "$work/out"
grep -Fxq 'Running task = [A] [1] [1000] for 1000 units.  (t = 0..1000)' "$work/out"

printf 'A, 1, 1001\n' > "$work/too_large.txt"
expect_rejection "$work/too_large.txt" 'burst must be in 1..1000'
printf 'A, 1, 10\n  A  , 2, 5\n' > "$work/duplicate.txt"
expect_rejection "$work/duplicate.txt" 'duplicate task name'

# Verify the byte limit with a multibyte name, including the accepted edge.
test_name=$(awk 'BEGIN { for (i = 0; i < 31; i++) printf "Ж"; printf "A" }')
printf '%s, 1, 1\n' "$test_name" > "$work/short_name.txt"
./fcfs "$work/short_name.txt" > "$work/out"
grep -Fxq 'Schedule summary: tasks=1 end=1' "$work/out"
printf '%sZ, 1, 1\n' "$test_name" > "$work/long_name.txt"
expect_rejection "$work/long_name.txt" 'task name is longer than 63 bytes'
printf 'A[B], 1, 2\n' > "$work/bracket.txt"
expect_rejection "$work/bracket.txt" 'task name cannot contain brackets or control characters'
printf 'A\001B, 1, 2\n' > "$work/control.txt"
expect_rejection "$work/control.txt" 'task name cannot contain brackets or control characters'
printf 'A, 1, 2\000trailing\n' > "$work/nul.txt"
expect_rejection "$work/nul.txt" 'NUL byte in input'

printf 'A, 999999999999999999999999999999, 1\n' > "$work/priority_overflow.txt"
expect_rejection "$work/priority_overflow.txt" 'priority must be an integer in 1..10'
printf 'A, 1, 999999999999999999999999999999\n' > "$work/burst_overflow.txt"
expect_rejection "$work/burst_overflow.txt" 'burst must be in 1..1000'
printf 'A, 1, 1\n' > "$work/single.txt"
for quantum in '' 1001 999999999999999999999999999999; do
  if ./rr "$work/single.txt" "$quantum" >"$work/out" 2>"$work/err"; then
    echo 'FAIL: invalid or overflowing quantum accepted' >&2
    exit 1
  fi
  grep -Fq 'quantum must be an integer from 1 to 1000' "$work/err"
done

awk 'BEGIN { printf "A,1,1"; for (i = 0; i < 250; i++) printf " "; printf "\n" }' > "$work/max_line.txt"
./fcfs "$work/max_line.txt" > "$work/out"
grep -Fxq 'Schedule summary: tasks=1 end=1' "$work/out"
awk 'BEGIN { printf "A, 1, 1"; for (i = 0; i < 255; i++) printf " "; print "trailing" }' > "$work/long_line.txt"
expect_rejection "$work/long_line.txt" 'line is longer than 255 bytes'
awk 'BEGIN { for (i = 1; i <= 1000; i++) printf "T%d, 1, 1000\n", i }' > "$work/capacity.txt"
./fcfs "$work/capacity.txt" > "$work/out"
grep -Fxq 'Schedule summary: tasks=1000 end=1000000' "$work/out"
printf 'T1001, 1, 1000\n' >> "$work/capacity.txt"
expect_rejection "$work/capacity.txt" 'more than 1000 tasks'
echo 'PASS: C input bounds, duplicate-name policy and maximum total time'
