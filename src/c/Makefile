# Usage:  make fcfs   (or sjf, priority, rr, priority_rr, or just: make)
#         ./fcfs ../../tests/schedule.txt [quantum]
CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11 -Wpedantic
ALGS    = fcfs sjf priority rr priority_rr
COMMON  = driver.c list.c CPU.c tasks.c
HDRS    = task.h list.h CPU.h schedulers.h

all: $(ALGS)

$(ALGS): %: %.c $(COMMON) $(HDRS)
	$(CC) $(CFLAGS) -o $@ $< $(COMMON)

test: all test_tid
	sh ../../tests/run_tests.sh
	sh ../../tests/input_limits_test.sh
	sh ../../tests/task_metrics_test.sh

test_tid:
	@work=$$(mktemp -d) || exit 1; \
	trap 'rm -rf "$$work"' EXIT HUP INT TERM; \
	$(CC) $(CFLAGS) -pthread -I. -o "$$work/tid_registry_test" ../../tests/tid_registry_test.c tasks.c && \
	"$$work/tid_registry_test" "$$work/report.txt"

clean:
	rm -f $(ALGS)

.PHONY: all test test_tid clean
