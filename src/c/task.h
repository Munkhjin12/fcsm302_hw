#ifndef TASK_H
#define TASK_H
#define MIN_PRIORITY 1
#define MAX_PRIORITY 10
#define QUANTUM 10      /* default time quantum (ms), as required by the textbook */
#define MAX_TASKS 1000

typedef struct task {
    char *name;
    int tid;            /* unique id, assigned atomically (Further Challenge 1) */
    int priority;       /* 1..10, higher value = higher priority */
    int burst;          /* original CPU burst (ms) */
    int remaining;      /* remaining CPU burst (ms) */
    int start;          /* time of first dispatch, -1 if not yet run */
    int finish;         /* completion time */
} Task;

Task *new_task(char *name, int priority, int burst);
void report(void);      /* prints waiting / turnaround / response times (Further Challenge 2) */
#endif
