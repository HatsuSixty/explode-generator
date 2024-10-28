#pragma once

#define magick_log_wand_exception(wand)                                                \
    {                                                                                  \
        ExceptionType severity;                                                        \
        char* description = MagickGetException(wand, &severity);                       \
                                                                                       \
        fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, __func__, description); \
                                                                                       \
        description = (char*)MagickRelinquishMemory(description);                      \
    }
