#ifndef TASK_H
#define TASK_H

#include <stddef.h>

typedef struct Task {
    char const* task_name;
} Task;

Task* new_task(Task* restrict init, char const* restrict task_name);

void display_task(Task const* restrict self, char* restrict buf, size_t n);

void print_task(Task const* restrict self);

#endif  // TASK_H
