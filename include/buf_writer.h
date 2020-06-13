#ifndef BUF_IO_BUF_WRITER_H
#define BUF_IO_BUF_WRITER_H

#include <stddef.h>

/**
 * If set to @p 1, perform runtime assertions on possibly invalid function
 * arguments.
 */
#define BUF_WRITER_RUNTIME_ASSERTS 0

/**
 * A wrapper around a file descriptor opened with write mode that provides
 * buffered writing.
 */
typedef struct BufWriter {
    int file_des;   //!< The underlying file descriptor.
    char* buf;  //!< The underlying intermediary buffer.
    size_t pos; //!< The current position in the buffer.
    size_t cap; //!< The buffer's maximum capacity.
} BufWriter;

/**
 * An outcome returned by BufWRiter functions.
 */
typedef enum BwOutcome {
    BW_OK,  //!< No error.
    BW_ERR_ALLOC_FAIL,  //!< Error occurred while allocating dynamic memory.
    BW_ERR_WRITE_FAIL,  //!< Error occurred while writing to a file.
    BW_ERR_CLOSE_FAIL,  //!< Error occurred while closing a file.
} BwOutcome;

/**
 * Creates a BufWriter with the default capacity from a file descriptor.
 * The current default capacity is 8192 bytes, but may change in the future.
 * The underlying buffer is allocated dynamically with <tt>malloc(3)</tt>.
 * If @p BUF_WRITER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * made:
 * 1. <tt>assert(init != NULL)</tt>;
 * 2. <tt>assert(file_des > 0)</tt>.
 * <tt>O(malloc(default capacity))</tt> complexity.
 * @param init (output parameter) address of the BufWriter to initialize.
 * <b>Must not be @p NULL.</b>
 * @param file_des the file descriptor to be associated with the BufWriter.
 * <b>Must be a valid file descriptor opened with write mode.</b>
 * @return @p BW_ERR_ALLOC_FAIL if memory allocation fails, otherwise @p BW_OK.
 */
BwOutcome bw_with_default_cap(BufWriter* init, int file_des);

/**
 * Creates a BufWriter with the provided capacity from a file descriptor.
 * The underlying buffer is allocated dynamically with <tt>malloc(3)</tt>.
 * If @p BUF_WRITER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * made:
 * 1. <tt>assert(init != NULL)</tt>;
 * 2. <tt>assert(file_des > 0)</tt>;
 * 3. <tt>assert(capacity > 0ul)</tt>.
 * <tt>O(malloc(capacity))</tt> complexity.
 * @param init (output parameter) address of the BufWriter to initialize.
 * <b>Must not be @p NULL.</b>
 * @param file_des the file descriptor to be associated with the BufWriter.
 * <b>Must be a valid file descriptor opened with write mode.</b>
 * @param capacity the desired capacity of the BufWriter.
 * <b>Must be positive.</b>
 * @return @p BW_ERR_ALLOC_FAIL if memory allocation fails, otherwise @p BW_OK.
 */
BwOutcome bw_with_cap(BufWriter* init, int file_des, size_t capacity);

/**
 * Creates a BufWriter from a file descriptor, that uses the provided buffer as
 * its underlying buffer.
 * If @p BUF_WRITER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * made:
 * 1. <tt>assert(init != NULL)</tt>;
 * 2. <tt>assert(file_des > 0)</tt>;
 * 3. <tt>assert(buf != NULL)</tt>;
 * 4. <tt>assert(buf_size > 0ul).
 * <tt>O(1)</tt> complexity.
 * @param init (output parameter) address of the BufWriter to initialize.
 * <b>Must not be @p NULL.</b>
 * @param file_des the file descriptor to be associated with the BufWriter.
 * <b>Must be a valid file descriptor opened with write mode.</b>
 * @param buf the buffer to be used as the BufWriter's intermediary buffer.
 * <b>Must not be @p NULL.</b>
 * @param buf_size the size of the provided buffer that will be used as the
 * BufWriter's capacity.
 * <b>Must be positive.</b>
 */
void bw_with_buf(
    BufWriter* restrict init,
    int file_des,
    char* restrict buf,
    size_t buf_size
);

/**
 * Flushes the BufWriter and deallocates its underlying buffer using
 * <tt>free(3)</tt>, but doesn't close the associated file.
 * <b>Attempts to write using the BufWriter with the deallocated buffer result
 * in undefined behaviour.</b>
 * <b>If flushing fails, the buffer isn't deallocated.</b>
 * If @p BUF_WRITER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * made:
 * 1. <tt>assert(self != NULL)</tt>.
 * <tt>O(free(self->buf))</tt> complexity.
 * @param self address of the BufWriter whose buffer shall be deallocated.
 * <b>Must not be @p NULL.</b>
 * @return @p BW_ERR_WRITE_FAIL if flushing fails, otherwise @p BW_OK.
 */
BwOutcome bw_drop(BufWriter* self);

/**
 * Flushes the BufWriter and closes its associated file using
 * <tt>close(2)</tt>, but doesn't deallocate the underlying buffer.
 * <b>Attempts to write using the BufWriter with the closed file result in
 * undefined behaviour, if a file write occurs.</b>
 * <b>If flushing fails, the file isn't closed.</b>
 * If @p BUF_WRITER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * made:
 * 1. <tt>assert(self != NULL)</tt>.
 * <tt>O(close(self->file_des))</tt> complexity.
 * @param self address of the BufWriter whose file shall be closed.
 * <b>Must not be @p NULL.</b>
 * @return @p BW_ERR_WRITE_FAIL if flushing fails, otherwise
 * @p BW_ERR_CLOSE_FAIL if closing fails, otherwise @p BW_OK.
 */
BwOutcome bw_close(BufWriter* self);

/**
 * Flushes the BufWriter, deallocates its underlying buffer using
 * <tt>free(3)</tt> and closes its associated file using <tt>close(2)</tt>.
 * <b>Attempts to write using the BufWriter with the deallocated buffer and
 * closed file result in undefined behaviour.</b>
 * <b>If flushing fails, the buffer isn't deallocated, and the file isn't
 * closed.</b>
 * If @p BUF_WRITER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * made:
 * 1. <tt>assert(self != NULL)</tt>.
 * <tt>O(free(self->buf) + close(self->file_des))</tt> complexity.
 * @param self address of the BufWriter whose buffer shall be deallocated and
 * file shall be closed.
 * <b>Must not be @p NULL.</b>
 * @return @p BW_ERR_WRITE_FAIL if flushing fails, otherwise
 * @p BW_ERR_CLOSE_FAIL if closing fails, otherwise @p BW_OK.
 */
BwOutcome bw_drop_and_close(BufWriter* self);

/**
 * Replaces the BufWriter's underlying buffer with the provided buffer, and its
 * capacity with the new buffer's size, and returns the old buffer.
 * <b>Doesn't flush the BufWriter.</b>
 * If @p BUF_WRITER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>;
 * 2. <tt>assert(new_buf != NULL)</tt>.
 * 3. <tt>assert(new_buf_size > 0ul)</tt>.
 * <tt>O(1)</tt> complexity.
 * @param self address of the BufWriter whose underlying buffer shall be
 * replaced.
 * <b>Must not be @p NULL.</b>
 * @param new_buf the buffer to replace the BufWriter's buffer.
 * <b>Must not be @p NULL.</b>
 * @param new_buf_size the size of the new buffer. <b>Must be positive.</b>
 * @return the BufWriter's old buffer.
 */
char* bw_replace_buf(
    BufWriter* restrict self,
    char* restrict new_buf,
    size_t new_buf_size
);

/**
 * Replaces the BufWriter's underlying file descriptor with the provided one,
 * and returns the old descriptor.
 * <b>Doesn't flush the BufWriter.</b>
 * If @p BUF_WRITER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>;
 * 2. <tt>assert(new_file_des > 0)</tt>.
 * <tt>O(1)</tt> complexity.
 * @param self address of the BufWriter whose underlying file descriptor shall
 * be replaced.
 * @param new_file_des the file descriptor to replace the BufWriter's file
 * descriptor.
 * <b>Must be a valid file descriptor opened with write mode.</b>
 * @return the BufWriter's old file descriptor.
 */
int bw_replace_file(BufWriter* self, int new_file_des);

/**
 * Returns the BufWriter's underlying file descriptor.
 * The caller is expected to not mutate the file.
 * If @p BUF_WRITER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>.
 * <tt>O(1)</tt> complexity.
 * @param self address of the BufWriter whose underlying file descriptor shall
 * be returned.
 * <b>Must not be @p NULL.</b>
 * @return the BufWriter's underlying file descriptor.
 */
int bw_descriptor(BufWriter const* self);

/**
 * Returns the BufWriter's underlying file descriptor.
 * If @p BUF_WRITER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>.
 * <tt>O(1)</tt> complexity.
 * @param self address of the BufWriter whose underlying file descriptor shall
 * be returned.
 * <b>Must not be @p NULL.</b>
 * @return the BufWriter's underlying file descriptor.
 */
int bw_descriptor_mut(BufWriter* self);

/**
 * Returns the amount of used bytes in the BufWriter's buffer, i.e. the
 * amount of bytes that haven't yet been flushed.
 * If @p BUF_WRITER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>.
 * <tt>O(1)</tt> complexity.
 * @param self address of the BufWriter whose buffer's position shall be
 * returned.
 * <b>Must not be @p NULL.</b>
 * @return the amount of used bytes in the BufWriter's buffer.
 */
size_t bw_used_bytes(BufWriter const* self);

/**
 * Returns the BufWriter's maximum capacity.
 * The current default capacity is 8192 bytes, but may change in the future.
 * If @p BUF_WRITER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>.
 * <tt>O(1)</tt> complexity.
 * @param self address of the BufWriter whose capacity shall be returned.
 * <b>Must not be @p NULL.</b>
 * @return the BufWriter's maximum capacity.
 */
size_t bw_cap(BufWriter const* self);

/**
 * Writes @p n bytes of @p buf to the BufWriter.
 * If @p BUF_WRITER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>;
 * 2. <tt>assert(buf != NULL)</tt>.
 * Amortized <tt>O(n)</tt> complexity.
 * @param self address of the BufWriter to which @p buf shall be written to.
 * <b>Must not be @p NULL.</b>
 * @param buf the buffer to be written to the BufWriter.
 * <b>Must not be @p NULL.</b>
 * @param n the amount of bytes to be written to the BufWriter.
 * @return @p BW_ERR_WRITE_FAIL if a file write occurs and fails, otherwise
 * @p BW_OK.
 */
BwOutcome bw_write(
    BufWriter* restrict self,
    char const* restrict buf,
    size_t n
);

/**
 * Writes @p n bytes of @p buf to the BufWriter, and appends a newline
 * character.
 * If @p BUF_WRITER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>;
 * 2. <tt>assert(buf != NULL)</tt>.
 * Amortized <tt>O(n)</tt> complexity.
 * @param self address of the BufWriter to which @p buf shall be written to.
 * <b>Must not be @p NULL.</b>
 * @param buf the buffer to be written to the BufWriter.
 * <b>Must not be @p NULL.</b>
 * @param n the amount of bytes to be written to the BufWriter.
 * @return @p BW_ERR_WRITE_FAIL if writing to the file occurs and fails,
 * otherwise @p BW_OK.
 */
BwOutcome bw_write_line(
    BufWriter* restrict self,
    char const* restrict buf,
    size_t n
);

/**
 * Writes a single character to the BufWriter.
 * If @p BUF_WRITER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>.
 * Amortized <tt>O(1)</tt> complexity.
 * @param self address of the BufWriter to which @p str shall be written to.
 * <b>Must not be @p NULL.</b>
 * @param c the character to be written to the BufWriter.
 * @return @p BW_ERR_WRITE_FAIL if a file write occurs and fails, otherwise
 * @p BW_OK.
 */
BwOutcome bw_write_char(BufWriter* self, char c);

/**
 * Flushes the BufWriter.
 * If @p BUF_WRITER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>.
 * <tt>O(self->pos)</tt> complexity.
 * @param self address of the BufWriter to be flushed.
 * <b>Must not be @p NULL.</b>
 * @return @p BW_ERR_WRITE_FAIL if a file write occurs and fails, otherwise
 * @p BW_OK.
 */
BwOutcome bw_flush(BufWriter* self);

#endif  // BUF_IO_BUF_WRITER_H
