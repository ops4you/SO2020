#include "parse_size.h"

#include <ctype.h>
#include <stdbool.h>

ParseSizeOutcome parse_size(
    char const* restrict str,
    size_t* const restrict size,
    char* const restrict opt_inv_char
) {
    while (*str && isspace(*str)) {
        ++str;
    }
    if (*str == '-') {
        return PARSE_SIZE_ERR_NEGATIVE;
    }
    bool digits_found = false;
    if (isdigit(*str)) {
        digits_found = true;
        *size = *str - '0';
        ++str;
    } else {
        if (opt_inv_char) {
            *opt_inv_char = *str;
        }
        return PARSE_SIZE_ERR_INV_CHAR;
    }
    while (*str && isdigit(*str)) {
        if (__builtin_mul_overflow(*size, 10, size) ||
            __builtin_add_overflow(*size, *str - '0', size)
        ) {
            return PARSE_SIZE_ERR_OUT_OF_RANGE;
        }
        ++str;
    }
    while (*str && isspace(*str)) {
        ++str;
    }
    if (*str) {
        if (opt_inv_char) {
            *opt_inv_char = *str;
        }
        return PARSE_SIZE_ERR_INV_CHAR;
    }
    if (!digits_found) {
        return PARSE_SIZE_ERR_NO_DIGITS;
    }
    return PARSE_SIZE_OK;
}

char const* parse_size_outcome_to_msg(ParseSizeOutcome const err) {
    switch (err) {
    case PARSE_SIZE_OK:
        return "success";
    case PARSE_SIZE_ERR_NEGATIVE:
        return "value cannot be negative";
    case PARSE_SIZE_ERR_INV_CHAR:
        return "invalid character";
    case PARSE_SIZE_ERR_NO_DIGITS:
        return "no digits found";
    case PARSE_SIZE_ERR_OUT_OF_RANGE:
        return "value could not be represented by an integral type";
    default:
        return "unknown parse error";
    }
}
