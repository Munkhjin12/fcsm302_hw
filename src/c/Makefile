# Usage:  make fcfs   (or sjf, priority, rr, priority_rr, or just: make)
#         ./fcfs ../../tests/schedule.txt [quantum]
CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=gnu11
ALGS    = fcfs sjf priority rr priority_rr
COMMON  = driver.c list.c CPU.c tasks.c
HDRS    = task.h list.h CPU.h schedulers.h

all: $(ALGS)

$(ALGS): %: %.c $(COMMON) $(HDRS)
	$(CC) $(CFLAGS) -o $@ $< $(COMMON)

test: all
	sh ../../tests/run_tests.sh

clean:
	rm -f $(ALGS)

.PHONY: all test clean
