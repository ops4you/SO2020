#ifndef COMFY_IO_H
#define COMFY_IO_H

#include <stdio.h>

/**
 * Format prints to stderr.
 */
#define eprintf(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)

/**
 * Format prints to stderr, and appends a newline. @p fmt must be a constant
 * expression.
 */
#define eprintln(fmt, ...) eprintf(fmt "\n", __VA_ARGS__)

/**
 * Prints an unformatted string to stderr, and appends a newline.
 */
#define eputs(s) eprintln("%s", s)

/**
 * Prints a single character to stderr.
 */
#define eputc(c) putc(c, stderr)

/**
 * Format prints to stderr, and prepends the program name.
 */
#define program_eprintf(fmt, ...) eprintf("%s: " fmt, program_name, __VA_ARGS__)

/**
 * Format prints to stderr, prepends the program name, and appends a newline.
 * @p fmt must be a constant expression.
 */
#define program_eprintln(fmt, ...) program_eprintf(fmt "\n", __VA_ARGS__)

/**
 * Prints an unformatted string to stderr, prepends the program name, and
 * appends a newline.
 */
#define program_eputs(s) program_eprintln("%s", s)

/**
 * Prints a single character to stderr, and prepends the program name.
 */
#define program_eputc(c) program_eprtinf("%c", c)

#endif  // COMFY_IO_H
