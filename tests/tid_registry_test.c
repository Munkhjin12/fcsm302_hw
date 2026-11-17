#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"

enum {
    THREADS = 8,
    TASKS_PER_THREAD = MAX_TASKS / THREADS + 1,
    ATTEMPTS = THREADS * TASKS_PER_THREAD
};
_Static_assert(MAX_TASKS % THREADS == 0, "each thread needs an equal workload");

static atomic_bool start_workers = ATOMIC_VAR_INIT(false);
static Task *created[ATTEMPTS];

static void *register_tasks(void *arg) {
    int worker = *(int *)arg;
    while (!atomic_load_explicit(&start_workers, memory_order_acquire)) sched_yield();
    for (int i = 0; i < TASKS_PER_THREAD; i++) {
        int index = worker * TASKS_PER_THREAD + i;
        char name[MAX_NAME_BYTES + 1];
        snprintf(name, sizeof name, "worker-task-%d", index);
        created[index] = new_task(name, MIN_PRIORITY, 1);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2 || !freopen(argv[1], "w+", stdout)) {
        fputs("FAIL: could not open registry report capture\n", stderr);
        return EXIT_FAILURE;
    }
    pthread_t threads[THREADS];
    int workers[THREADS];
    for (int i = 0; i < THREADS; i++) {
        workers[i] = i;
        if (pthread_create(&threads[i], NULL, register_tasks, &workers[i]) != 0) {
            fputs("FAIL: could not start registry test worker\n", stderr);
            return EXIT_FAILURE;
        }
    }
    atomic_store_explicit(&start_workers, true, memory_order_release);
    for (int i = 0; i < THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fputs("FAIL: could not join registry test worker\n", stderr);
            return EXIT_FAILURE;
        }
    }

    bool seen[MAX_TASKS + 1] = {false};
    int accepted = 0;
    for (int i = 0; i < ATTEMPTS; i++) {
        char expected[MAX_NAME_BYTES + 1];
        snprintf(expected, sizeof expected, "worker-task-%d", i);
        Task *task = created[i];
        if (!task) continue;
        if (strcmp(task->name, expected) != 0
            || task->tid < 1 || task->tid > MAX_TASKS || seen[task->tid]) {
            fputs("FAIL: concurrent new_task lost a task or duplicated an ID\n", stderr);
            return EXIT_FAILURE;
        }
        seen[task->tid] = true;
        task->start = 0;
        task->finish = 1;
        task->remaining = 0;
        accepted++;
    }
    if (accepted != MAX_TASKS) {
        fputs("FAIL: concurrent registry capacity is incorrect\n", stderr);
        return EXIT_FAILURE;
    }
    char overflow[] = "additional-task";
    if (new_task(overflow, MIN_PRIORITY, 1) != NULL) {
        fputs("FAIL: registry capacity was exceeded\n", stderr);
        return EXIT_FAILURE;
    }

    /* Exercise the actual registry reader after pthread_join has synchronized
       every worker publication, then check each registered pointer exactly once. */
    report();
    if (fflush(stdout) != 0) return EXIT_FAILURE;
    rewind(stdout);
    bool reported[ATTEMPTS] = {false};
    int rows = 0;
    bool summary = false;
    char line[256];
    while (fgets(line, sizeof line, stdout)) {
        int index, tid, priority, burst;
        long long waiting, turnaround, response;
        if (sscanf(line, "worker-task-%d %d %d %d %lld %lld %lld",
                   &index, &tid, &priority, &burst, &waiting, &turnaround, &response) == 7) {
            if (index < 0 || index >= ATTEMPTS || !created[index] || reported[index]
                || created[index]->tid != tid || priority != MIN_PRIORITY || burst != 1
                || waiting != 0 || turnaround != 1 || response != 0) {
                fputs("FAIL: registry report lost or duplicated a task\n", stderr);
                return EXIT_FAILURE;
            }
            reported[index] = true;
            rows++;
        }
        int count;
        long long end;
        if (sscanf(line, "Schedule summary: tasks=%d end=%lld", &count, &end) == 2)
            summary = count == MAX_TASKS && end == 1;
    }
    if (rows != MAX_TASKS || !summary) {
        fputs("FAIL: registry report is incomplete\n", stderr);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < ATTEMPTS; i++) {
        if (created[i]) {
            free(created[i]->name);
            free(created[i]);
        }
    }
    fprintf(stderr, "PASS: %d concurrent new_task attempts, %d unique registrations and report rows across %d threads\n",
            ATTEMPTS, MAX_TASKS, THREADS);
    return EXIT_SUCCESS;
}
