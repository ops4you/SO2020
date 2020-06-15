#ifndef TASK_TASK_H
#define TASK_TASK_H

#include <sys/types.h>

#include <stddef.h>

typedef struct Task {
    size_t task_id;
    char const* task_name;
    pid_t process_group;
} Task;

#endif  // TASK_TASK_H
