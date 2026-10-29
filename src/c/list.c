#include <stdlib.h>
#include "list.h"

void insert(struct node **head, Task *task) {
    struct node *n = malloc(sizeof *n), *p;
    n->task = task; n->next = NULL;
    if (!*head) { *head = n; return; }
    for (p = *head; p->next; p = p->next) ;
    p->next = n;
}

void delete(struct node **head, Task *task) {
    struct node *p = *head, *prev = NULL;
    while (p && p->task != task) { prev = p; p = p->next; }
    if (!p) return;
    if (prev) prev->next = p->next; else *head = p->next;
    free(p);
}
