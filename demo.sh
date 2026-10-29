#!/bin/sh
# Видео бичлэгт зориулсан туслах скрипт. Terminal дээр:  sh demo.sh
# Алхам бүрийн дараа Enter дарж үргэлжлүүлнэ (тайлбараа ярьж дуусаад дарна).
cd "$(dirname "$0")/src/c" || exit 1

step() { printf '\n\033[1;36m# %s\033[0m\n' "$1"; printf '(Enter дарна)'; read _x; }
run()  { printf '\033[1m$ %s\033[0m\n' "$1"; sleep 1; sh -c "$1"; }

make clean >/dev/null 2>&1; clear
step "1. Build: таван алгоритм"
run "make"
step "2. FCFS"
run "./fcfs ../../tests/schedule.txt"
step "3. SJF"
run "./sjf ../../tests/schedule.txt"
step "4. Priority"
run "./priority ../../tests/schedule.txt"
step "5. Round Robin, quantum = 4"
run "./rr ../../tests/schedule.txt 4"
step "6. Priority + Round Robin, quantum = 10"
run "./priority_rr ../../tests/schedule.txt"
step "7. Буруу оролт: priority 11"
run "./fcfs ../../tests/bad_priority.txt"
step "8. Буруу quantum = 0"
run "./rr ../../tests/schedule.txt 0"
step "9. Автомат тест"
run "make test"
printf '\n\033[1;32mДууслаа.\033[0m\n'
