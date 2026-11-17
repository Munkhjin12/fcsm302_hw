#include <stdio.h>
#include <stdlib.h>
#include "list.h"

void insert(struct node **head, Task *task) {
    struct node *n = task ? malloc(sizeof *n) : NULL;
    if (!n) {
        fputs("Error: cannot register or queue task.\n", stderr);
        exit(EXIT_FAILURE);
    }
    struct node *p;
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
