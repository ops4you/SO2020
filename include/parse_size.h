#ifndef PARSE_SIZE_H
#define PARSE_SIZE_H

#include <stddef.h>

/**
 * A parsing error that may occur when parsing sizes.
 */
typedef enum ParseSizeOutcome {
    PARSE_SIZE_OK,  //!< No error.
    PARSE_SIZE_ERR_NEGATIVE,    //!< Negative size value.
    PARSE_SIZE_ERR_INV_CHAR,    //!< Invalid character found.
    PARSE_SIZE_ERR_NO_DIGITS,   //!< String doesn't contain digits.
    PARSE_SIZE_ERR_OUT_OF_RANGE,//!< Size couldn't be represented as an integer.
} ParseSizeOutcome;

/**
 * Parses a size from a string.
 * The string must be null terminated, have at least one decimal digit,
 * and contain no characters besides digits and whitespace.
 * Any trailing whitespace is ignored.
 * @param str a null terminated string from which to parse the size.
 * <b>Must not be @p NULL</b>.
 * @param size (output parameter) pointer to a size type to which the parsed
 * size is stored. <b>Must not be @p NULL</b>.
 * @param opt_inv_char optional (output parameter) pointer to a character in
 * which to store the first invalid character, if found. If @p opt_inv_char is
 * @p NULL, it is unused.
 * @return If the first non whitespace character is a negative sign ('-'),
 * @p PARSE_SIZE_ERR_NEGATIVE is returned.
 * If the string contains any character besides digits or whitespace,
 * @p PARSE_SIZE_ERR_INV_CHAR is returned, and if @p opt_inv_char is not
 * @p NULL, it will store that invalid character.
 * If the string contains only whitespace, @p PARSE_SIZE_ERR_NO_DIGITS is
 * returned.
 * If the size can't be represented in @p size, @p PARSE_SIZE_ERR_OUT_OF_RANGE
 * is returned.
 * Otherwise, @p PARSE_SIZE_OK is returned.
 * Reading from @p size if the return value is different from @p PARSE_SIZE_OK
 * is undefined.
 */
ParseSizeOutcome parse_size(
    char const* restrict str,
    size_t* restrict size,
    char* restrict opt_inv_char
);

/**
 * Parses a size from a string slice.
 * The string must have at least one decimal digit, and contain no characters
 * besides digits and whitespace.
 * Any trailing whitespace is ignored.
 * @param begin a pointer to the first character of the string from which to
 * parse the size. <b>Must not be @p NULL</b>.
 * @param begin a pointer to the past-the-last character of the string from
 * which to parse the size. <b>Must not be @p NULL</b>.
 * @param size (output parameter) pointer to a size type to which the parsed
 * size is stored. <b>Must not be @p NULL</b>.
 * @param opt_inv_char optional (output parameter) pointer to a character in
 * which to store the first invalid character, if found. If @p opt_inv_char is
 * @p NULL, it is unused.
 * @return If the first non whitespace character is a negative sign ('-'),
 * @p PARSE_SIZE_ERR_NEGATIVE is returned.
 * If the string contains any character besides digits or whitespace,
 * @p PARSE_SIZE_ERR_INV_CHAR is returned, and if @p opt_inv_char is not
 * @p NULL, it will store that invalid character.
 * If the string contains only whitespace, @p PARSE_SIZE_ERR_NO_DIGITS is
 * returned.
 * If the size can't be represented in @p size, @p PARSE_SIZE_ERR_OUT_OF_RANGE
 * is returned.
 * Otherwise, @p PARSE_SIZE_OK is returned.
 * Reading from @p size if the return value is different from @p PARSE_SIZE_OK
 * is undefined.
 */
ParseSizeOutcome parse_size_slice(
    char const* begin,
    char const* end,
    size_t* restrict size,
    char* restrict opt_inv_char
);

/**
 * Returns the message associated with the ParseSizeOutcome.
 * @param outcome the ParseSizeOutcome whose associated message is returned. If
 * not a valid ParseSizeOutcome, a message describing that the outcome is
 * unknown is returned.
 * @return the message associated with the ParseSizeOutcome.
 */
char const* parse_size_outcome_msg(ParseSizeOutcome outcome);

#endif  // PARSE_SIZE_H
