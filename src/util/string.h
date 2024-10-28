#pragma once

#include <stdbool.h>
#include <stddef.h> // IWYU pragma: keep
#include <string.h> // IWYU pragma: keep
#include <alloca.h> // IWYU pragma: keep

#define string_append_prefix(string, prefix)                         \
    ({                                                               \
        size_t string_len = strlen(string);                          \
        size_t prefix_len = strlen(prefix);                          \
                                                                     \
        size_t string_with_prefix_len = string_len + prefix_len + 1; \
        char* string_with_prefix = alloca(string_with_prefix_len);   \
        memset(string_with_prefix, 0, string_with_prefix_len);       \
                                                                     \
        memcpy(string_with_prefix, prefix, prefix_len);              \
        memcpy(string_with_prefix + prefix_len, string, string_len); \
                                                                     \
        string_with_prefix;                                          \
    })

bool string_ends_with(const char* str, const char* suffix);
