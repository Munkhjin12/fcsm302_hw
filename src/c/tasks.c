#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "task.h"

static Task *all_tasks[MAX_TASKS];
static int task_count = 0;
static int next_tid = 1;

Task *new_task(char *name, int priority, int burst) {
    Task *t = malloc(sizeof *t);
    t->name = strdup(name);
    t->tid = __sync_fetch_and_add(&next_tid, 1);   /* atomic increment: no race on tid */
    t->priority = priority;
    t->burst = t->remaining = burst;
    t->start = -1;
    t->finish = 0;
    all_tasks[task_count++] = t;                   /* single-threaded driver: registry needs no lock */
    return t;
}

void report(void) {
    double tw = 0, tt = 0, tr = 0;
    printf("\n%-8s %4s %4s %6s %9s %11s %9s\n", "Task", "tid", "Pri", "Burst", "Waiting", "Turnaround", "Response");
    for (int i = 0; i < task_count; i++) {
        Task *t = all_tasks[i];
        int tat = t->finish, wait = tat - t->burst, resp = t->start;   /* arrival = 0 */
        printf("%-8s %4d %4d %6d %9d %11d %9d\n", t->name, t->tid, t->priority, t->burst, wait, tat, resp);
        tw += wait; tt += tat; tr += resp;
    }
    if (task_count == 0) return;
    printf("\nAverage waiting time    = %.2f\n", tw / task_count);
    printf("Average turnaround time = %.2f\n", tt / task_count);
    printf("Average response time   = %.2f\n", tr / task_count);
}
