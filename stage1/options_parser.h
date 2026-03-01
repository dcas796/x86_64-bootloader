#ifndef OPTIONS_PARSER_H
#define OPTIONS_PARSER_H

#include <types.h>

#define PARSER_KEY_MAX_LENGTH 64
#define PARSER_VALUE_MAX_LENGTH 256

typedef enum {
    PARSER_SUCCESS,
    PARSER_UNRECOGNIZED_KEY,
    PARSER_VALUE_TOO_LONG,
    PARSER_INCOMPATIBLE_LOAD_OFFSET,
    PARSER_SYNTAX_ERROR,
    PARSER_MISSING_KEY,
} parser_result_t;

typedef struct {
    char boot_binary[PARSER_VALUE_MAX_LENGTH + 1];
    uint32_t load_offset;
} boot_options_t;

parser_result_t parse(const char *text, boot_options_t *boot_options_out);
const char *parser_result_to_str(parser_result_t result);

#endif