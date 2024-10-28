#include "string.h"

#include <alloca.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

static void strtolwr(char* str)
{
    size_t len = strlen(str);
    for (size_t i = 0; i < len; ++i) {
        str[i] = tolower(str[i]);
    }
}

bool string_ends_with(const char* str, const char* suffix)
{
    size_t len_str = strlen(str);
    size_t len_suffix = strlen(suffix);

    if (len_suffix > len_str)
        return false;

    char* str_lower = alloca(len_str + 1);
    memcpy(str_lower, str, len_str);
    strtolwr(str_lower);

    char* suffix_lower = alloca(len_suffix + 1);
    memcpy(suffix_lower, suffix, len_suffix);
    strtolwr(suffix_lower);

    return strncmp(str_lower + len_str - len_suffix, suffix_lower, len_suffix) == 0;
}
