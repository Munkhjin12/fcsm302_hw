/* Priority with round-robin: highest priority first; tasks of equal priority share the CPU
   in round-robin order (an unfinished task is re-appended to the tail of the list). */
#include <stdlib.h>
#include "schedulers.h"
#include "list.h"
#include "CPU.h"
static struct node *head = NULL;
void add(char *name, int priority, int burst) { insert(&head, new_task(name, priority, burst)); }
Task *pickNextTask(void) {
    Task *best = NULL;
    for (struct node *p = head; p; p = p->next)
        if (!best || p->task->priority > best->priority) best = p->task;   /* first in list among equals */
    return best;
}
void schedule(void) {
    Task *t;
    while ((t = pickNextTask())) {
        int slice = t->remaining < quantum ? t->remaining : quantum;
        run(t, slice);
        delete(&head, t);
        if (t->remaining > 0) insert(&head, t);
    }
}
