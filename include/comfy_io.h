#ifndef COMFY_IO_H
#define COMFY_IO_H

#include <stdio.h>

/**
 * Format prints to stderr.
 * Equivalent to calling <tt>fprintf(stderr, format, __VA_ARGS__)</tt>.
 */
#define eprintf(format, ...) fprintf(stderr, format, __VA_ARGS__)

/**
 * Format prints to stderr, and appends a newline. @p format must be a constant
 * expression. Equivalent to calling
 * <tt>fprintf(stderr, format "\n", __VA_ARGS__)</tt>.
 */
#define eprintln(format, ...) fprintf(stderr, format "\n", __VA_ARGS__)

/**
 * Prints an unformatted string to stderr, and appends a newline. Equivalent to
 * calling * <tt>fprintf(stderr, "%s\n", s)</tt>.
 */
#define eputs(s) eprintln("%s", s)

/**
 * Prints a single character to stderr. Equivalent to calling
 * <tt>putc(c, stderr)</tt>.
 */
#define eputc(c) putc(c, stderr)

#endif  // COMFY_IO_H
