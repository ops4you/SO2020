#include "buf_writer.h"

#include <unistd.h>

#include <sys/uio.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifndef static_assert
#define static_assert _Static_assert
#endif  // static_assert

#define try_write_(fd, buf, n) \
    if (write(fd, buf, n) == -1l) return BW_ERR_WRITE_FAIL

#define try_writev_(fd, iov, iovcnt) \
    if (writev(fd, iov, iovcnt) == -1l) return BW_ERR_WRITE_FAIL

#define is_at_max_cap_(self) ((self)->pos == (self)->cap)

#define try_flush_(self) \
    if (self->pos > 0ul) try_write_(self->file_des, self->buf, self->pos)

#define DEFAULT_CAP 8192ul

BwOutcome bw_with_default_cap(BufWriter* const init, int const file_des) {
    static_assert(
        DEFAULT_CAP > 0ul,
        "Expected initial BufWriter capacity to be positive"
    );

#   if BUF_WRITER_RUNTIME_ASSERTS
    assert(init != NULL);
    assert(file_des > 0);
#   endif  // BUF_WRITER_RUNTIME_ASSERTS

    init->buf = malloc(DEFAULT_CAP);
    if (!init->buf) {
        return BW_ERR_ALLOC_FAIL;
    }
    init->file_des = file_des;
    init->pos = 0ul;
    init->cap = DEFAULT_CAP;
    return BW_OK;
}

BwOutcome bw_with_cap(
    BufWriter* const init,
    int const file_des,
    size_t const capacity
) {
#   if BUF_WRITER_RUNTIME_ASSERTS
    assert(init != NULL);
    assert(file_des > 0);
    assert(capacity > 0ul);
#   endif  // BUF_WRITER_RUNTIME_ASSERTS

    init->buf = malloc(capacity);
    if (!init->buf) {
        return BW_ERR_ALLOC_FAIL;
    }
    init->file_des = file_des;
    init->pos = 0ul;
    init->cap = capacity;
    return BW_OK;
}

void bw_with_buf(
    BufWriter* const restrict init,
    int const file_des,
    char* const restrict buf,
    size_t const buf_size
) {
#   if BUF_WRITER_RUNTIME_ASSERTS
    assert(init != NULL);
    assert(file_des > 0);
    assert(buf != NULL);
    assert(buf_size > 0ul);
#   endif  // BUF_WRITER_RUNTIME_ASSERTS

    init->file_des = file_des;
    init->buf = buf;
    init->pos = 0ul;
    init->cap = buf_size;
}

BwOutcome bw_drop(BufWriter* const self) {
#   if BUF_WRITER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_WRITER_RUNTIME_ASSERTS

    try_flush_(self);
    free(self->buf);
    self->pos = 0ul;
    return BW_OK;
}

BwOutcome bw_close(BufWriter* const self) {
#   if BUF_WRITER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_WRITER_RUNTIME_ASSERTS

    try_flush_(self);
    return close(self->file_des) == -1l ? BW_ERR_CLOSE_FAIL : BW_OK;
}

BwOutcome bw_drop_and_close(BufWriter* const self) {
#   if BUF_WRITER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_WRITER_RUNTIME_ASSERTS

    try_flush_(self);
    free(self->buf);
    self->pos = 0ul;
    return close(self->file_des) == -1l ? BW_ERR_CLOSE_FAIL : BW_OK;
}

char* bw_replace_buf(
    BufWriter* const restrict self,
    char* const restrict new_buf,
    size_t const new_buf_size
) {
#   if BUF_WRITER_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(new_buf != NULL);
    assert(new_buf_size > 0ul);
#   endif  // BUF_WRITER_RUNTIME_ASSERTS

    char* const old_buf = self->buf;
    self->buf = new_buf;
    self->cap = new_buf_size;
    self->pos = 0ul;
    return old_buf;
}

int bw_replace_file(BufWriter* const self, int const new_file_des) {
#   if BUF_WRITER_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(new_file_des > 0);
#   endif  // BUF_WRITER_RUNTIME_ASSERTS

    int const old_descriptor = self->file_des;
    self->file_des = new_file_des;
    self->pos = 0ul;
    return old_descriptor;
}

int bw_descriptor(BufWriter const* const self) {
#   if BUF_WRITER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_WRITER_RUNTIME_ASSERTS

    return self->file_des;
}

int bw_descriptor_mut(BufWriter* const self) {
#   if BUF_WRITER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_WRITER_RUNTIME_ASSERTS

    return self->file_des;
}

size_t bw_used_bytes(BufWriter const* const self) {
#   if BUF_WRITER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_WRITER_RUNTIME_ASSERTS

    return self->pos;
}

size_t bw_cap(BufWriter const* const self) {
#   if BUF_WRITER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_WRITER_RUNTIME_ASSERTS

    return self->cap;
}

BwOutcome bw_write(
    BufWriter* const restrict self,
    char const* const restrict buf,
    size_t const n
) {
#   if BUF_WRITER_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(buf != NULL);
#   endif  // BUF_WRITER_RUNTIME_ASSERTS

    size_t const available_size = self->cap - self->pos;
    if (n > available_size) {
        if (n > self->cap) {
            if (self->pos != 0ul) {
                struct iovec iov[2];
                iov[0].iov_base = self->buf;
                iov[0].iov_len = self->pos;
                iov[1].iov_base = (char*) buf;  // iovecs not const ¯\_(ツ)_/¯
                iov[1].iov_len = n;
                try_writev_(self->file_des, iov, 2);
                self->pos = 0ul;
            } else {
                try_write_(self->file_des, buf, n);
            }
        } else {
            memcpy(self->buf + self->pos, buf, available_size);
            try_write_(self->file_des, self->buf, self->cap);
            self->pos = n - available_size;
            memcpy(self->buf, buf + available_size, self->pos);
        }
    } else {
        memcpy(self->buf + self->pos, buf, n);
        self->pos += n;
    }
    return BW_OK;
}

BwOutcome bw_write_line(
    BufWriter* const restrict self,
    char const* const restrict buf,
    size_t const n
) {
#   if BUF_WRITER_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(buf != NULL);
#   endif  // BUF_WRITER_RUNTIME_ASSERTS

    size_t const available_size = self->cap - self->pos;
    if (n > available_size) {
        if (n > self->cap) {
            if (self->pos != 0ul) {
                struct iovec iov[2];
                iov[0].iov_base = self->buf;
                iov[0].iov_len = self->pos;
                iov[1].iov_base = (char*) buf;  // iovecs not const ¯\_(ツ)_/¯
                iov[1].iov_len = n;
                try_writev_(self->file_des, iov, 2);
            } else {
                try_write_(self->file_des, buf, n);
            }
            self->buf[0] = '\n';
            self->pos = 1ul;
        } else {
            memcpy(self->buf + self->pos, buf, available_size);
            try_write_(self->file_des, self->buf, self->cap);
            self->pos = n - available_size;
            memcpy(self->buf, buf + available_size, self->pos);
            self->buf[self->pos++] = '\n';
        }
    } else {
        memcpy(self->buf + self->pos, buf, n);
        if (available_size == n) {
            try_write_(self->file_des, self->buf, self->cap);
            self->buf[0] = '\n';
            self->pos = 1ul;
        } else {
            self->pos += n;
            self->buf[self->pos++] = '\n';
        }
    }
    return BW_OK;
}

BwOutcome bw_write_char(BufWriter* self, char c) {
#   if BUF_WRITER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_WRITER_RUNTIME_ASSERTS

    if (is_at_max_cap_(self)) {
        try_write_(self->file_des, self->buf, self->cap);
        self->buf[0] = c;
        self->pos = 1ul;
    } else {
        self->buf[self->pos++] = c;
    }
    return BW_OK;
}

BwOutcome bw_flush(BufWriter* self) {
#   if BUF_WRITER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_WRITER_RUNTIME_ASSERTS

    try_flush_(self);
    return BW_OK;
}
