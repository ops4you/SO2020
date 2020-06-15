#include "buf_io/buf_reader.h"

#include <unistd.h>

#include <sys/uio.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifndef static_assert
#define static_assert _Static_assert
#endif  // static_assert

#ifndef thread_local
#define thread_local _Thread_local
#endif  // thread_local

#define try_read_(fd, buf, n) \
    if (read(fd, buf, n) == -1l) return BR_ERR_READ_FAIL

#define try_readv_(fd, iov, iovcnt) \
    if (readv(fd, iov, iovcnt) == -1l) return BR_ERR_READ_FAIL

#define DEFAULT_CAP 8192ul
#define OUTCOME_MSG_SIZE 1024ul

BrOutcome br_with_default_cap(BufReader* const init, int const file_des) {
    static_assert(
        DEFAULT_CAP > 0ul,
        "Expected initial BufReader capacity to be positive"
    );

#   if BUF_READER_RUNTIME_ASSERTS
    assert(init != NULL);
    assert(file_des > 0);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    init->buf = malloc(DEFAULT_CAP);
    if (!init->buf) {
        return BR_ERR_ALLOC_FAIL;
    }
    init->file_des = file_des;
    init->begin = 0ul;
    init->end = 0ul;
    init->cap = DEFAULT_CAP;
    return BR_OK;
}

BrOutcome br_with_cap(
    BufReader* const init,
    int const file_des,
    size_t const capacity
) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(init != NULL);
    assert(file_des > 0);
    assert(capacity > 0ul);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    init->buf = malloc(capacity);
    if (!init->buf) {
        return BR_ERR_ALLOC_FAIL;
    }
    init->file_des = file_des;
    init->begin = 0ul;
    init->end = 0ul;
    init->cap = capacity;
    return BR_OK;
}

void br_with_buf(
    BufReader* const restrict init,
    int const file_des,
    char* const restrict buf,
    size_t const buf_size
) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(init != NULL);
    assert(file_des > 0);
    assert(buf != NULL);
    assert(buf_size > 0ul);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    init->file_des = file_des;
    init->buf = buf;
    init->begin = 0ul;
    init->end = 0ul;
    init->cap = buf_size;
}

void br_drop(BufReader* const self) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    free(self->buf);
}

BrOutcome br_close(BufReader* const self) {
    return close(self->file_des) == -1l ? BR_ERR_CLOSE_FAIL : BR_OK;
}

BrOutcome br_drop_and_close(BufReader* const self) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    free(self->buf);
    return close(self->file_des) == -1l ? BR_ERR_CLOSE_FAIL : BR_OK;
}

char* br_replace_buf(
    BufReader* const restrict self,
    char* const restrict new_buf,
    size_t const new_buf_size
) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(new_buf != NULL);
    assert(new_buf_size > 0ul);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    char* const old_buf = self->buf;
    self->buf = new_buf;
    self->cap = new_buf_size;
    self->begin = 0ul;
    self->end = 0ul;
    return old_buf;
}

int br_replace_file(BufReader* const self, int const new_file_des) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(new_file_des > 0);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    int const old_descriptor = self->file_des;
    self->file_des = new_file_des;
    self->begin = 0ul;
    self->end = 0ul;
    return old_descriptor;
}

int br_descriptor(BufReader const* const self) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    return self->file_des;
}

int br_descriptor_mut(BufReader* const self) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    return self->file_des;
}

char const* br_internal_buf(BufReader const* const self) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    return self->buf;
}

char* br_internal_buf_mut(BufReader* const self) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    return self->buf;
}

// TODO: implement.
size_t br_available_bytes(BufReader const* const self) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    (void) self;

    return 0ul;
}

size_t br_cap(BufReader const* const self) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    return self->cap;
}

// TODO: implement.
BrOutcome br_read(
    BufReader* const restrict self,
    char* const restrict buf,
    size_t n,
    size_t* const restrict read_bytes
) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(buf != NULL);
    assert(read_bytes != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    (void) self;
    (void) buf;
    (void) n;
    (void) read_bytes;

    return BR_OK;
}

// TODO: implement.
BrOutcome br_read_line(
    BufReader* const restrict self,
    char* const restrict buf,
    size_t const n,
    size_t* const restrict read_bytes
) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(buf != NULL);
    assert(read_bytes != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    (void) self;
    (void) buf;
    (void) n;
    (void) read_bytes;

    return BR_OK;
}

// TODO: implement.
BrOutcome br_read_char(BufReader* const restrict self, char* const restrict c) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(c != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    (void) self;
    (void) c;

    return BR_OK;
}

char const* br_outcome_msg(
    BrOutcome const outcome,
    int const* const opt_errno
) {
    static_assert(
        BR_OK == 0 &&
            BR_ERR_ALLOC_FAIL == 1 &&
            BR_ERR_READ_FAIL == 2 &&
            BR_ERR_CLOSE_FAIL == 3,
        "Unexpected BrOutcome enumerate values"
    );

#   if BUF_READER_RUNTIME_ASSERTS
    assert(outcome >= BR_OK && outcome <= BR_ERR_CLOSE_FAIL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    static char const* const msgs[] = {
        "success",
        "failed allocating dynamic memory for the BufReader",
        "failed reading from the file descriptor",
        "failed closing the file",
        "unknown BufReader error"
    };
    switch (outcome) {
    case BR_ERR_ALLOC_FAIL:
    case BR_ERR_READ_FAIL:
    case BR_ERR_CLOSE_FAIL:
        if (opt_errno) {
            static thread_local char msg_with_err[OUTCOME_MSG_SIZE];
            snprintf(
                msg_with_err,
                OUTCOME_MSG_SIZE,
                "%s: %s",
                msgs[outcome],
                strerror(*opt_errno)
            );
            return msg_with_err;
        }
    case BR_OK:
        return msgs[outcome];
    default:
        return msgs[sizeof msgs / sizeof *msgs - 1];
    }
}
