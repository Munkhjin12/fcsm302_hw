#ifndef SCHEDULERS_H
#define SCHEDULERS_H
#include "task.h"
extern int quantum;                                   /* set by driver.c */
void add(char *name, int priority, int burst);        /* add a task to the scheduler's list */
void schedule(void);                                  /* run all tasks with the selected algorithm */
Task *pickNextTask(void);                             /* choose the next task to run */
#endif
