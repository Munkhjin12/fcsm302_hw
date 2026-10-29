/* Priority scheduling: highest priority value first (ties: earliest added). */
#include <stdlib.h>
#include "schedulers.h"
#include "list.h"
#include "CPU.h"
static struct node *head = NULL;
void add(char *name, int priority, int burst) { insert(&head, new_task(name, priority, burst)); }
Task *pickNextTask(void) {
    Task *best = NULL;
    for (struct node *p = head; p; p = p->next)
        if (!best || p->task->priority > best->priority) best = p->task;
    return best;
}
void schedule(void) {
    Task *t;
    while ((t = pickNextTask())) { run(t, t->remaining); delete(&head, t); }
}
