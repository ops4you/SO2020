#include "task.h"

#include <stdio.h>
#include <string.h>

Task* new_task(
    Task* const restrict init,
    char const* const restrict task_name
) {
    init->task_name = task_name;
    return init;
}

void display_task(Task const* const self, char* const restrict buf, size_t n) {
    strncpy(buf, self->task_name, n);
}

void print_task(Task const* const restrict self) {
    puts(self->task_name);
}
