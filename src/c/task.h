#ifndef TASK_H
#define TASK_H
#define MIN_PRIORITY 1
#define MAX_PRIORITY 10
#define QUANTUM 10      /* default time quantum (ms), as required by the textbook */
#define MAX_TASKS 1000
#define MAX_BURST 1000
#define MAX_QUANTUM 1000
#define MAX_NAME_BYTES 63
#define MAX_TOTAL_TIME ((long long)MAX_TASKS * MAX_BURST)

typedef struct task {
    char *name;
    int tid;            /* unique registry slot + 1, reserved atomically */
    int priority;       /* 1..10, higher value = higher priority */
    int burst;          /* original CPU burst (ms) */
    int remaining;      /* remaining CPU burst (ms) */
    long long start;    /* time of first dispatch, -1 if not yet run */
    long long finish;   /* completion time */
} Task;

/* Registration supports concurrent callers. Returns NULL if full or out of memory. */
Task *new_task(char *name, int priority, int burst);
/* Call only after all registrations and scheduling have completed. */
void report(void);
#endif
