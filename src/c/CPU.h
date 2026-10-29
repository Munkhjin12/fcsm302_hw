#ifndef CPU_H
#define CPU_H
#include "task.h"
void run(Task *task, int slice);   /* execute task on the (simulated) CPU for slice ms */
#endif
