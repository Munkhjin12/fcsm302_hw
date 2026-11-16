/* Usage: ./fcfs schedule.txt [quantum]. Each task arrives at t = 0. */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "schedulers.h"

int quantum = QUANTUM;

enum { MAX_LINE_BYTES = 255 };

struct input_task {
    char name[MAX_NAME_BYTES + 1];
    int priority;
    int burst;
};

static char *trim(char *value) {
    while (isspace((unsigned char)*value)) value++;
    char *end = value + strlen(value);
    while (end > value && isspace((unsigned char)end[-1])) end--;
    *end = '\0';
    return value;
}

static int parse_integer(const char *value, int min, int max, int *result) {
    char *end;
    errno = 0;
    long number = strtol(value, &end, 10);
    if (end == value || *end != '\0' || errno == ERANGE || number < min || number > max)
        return 0;
    *result = (int)number;
    return 1;
}

static const char *parse_task(char *line, struct input_task *task) {
    char *priority = strchr(line, ',');
    if (!priority) return "expected name, priority, burst";
    *priority++ = '\0';
    char *burst = strchr(priority, ',');
    if (!burst) return "expected name, priority, burst";
    *burst++ = '\0';

    char *name = trim(line);
    size_t len = strlen(name);
    if (len == 0) return "task name cannot be empty";
    if (len > MAX_NAME_BYTES) return "task name is longer than 63 bytes";
    for (size_t i = 0; i < len; i++) {
        unsigned char ch = (unsigned char)name[i];
        if (ch < ' ' || ch == 127 || ch == '[' || ch == ']')
            return "task name cannot contain brackets or control characters";
    }

    if (!parse_integer(trim(priority), MIN_PRIORITY, MAX_PRIORITY, &task->priority))
        return "priority must be an integer in 1..10";
    if (!parse_integer(trim(burst), 1, MAX_BURST, &task->burst))
        return "CPU burst must be in 1..1000 and must be an integer";
    memcpy(task->name, name, len + 1);
    return NULL;
}

static int fail_at(const char *file, size_t line, const char *message) {
    fprintf(stderr, "Error (%s, line %zu): %s.\n", file, line, message);
    return 0;
}

static int read_tasks(FILE *input, const char *path, struct input_task tasks[], int *count) {
    char line[MAX_LINE_BYTES + 1];
    size_t line_number = 0;
    long long total = 0;
    while (1) {
        size_t length = 0;
        line_number++;
        int ch;
        while ((ch = fgetc(input)) != EOF && ch != '\n') {
            if (ch == '\0') return fail_at(path, line_number, "NUL byte in input");
            if (length == MAX_LINE_BYTES)
                return fail_at(path, line_number, "line is longer than 255 bytes");
            line[length++] = (char)ch;
        }
        if (ferror(input)) {
            perror(path);
            return 0;
        }
        if (ch == EOF && length == 0) break;
        line[length] = '\0';

        char *start = trim(line);
        if (*start == '\0' || *start == '#') continue;
        if (*count == MAX_TASKS)
            return fail_at(path, line_number, "more than 1000 tasks");

        struct input_task *task = &tasks[*count];
        const char *error = parse_task(start, task);
        if (error) return fail_at(path, line_number, error);
        for (int i = 0; i < *count; i++) {
            if (strcmp(task->name, tasks[i].name) == 0)
                return fail_at(path, line_number, "duplicate task name");
        }
        if (total > MAX_TOTAL_TIME - task->burst)
            return fail_at(path, line_number, "schedule exceeds the total time limit");
        total += task->burst;
        (*count)++;
    }
    if (*count == 0) {
        fprintf(stderr, "Error: %s contains no tasks.\n", path);
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <schedule.txt> [quantum]\n", argv[0]);
        return 1;
    }
    if (argc == 3 && !parse_integer(argv[2], 1, MAX_QUANTUM, &quantum)) {
        fprintf(stderr, "Error: quantum must be an integer from 1 to %d.\n", MAX_QUANTUM);
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    if (!input) {
        perror(argv[1]);
        return 1;
    }
    struct input_task tasks[MAX_TASKS];
    int count = 0;
    int valid = read_tasks(input, argv[1], tasks, &count);
    fclose(input);
    if (!valid) return 1;

    for (int i = 0; i < count; i++) add(tasks[i].name, tasks[i].priority, tasks[i].burst);
    schedule();
    report();
    return 0;
}
