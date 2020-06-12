#include "buf_reader.h"

#include <unistd.h>

#include <sys/uio.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifndef static_assert
#define static_assert _Static_assert
#endif  // static_assert

#define try_read_(fd, buf, n) \
    if (read(fd, buf, n) == -1l) return BR_ERR_READ_FAIL

#define try_readv_(fd, iov, iovcnt) \
    if (readv(fd, iov, iovcnt) == -1l) return BR_ERR_READ_FAIL

#define DEFAULT_CAP 8192ul

BrOutcome br_with_deafault_cap(BufReader* const init, int const file_des) {
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
    init->pos = 0ul;
    init->cap = 0ul;
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
    init->pos = 0ul;
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
    init->pos = 0ul;
    init->cap = buf_size;
}

void br_drop(BufReader* const self) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    free(self->buf);
    self->pos = 0ul;
}

BrOutcome br_close(BufReader* const self) {
    return close(self->file_des) == -1l ? BR_ERR_CLOSE_FAIL : BR_OK;
}

BrOutcome br_drop_and_close(BufReader* const self) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    free(self->buf);
    self->pos = 0ul;
    return close(self->file_des) == -1l ? BR_ERR_CLOSE_FAIL : BR_OK;
}

char* br_replace_buf(
    BufReader* const restrict self,
    char* const restrict new_buf
) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(new_buf != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    char* const old_buf = self->buf;
    self->buf = new_buf;
    self->pos = 0ul;
    return old_buf;
}

int br_replace_file(BufReader* const self, int const new_file_des) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(new_file_des > 0);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    int const old_descriptor = self->file_des;
    self->file_des = new_file_des;
    self->pos = 0ul;
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

size_t br_pos(BufReader const* const self) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    return self->pos;
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
    size_t const n
) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(buf != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    // if (n < self->pos) {
    //     memcpy(buf, self->buf, n);
    //     self->pos -= self->pos;
    // } else {
    //     memcpy(buf, self->buf, self->pos);
    //     if (n > self->cap) {
    //         try_read_(self->file_des, buf, n - self->pos);
    //         self->pos = 0u;
    //     } else {
    //         struct iovec iov[2];
    //         iov[0].iov_base = buf;
    //         iov[0].iov_len = n - self->pos;
    //         iov[1].iov_base = self->buf;
    //         iov[1].iov_len = self->cap;
    //         try_readv_(self->file_des, iov, 2);
    //     }
    // }
    return BR_OK;
}

// TODO: implement.
BrOutcome br_read_line(
    BufReader* const restrict self,
    char* const restrict buf,
    size_t const n,
    size_t* const restrict line_len
) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(buf != NULL);
    assert(line_len != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    return BR_OK;
}

// TODO: implement.
BrOutcome br_read_char(BufReader* const restrict self, char* const restrict c) {
#   if BUF_READER_RUNTIME_ASSERTS
    assert(self != NULL);
    assert(c != NULL);
#   endif  // BUF_READER_RUNTIME_ASSERTS

    return BR_OK;
}
