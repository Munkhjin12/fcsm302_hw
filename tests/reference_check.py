#!/usr/bin/env python3
"""Independent reference model of the five schedulers (written separately from the C code).
Runs every ./src/c/<alg> binary on both test files (quantum 10 and 4) and compares the averages.
Usage:  make -C src/c && python3 tests/reference_check.py"""
import subprocess, sys, os
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

def ref(alg, tasks, q=10):
    T = [dict(n=n, p=p, b=b, r=b, s=None, f=None) for n, p, b in tasks]
    L, now = list(T), 0
    while L:
        if alg in ('fcfs', 'rr'):
            t = L[0]
        elif alg == 'sjf':
            t = min(L, key=lambda x: x['b'])                        # ties: earliest in list
        else:                                                        # priority, priority_rr
            t = max(L, key=lambda x: (x['p'], -L.index(x)))          # ties: earliest in list
        sl = t['r'] if alg in ('fcfs', 'sjf', 'priority') else min(q, t['r'])
        if t['s'] is None:
            t['s'] = now
        now += sl; t['r'] -= sl; L.remove(t)
        if t['r'] > 0: L.append(t)
        else: t['f'] = now
    n = len(T)
    return (round(sum(x['f'] - x['b'] for x in T) / n, 2),
            round(sum(x['f'] for x in T) / n, 2),
            round(sum(x['s'] for x in T) / n, 2))

SETS = {'schedule.txt': [('T1', 4, 20), ('T2', 2, 25), ('T3', 3, 25), ('T4', 3, 15), ('T5', 10, 10)],
        'schedule_ties.txt': [('A', 5, 12), ('B', 5, 7), ('C', 5, 3), ('D', 1, 9)]}
bad = 0
for f, tasks in SETS.items():
    for alg in ('fcfs', 'sjf', 'priority', 'rr', 'priority_rr'):
        for q in (10, 4):
            cmd = [os.path.join(ROOT, 'src/c', alg), os.path.join(ROOT, 'tests', f)] + ([str(q)] if q != 10 else [])
            out = subprocess.run(cmd, capture_output=True, text=True).stdout
            got = tuple(float(l.split('=')[1]) for l in out.splitlines() if l.startswith('Average'))
            ok = got == ref(alg, tasks, q)
            bad += not ok
            print(f"{'OK  ' if ok else 'DIFF'} {f:18} {alg:12} q={q:<3} C={got} ref={ref(alg, tasks, q)}")
print('mismatches:', bad)
sys.exit(1 if bad else 0)
