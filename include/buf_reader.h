#ifndef BUF_IO_BUF_READER_H
#define BUF_IO_BUF_READER_H

#include <stddef.h>

/**
 * If set to @p 1, perform runtime assertions on possibly invalid function
 * arguments.
 */
#define BUF_READER_RUNTIME_ASSERTS 0

/**
 * A wrapper around a file descriptor opened with read mode that provides
 * buffered reading.
 */
typedef struct BufReader {
    int file_des;   //!< The underlying file descriptor.
    char* buf;  //!< The underlying intermediary circular buffer.
    size_t begin;   //!< The index of the first character of the buffer.
    size_t end; //!< The index of the past the end character of the buffer.
    size_t cap; //!< The buffer's maximum capacity.
} BufReader;

/**
 * An outcome returned by BufReader functions.
 */
typedef enum BrOutcome {
    BR_OK,  //!< No error
    // BR_EOF, //!< End of file reached while reading from the file.
    BR_ERR_ALLOC_FAIL,  //!< Error occurred while allocating dynamic memory.
    BR_ERR_READ_FAIL,   //!< Error occurred while reading from a file.
    BR_ERR_CLOSE_FAIL,  //!< Error occurred while closing a file.
} BrOutcome;

/**
 * Creates a BufReader with the default capacity from a file descriptor.
 * The current default capacity is 8192 bytes, but may change in the future.
 * The underlying buffer is allocated dynamically with <tt>malloc(3)</tt>.
 * If @p BUF_READER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * made:
 * 1. <tt>assert(init != NULL)</tt>;
 * 2. <tt>assert(file_des > 0)</tt>.
 * <tt>O(malloc(default capacity))</tt> complexity.
 * @param init (output parameter) address of the BufReader to initialize.
 * <b>Must not be @p NULL.</b>
 * @param file_des the file descriptor to be associated with the BufReader.
 * <b>Must be a valid file descriptor opened with read mode.</b>
 * @return @p BR_ERR_ALLOC_FAIL if memory allocation fails, otherwise @p BR_OK.
 */
BrOutcome br_with_deafault_cap(BufReader* init, int file_des);

/**
 * Creates a BufReader with the provided capacity from a file descriptor.
 * The underlying buffer is allocated dynamically with <tt>malloc(3)</tt>.
 * If @p BUF_READER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * made:
 * 1. <tt>assert(init != NULL)</tt>;
 * 2. <tt>assert(file_des > 0)</tt>;
 * 3. <tt>assert(capacity > 0ul)</tt>.
 * <tt>O(malloc(capacity))</tt> complexity.
 * @param init (output parameter) address of the BufReader to initialize.
 * <b>Must not be @p NULL.</b>
 * @param file_des the file descriptor to be associated with the BufReader.
 * <b>Must be a valid file descriptor opened with read mode.</b>
 * @param capacity the desired capacity of the BufReader.
 * <b>Must be positive.</b>
 * @return @p BR_ERR_ALLOC_FAIL if memory allocation fails, otherwise @p BR_OK.
 */
BrOutcome br_with_cap(BufReader* init, int file_des, size_t capacity);

/**
 * Creates a BufReader from a file descriptor, that uses the provided buffer as
 * its underlying buffer.
 * If @p BUF_READER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * made:
 * 1. <tt>assert(init != NULL)</tt>;
 * 2. <tt>assert(file_des > 0)</tt>;
 * 3. <tt>assert(buf != NULL)</tt>;
 * 4. <tt>assert(buf_size > 0ul).
 * <tt>O(1)</tt> complexity.
 * @param init (output parameter) address of the BufReader to initialize.
 * <b>Must not be @p NULL.</b>
 * @param file_des the file descriptor to be associated with the BufReader.
 * <b>Must be a valid file descriptor opened with read mode.</b>
 * @param buf the buffer to be used as the BufReader's intermediary buffer.
 * <b>Must not be @p NULL.</b>
 * @param buf_size the size of the provided buffer that will be used as the
 * BufReader's capacity.
 * <b>Must be positive.</b>
 */
void br_with_buf(
    BufReader* restrict init,
    int file_des,
    char* restrict buf,
    size_t buf_size
);

/**
 * Deallocates the BufReader's underlying buffer using <tt>free(3)</tt>, but
 * doesn't close the associated file.
 * <b>Attempts to read using the BufReader with the deallocated buffer result in
 * undefined behaviour.</b>
 * If @p BUF_READER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * made:
 * 1. <tt>assert(self != NULL)</tt>.
 * <tt>O(free(self->buf))</tt> complexity.
 * @param self address of the BufReader whose buffer shall be deallocated.
 * <b>Must not be @p NULL.</b>
 */
void br_drop(BufReader* self);

/**
 * Closes the BufReader's associated file using <tt>close(2)</tt>, but doesn't
 * deallocate the underlying buffer.
 * <b>Attempts to read using the BufReader with the closed file result in
 * undefined behaviour, if a file read occurs.</b>
 * If @p BUF_READER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * made:
 * 1. <tt>assert(self != NULL)</tt>.
 * <tt>O(close(self->file_des))</tt> complexity.
 * @param self address of the BufReader whose file shall be closed.
 * <b>Must not be @p NULL.</b>
 * @return @p BR_ERR_CLOSE_FAIL if closing fails, otherwise @p BR_OK.
 */
BrOutcome br_close(BufReader* self);

/**
 * Deallocates the BufReader's underlying buffer using <tt>free(3)</tt> and
 * closes its associated file using <tt>close(2)</tt>.
 * <b>Attempts to read using the BufReader with the deallocated buffer and
 * closed file result in undefined behaviour.</b>
 * If @p BUF_READER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * made:
 * 1. <tt>assert(self != NULL)</tt>.
 * <tt>O(free(self->buf) + close(self->file_des))</tt> complexity.
 * @param self address of the BufReader whose buffer shall be deallocated and
 * file shall be closed.
 * <b>Must not be @p NULL.</b>
 * @return @p BR_ERR_CLOSE_FAIL if closing fails, otherwise @p BW_OK.
 */
BrOutcome br_drop_and_close(BufReader* self);

/**
 * Replaces the BufReader's underlying buffer with the provided buffer, and its
 * capacity with the new buffer's size, and returns the old buffer.
 * If @p BUF_READER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>;
 * 2. <tt>assert(new_buf != NULL)</tt>.
 * 3. <tt>assert(new_buf_size > 0ul)</tt>.
 * <tt>O(1)</tt> complexity.
 * @param self address of the BufReader whose underlying buffer shall be
 * replaced.
 * <b>Must not be @p NULL.</b>
 * @param new_buf the buffer to replace the BufReader's buffer.
 * <b>Must not be @p NULL.</b>
 * @param new_buf_size the size of the new buffer. <b>Must be positive.</b>
 * @return the BufReader's old buffer.
 */
char* br_replace_buf(
    BufReader* restrict self,
    char* restrict new_buf,
    size_t new_buf_size
);

/**
 * Replaces the BufReader's underlying file descriptor with the provided one,
 * and returns the old descriptor.
 * If @p BUF_READER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>;
 * 2. <tt>assert(new_file_des > 0)</tt>.
 * <tt>O(1)</tt> complexity.
 * @param self address of the BufReader whose underlying file descriptor shall
 * be replaced. <b>Must not be @p NULL.</b>
 * @param new_file_des the file descriptor to replace the BufReader's file
 * descriptor.
 * <b>Must be a valid file descriptor opened with read mode.</b>
 * @return the BufReader's old file descriptor.
 */
int br_replace_file(BufReader* self, int new_file_des);

/**
 * Returns the BufReader's underlying file descriptor.
 * The caller is expected to not mutate the file.
 * If @p BUF_READER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>.
 * <tt>O(1)</tt> complexity.
 * @param self address of the BufReader whose underlying file descriptor shall
 * be returned. <b>Must not be @p NULL.</b>
 * @return the BufReader's underlying file descriptor.
 */
int br_descriptor(BufReader const* self);

/**
 * Returns the BufReader's underlying file descriptor.
 * If @p BUF_READER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>.
 * <tt>O(1)</tt> complexity.
 * @param self address of the BufReader whose underlying file descriptor shall
 * be returned. <b>Must not be @p NULL.</b>
 * @return the BufReader's underlying file descriptor.
 */
int br_descriptor_mut(BufReader* self);

/**
 * Returns the amount of available bytes in the BufReader's buffer, i.e. the
 * amount of bytes that can be read from the BufReader without causing a file
 * read.
 * If @p BUF_READER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>.
 * <tt>O(1)</tt> complexity.
 * @param self address of the BufReader whose buffer's position shall be
 * returned. <b>Must not be @p NULL.</b>
 * @return the amount of available bytes in the BufReader's buffer.
 */
size_t br_available_bytes(BufReader const* self);

/**
 * Returns the BufReader's maximum capacity.
 * The current default capacity is 8192 bytes, but may change in the future.
 * If @p BUF_READER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>.
 * <tt>O(1)</tt> complexity.
 * @param self address of the BufReader whose capacity shall be returned.
 * <b>Must not be @p NULL.</b>
 * @return the BufReader's maximum capacity.
 */
size_t br_cap(BufReader const* self);

/**
 * Reads at most @p n bytes to @p buf. Reading stops after an EOF.
 * The amount of bytes read is stored in @p read_bytes.
 * If @p BUF_READER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>;
 * 2. <tt>assert(buf != NULL)</tt>.
 * 3. <tt>assert(read_bytes != NULL)</tt>.
 * Amortized <tt>O(n)</tt> complexity.
 * @param self address of the BufReader from which @p buf shall be read to.
 * <b>Must not be @p NULL.</b>
 * @param buf (output parameter) the buffer to be read from the BufReader.
 * <b>Must not be @p NULL.</b>
 * @param n the amount of bytes to be read from the BufReader.
 * @param read_bytes (output parameter) pointer to a size type to which the
 * amount of bytes read is stored. <b>Must not be @p NULL.</b>
 * @return @p BR_ERR_READ_FAIL if a file read occurs and fails, otherwise
 * @p BR_OK.
 */
BrOutcome br_read(
    BufReader* restrict self,
    char* restrict buf,
    size_t n,
    size_t* restrict read_bytes
);

/**
 * Reads at most @p n bytes to @p buf. Reading stops after an EOF or a newline.
 * If a newline is read, it is not stored into the buffer.
 * The amount of bytes read is stored in @p read_bytes.
 * If @p BUF_READER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>;
 * 2. <tt>assert(buf != NULL)</tt>.
 * 3. <tt>assert(read_bytes != NULL)</tt>.
 * Amortized <tt>O(n)</tt> complexity.
 * @param self address of the BufReader from which @p buf shall be read to.
 * <b>Must not be @p NULL.</b>
 * @param buf (output parameter) the buffer to be read to the BufReader.
 * <b>Must not be @p NULL.</b>
 * @param n the maximum amount of bytes to be read from the BufReader.
 * @param read_bytes (output parameter) pointer to a size type to which the
 * amount of bytes read is stored. <b>Must not be @p NULL.</b>
 * @return @p BR_ERR_READ_FAIL if a file read occurs and fails, otherwise
 * @p BW_OK.
 */
BrOutcome br_read_line(
    BufReader* restrict self,
    char* restrict buf,
    size_t n,
    size_t* restrict read_bytes
);

/**
 * Reads a single character to @p c.
 * If @p BUF_READER_RUNTIME_ASSERTS is set to @p 1, the following assertions are
 * are made:
 * 1. <tt>assert(self != NULL)</tt>.
 * 2. <tt>assert(c != NULL)</tt>.
 * Amortized <tt>O(1)</tt> complexity.
 * @param self address of the BufReader from which @p c shall be read to.
 * <b>Must not be @p NULL.</b>
 * @param c (output parameter) pointer to a character to which the character
 * shall be read.
 * <b>Must not be @p NULL.</b>
 * @return @p BR_ERR_READ_FAIL if a file read occurs and fails, otherwise
 * @p BW_OK.
 */
BrOutcome br_read_char(BufReader* restrict self, char* restrict c);

#endif  // BUF_IO_BUF_READER_H
