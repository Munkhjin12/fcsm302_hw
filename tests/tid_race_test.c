/* Further Challenge 1 -- demonstrates why the task id counter must be atomic.
   N threads each take PER ids from a shared counter. With an atomic increment every id is
   unique (the counter ends exactly at N*PER+1); with a plain "x++" ids are lost/duplicated.
   Build: gcc -O2 -pthread tid_race_test.c -o tid_race_test */
#include <stdio.h>
#include <pthread.h>
#define N 8
#define PER 200000
static volatile int plain_id = 1;
static int atomic_id = 1;
static void *worker(void *arg) {
    int mode = *(int *)arg;
    for (int i = 0; i < PER; i++) {
        if (mode) __sync_fetch_and_add(&atomic_id, 1);
        else plain_id = plain_id + 1;            /* read-modify-write: not atomic */
    }
    return NULL;
}
static int run(int mode) {
    pthread_t th[N];
    for (int i = 0; i < N; i++) pthread_create(&th[i], NULL, worker, &mode);
    for (int i = 0; i < N; i++) pthread_join(th[i], NULL);
    return mode ? atomic_id : plain_id;
}
int main(void) {
    int expected = N * PER + 1;
    int p = run(0), a = run(1);
    printf("expected counter value : %d\n", expected);
    printf("plain  x = x + 1       : %d  (lost updates: %d)\n", p, expected - p);
    printf("__sync_fetch_and_add   : %d  (lost updates: %d)\n", a, expected - a);
    return a == expected ? 0 : 1;
}
