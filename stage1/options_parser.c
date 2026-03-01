#include "options_parser.h"

#include "mem.h"
#include "string.h"

#define BOOT_OPTIONS_BOOT_BINARY_KEY "boot_binary"
#define BOOT_OPTIONS_LOAD_OFFSET_KEY "load_offset"

parser_result_t update_options(const char *key, const char *value, boot_options_t *boot_options_out) {
    size_t key_len = strlen(key);
    size_t value_len = strlen(value);
    if (key_len > PARSER_KEY_MAX_LENGTH) return PARSER_UNRECOGNIZED_KEY;
    if (value_len > PARSER_VALUE_MAX_LENGTH) return PARSER_VALUE_TOO_LONG;

    if (streq(key, BOOT_OPTIONS_BOOT_BINARY_KEY)) {
        memcpy(&boot_options_out->boot_binary, value, value_len);
        boot_options_out->boot_binary[value_len] = '\0';
    } else if (streq(key, BOOT_OPTIONS_LOAD_OFFSET_KEY)) {
        uint8_t base;
        uint8_t offset;
        if (value_len > 2 &&
            value[0] == '0' && value[1] == 'x') {
            base = 16;
            offset = 2;
            } else {
                base = 10;
                offset = 0;
            }
        if (!atoi(value + offset, base, &boot_options_out->load_offset)) {
            return PARSER_SYNTAX_ERROR;
        }
    } else {
        return PARSER_UNRECOGNIZED_KEY;
    }

    return PARSER_SUCCESS;
}

parser_result_t parse(const char *text, boot_options_t *boot_options_out) {
    uint32_t i = 0;

    char key[PARSER_KEY_MAX_LENGTH + 1];
    char value[PARSER_VALUE_MAX_LENGTH + 1];
    key[0] = '\0';
    value[0] = '\0';

    uint32_t j = 0;
    bool in_key = true;
    while (text[i] != '\0') {
        switch (text[i]) {
            case '\n':
                value[j] = '\0';
                parser_result_t result = update_options(key, value, boot_options_out);
                if (result != PARSER_SUCCESS) return result;
                j = 0;
                in_key = true;
                break;
            case '\r':
                break;
            case '=':
                key[j] = '\0';
                j = 0;
                in_key = false;
                break;
            default:
                if (in_key) {
                    if (j > PARSER_KEY_MAX_LENGTH) return PARSER_UNRECOGNIZED_KEY;
                    key[j] = text[i];
                } else {
                    if (j > PARSER_VALUE_MAX_LENGTH) return PARSER_VALUE_TOO_LONG;
                    value[j] = text[i];
                }
                ++j;
        }
        ++i;
    }

    if (strlen(boot_options_out->boot_binary) == 0) return PARSER_MISSING_KEY;

    return PARSER_SUCCESS;
}

const char *parser_result_to_str(parser_result_t result) {
    switch (result) {
        case PARSER_SUCCESS:
            return "Success";
        case PARSER_UNRECOGNIZED_KEY:
            return "Unrecognized key";
        case PARSER_VALUE_TOO_LONG:
            return "Value too long";
        case PARSER_INCOMPATIBLE_LOAD_OFFSET:
            return "Incompatible load offset";
        case PARSER_SYNTAX_ERROR:
            return "Syntax error";
        case PARSER_MISSING_KEY:
            return "Missing key";
        default:
            return "Unknown";
    }
}


