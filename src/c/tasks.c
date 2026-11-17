#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"

static Task *all_tasks[MAX_TASKS];
static atomic_int task_count = ATOMIC_VAR_INIT(0);

Task *new_task(char *name, int priority, int burst) {
    Task *task = malloc(sizeof *task);
    if (!task) return NULL;
    size_t name_bytes = strlen(name) + 1;
    task->name = malloc(name_bytes);
    if (!task->name) {
        free(task);
        return NULL;
    }
    memcpy(task->name, name, name_bytes);

    /* Reserve a unique slot and ID without exceeding registry capacity. A worker
       publishes into its private slot; report() runs only after workers join. */
    int slot = atomic_load_explicit(&task_count, memory_order_relaxed);
    do {
        if (slot >= MAX_TASKS) {
            free(task->name);
            free(task);
            return NULL;
        }
    } while (!atomic_compare_exchange_weak_explicit(&task_count, &slot, slot + 1,
                                                     memory_order_relaxed, memory_order_relaxed));

    task->tid = slot + 1;
    task->priority = priority;
    task->burst = task->remaining = burst;
    task->start = -1;
    task->finish = 0;
    all_tasks[slot] = task;
    return task;
}

void report(void) {
    int count = atomic_load_explicit(&task_count, memory_order_relaxed);
    if (count == 0) return;

    long long total_waiting = 0, total_turnaround = 0, total_response = 0;
    long long end = 0;
    printf("\n%-8s %4s %4s %6s %9s %11s %9s\n", "Task", "tid", "Pri", "Burst", "Waiting", "Turnaround", "Response");
    for (int i = 0; i < count; i++) {
        Task *task = all_tasks[i];
        /* All tasks arrive at t = 0. */
        long long waiting = task->finish - task->burst;
        printf("%-8s %4d %4d %6d %9lld %11lld %9lld\n",
               task->name, task->tid, task->priority, task->burst, waiting, task->finish, task->start);
        total_waiting += waiting;
        total_turnaround += task->finish;
        total_response += task->start;
        if (task->finish > end) end = task->finish;
    }
    printf("\nAverage waiting time    = %.2f\n", (double)total_waiting / count);
    printf("Average turnaround time = %.2f\n", (double)total_turnaround / count);
    printf("Average response time   = %.2f\n", (double)total_response / count);
    printf("Schedule summary: tasks=%d end=%lld\n", count, end);
}
