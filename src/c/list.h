#ifndef LIST_H
#define LIST_H
#include "task.h"
struct node { Task *task; struct node *next; };
void insert(struct node **head, Task *task);   /* append at tail (keeps file order) */
void delete(struct node **head, Task *task);   /* remove the node holding task */
#endif
