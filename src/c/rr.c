/* Round-robin: each task runs for min(quantum, remaining); unfinished tasks go to the tail. */
#include <stdlib.h>
#include "schedulers.h"
#include "list.h"
#include "CPU.h"
static struct node *head = NULL;
void add(char *name, int priority, int burst) { insert(&head, new_task(name, priority, burst)); }
Task *pickNextTask(void) { return head ? head->task : NULL; }
void schedule(void) {
    Task *t;
    while ((t = pickNextTask())) {
        int slice = t->remaining < quantum ? t->remaining : quantum;
        run(t, slice);
        delete(&head, t);
        if (t->remaining > 0) insert(&head, t);
    }
}
