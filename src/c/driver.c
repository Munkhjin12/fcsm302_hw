/* Reads a schedule file ("name, priority, burst" per line), adds the tasks to the
   selected scheduler (chosen at build time), runs it and prints the metrics.
   Usage: ./fcfs schedule.txt [quantum] */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "task.h"
#include "schedulers.h"

int quantum = QUANTUM;

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <schedule.txt> [quantum]\n", argv[0]);
        return 1;
    }
    if (argc == 3) {
        char *end; long v = strtol(argv[2], &end, 10);
        if (*end || v < 1 || v > 1000) {
            fprintf(stderr, "Error: quantum must be an integer from 1 to 1000 (got \"%s\").\n", argv[2]);
            return 1;
        }
        quantum = (int)v;
    }
    FILE *in = fopen(argv[1], "r");
    if (!in) { perror(argv[1]); return 1; }

    char line[256]; int ln = 0, count = 0;
    while (fgets(line, sizeof line, in)) {
        ln++;
        char *s = line; while (isspace((unsigned char)*s)) s++;
        if (*s == '\0' || *s == '#') continue;               /* skip blank lines and comments */
        char name[64], extra; int pr, bu;
        if (sscanf(s, " %63[^,], %d , %d %c", name, &pr, &bu, &extra) != 3) {
            fprintf(stderr, "Error (%s, line %d): expected \"name, priority, burst\" but found: %s", argv[1], ln, line);
            return 1;
        }
        for (int i = (int)strlen(name) - 1; i >= 0 && isspace((unsigned char)name[i]); i--) name[i] = '\0';
        if (pr < MIN_PRIORITY || pr > MAX_PRIORITY) {
            fprintf(stderr, "Error (%s, line %d): priority %d is outside %d..%d.\n", argv[1], ln, pr, MIN_PRIORITY, MAX_PRIORITY);
            return 1;
        }
        if (bu < 1) {
            fprintf(stderr, "Error (%s, line %d): CPU burst must be at least 1 (got %d).\n", argv[1], ln, bu);
            return 1;
        }
        if (count >= MAX_TASKS) { fprintf(stderr, "Error: more than %d tasks.\n", MAX_TASKS); return 1; }
        add(name, pr, bu);
        count++;
    }
    fclose(in);
    if (count == 0) { fprintf(stderr, "Error: %s contains no tasks.\n", argv[1]); return 1; }

    schedule();
    report();
    return 0;
}
