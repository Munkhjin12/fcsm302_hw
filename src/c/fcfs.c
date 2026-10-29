/* First-come, first-served: tasks run in the order they were added. */
#include <stdlib.h>
#include "schedulers.h"
#include "list.h"
#include "CPU.h"
static struct node *head = NULL;
void add(char *name, int priority, int burst) { insert(&head, new_task(name, priority, burst)); }
Task *pickNextTask(void) { return head ? head->task : NULL; }
void schedule(void) {
    Task *t;
    while ((t = pickNextTask())) { run(t, t->remaining); delete(&head, t); }
}
