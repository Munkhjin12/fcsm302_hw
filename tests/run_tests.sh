#!/bin/sh
# Runs every algorithm on the test schedules and compares the average waiting / turnaround /
# response times with expected.txt (computed by the independent JavaScript engine in src/web).
cd "$(dirname "$0")/../src/c" || exit 1
make -s all || exit 1
T=../../tests; pass=0; fail=0
check() { if [ "$1" = "$2" ]; then pass=$((pass+1)); else fail=$((fail+1)); echo "FAIL: $3 (expected '$2', got '$1')"; fi; }
while read file alg q w t r; do
  out=$(./$alg $T/$file $q) || { echo "FAIL: $alg $file crashed"; fail=$((fail+1)); continue; }
  gw=$(echo "$out" | awk '/Average waiting/{print $NF}')
  gt=$(echo "$out" | awk '/Average turnaround/{print $NF}')
  gr=$(echo "$out" | awk '/Average response/{print $NF}')
  check "$gw $gt $gr" "$w $t $r" "$alg $file q=$q (waiting turnaround response)"
done < $T/expected.txt
# invalid inputs must be rejected with a non-zero exit code and a message
for f in bad_priority bad_burst bad_format bad_empty; do
  ./fcfs $T/$f.txt >/dev/null 2>$T/err.tmp; code=$?
  [ $code -ne 0 ] && [ -s $T/err.tmp ] && pass=$((pass+1)) || { fail=$((fail+1)); echo "FAIL: $f.txt was not rejected"; }
done
for q in 0 -3 abc; do
  ./rr $T/schedule.txt $q >/dev/null 2>&1 && { fail=$((fail+1)); echo "FAIL: quantum $q accepted"; } || pass=$((pass+1))
done
./fcfs $T/missing.txt >/dev/null 2>&1 && { fail=$((fail+1)); echo "FAIL: missing file accepted"; } || pass=$((pass+1))
rm -f $T/err.tmp
echo "passed: $pass, failed: $fail"
[ $fail -eq 0 ]
