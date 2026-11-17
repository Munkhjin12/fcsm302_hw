#include <stdio.h>
#include "CPU.h"

static long long now = 0;   /* simulated clock (ms); all tasks arrive at t = 0 */

void run(Task *task, int slice) {
    printf("Running task = [%s] [%d] [%d] for %d units.  (t = %lld..%lld)\n",
           task->name, task->priority, task->remaining, slice, now, now + slice);
    if (task->start < 0) task->start = now;
    now += slice;
    task->remaining -= slice;
    if (task->remaining == 0) task->finish = now;
}
