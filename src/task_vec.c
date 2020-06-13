#include "task_vec.h"

#include "task.h"

#if TASK_VEC_RUNTIME_ASSERTS
#include <assert.h>
#endif  // TASK_VEC_RUNTIME_ASSERTS

#include <stdlib.h>
#include <string.h>

#define FIRST_ALLOC_CAP 2ul
#define ALLOC_ATTEMPT_COUNT 5

#define is_at_max_cap_(self) self->len == (self)->cap
#define is_empty_(self) (self)->len == 0
#define end_(self) (self)->buf + (self)->len
#define at_(self, idx) (idx) >= (self)->len ? NULL : (self)->buf + (idx)

TaskVec* tvec_new(TaskVec* const init) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(init != NULL);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    init->buf = NULL;
    init->len = 0ul;
    init->cap = 0ul;
    return init;
}

TaskVec* tvec_with_cap(TaskVec* const init, size_t const capacity) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(init != NULL);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    size_t buf_size;
    if (__builtin_mul_overflow(capacity, sizeof *init->buf, &buf_size)) {
        return NULL;
    }
    init->buf = malloc(capacity * sizeof *init->buf);
    if (!init->buf) {
        return false;
    }
    init->len = 0ul;
    init->cap = capacity;
    return init;
}

void tvec_drop(TaskVec* const self) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    free(self->buf);
}

size_t tvec_len(TaskVec const* const self) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    return self->len;
}

size_t tvec_cap(TaskVec const* const self) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    return self->cap;
}

bool tvec_is_empty(TaskVec const* const self) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    return is_empty_(self);
}

bool tvec_is_at_max_cap(TaskVec const* const self) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    return is_at_max_cap_(self);
}

bool tvec_shrink_to_fit(TaskVec* const self) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    if (is_at_max_cap_(self)) {
        return false;
    }
    Task* const new_buf = realloc(self->buf, self->len * sizeof *self->buf);
    if (!new_buf) {
        return false;
    }
    self->buf = new_buf;
    self->cap = self->len;
    return true;
}

bool tvec_shrink_to(TaskVec* const self, size_t const min_capacity) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    if (is_at_max_cap_(self)) {
        return false;
    }
    size_t const new_cap = self->len < min_capacity ? self->len : min_capacity;
    Task* const new_buf = realloc(self->buf, new_cap * sizeof *self->buf);
    if (!new_buf) {
        return false;
    }
    self->buf = new_buf;
    self->cap = new_cap;
    return true;
}

Task const* tvec_begin(TaskVec const* const self) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    return self->buf;
}

Task* tvec_begin_mut(TaskVec* const self) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    return self->buf;
}

Task const* tvec_end(TaskVec const* const self) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    return end_(self);
}

Task const* tvec_at(TaskVec const* const self, size_t const idx) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(idx < self->len);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    return at_(self, idx);
}

Task* tvec_at_mut(TaskVec* const self, size_t const idx) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(idx < self->len);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    return at_(self, idx);
}

bool tvec_push(TaskVec* const restrict self, Task* const restrict task) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(task != NULL);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    if (is_at_max_cap_(self)) {
        Task* new_buf;
        size_t new_cap;
        size_t additional_cap =
            self->cap != 0 ? (self->cap >> 1u) : FIRST_ALLOC_CAP;
        for (size_t i = 0; i < ALLOC_ATTEMPT_COUNT; ++i) {
            size_t new_size;
            if (__builtin_add_overflow(self->cap, additional_cap, &new_cap) ||
                __builtin_mul_overflow(new_cap, sizeof *self->buf, &new_size)
            ) {
                additional_cap = self->cap >> (i + 1);
                if (additional_cap <= self->cap ||
                    i == ALLOC_ATTEMPT_COUNT - 1
                ) {
                    return false;
                }
                continue;
            }
            new_buf = realloc(self->buf, new_size);
            if (new_buf) {
                break;
            }
        }
        self->buf = new_buf;
        self->cap = new_cap;
    }
    self->buf[self->len++] = *task;
    return true;
}

bool tvec_insert_at(
    TaskVec* const restrict self,
    struct Task* const restrict task,
    size_t const idx
) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(task != NULL);
    assert(idx < self->len);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    if (idx > self->len) {
        return false;
    }
    if (is_at_max_cap_(self)) {
        Task* new_buf;
        size_t new_cap;
        size_t additional_cap =
            self->cap != 0 ? (self->cap >> 1u) : FIRST_ALLOC_CAP;
        for (size_t i = 0; i < ALLOC_ATTEMPT_COUNT; ++i) {
            size_t new_size;
            if (__builtin_add_overflow(self->cap, additional_cap, &new_cap) ||
                __builtin_mul_overflow(new_cap, sizeof *self->buf, &new_size)
            ) {
                additional_cap = self->cap >> (i + 1);
                if (additional_cap <= self->cap ||
                    i == ALLOC_ATTEMPT_COUNT - 1
                ) {
                    return false;
                }
                continue;
            }
            new_buf = realloc(self->buf, new_size);
            if (new_buf) {
                break;
            }
        }
        self->buf = new_buf;
        self->cap = new_cap;
    }
    if (self->len > 0) {
        memmove(
            self->buf + idx + 1,
            self->buf + idx,
            (self->len - idx) * sizeof *self->buf
        );
        self->buf[idx] = *task;
    } else {
        self->buf[0] = *task;
    }
    ++self->len;
    return true;
}

Task* tvec_pop(TaskVec* const restrict self, Task* const restrict opt_task) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    if (is_empty_(self)) {
        return NULL;
    }
    --self->len;
    if (opt_task) {
        *opt_task = self->buf[self->len];
    }
    return opt_task;
}

bool tvec_rm_at(TaskVec* const self, size_t const idx) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(idx < self->len);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    if (idx >= self->len) {
        return NULL;
    }
    if (self->len > 1) {
        self->buf[idx] = self->buf[self->len - 1];
    }
    --self->len;
    return true;
}

bool tvec_rm_ord_at(TaskVec* const self, size_t const idx) {
#   if TASK_VEC_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(idx < self->len);
#   endif  // TASK_VEC_RUNTIME_ASSERTS

    if (idx >= self->len) {
        return NULL;
    }
    if (self->len > 1) {
        memmove(
            self->buf + idx,
            self->buf + idx + 1,
            (self->len - idx - 1) * sizeof *self->buf
        );
    }
    --self->len;
    return true;
}
