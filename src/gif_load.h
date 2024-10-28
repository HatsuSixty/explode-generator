#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    size_t count;
    int width;
    int height;
} GifFramesInfo;

GifFramesInfo gif_get_frames_info(const char* input_file);
bool gif_load(const char* input_file,
              void** frames, size_t frames_count);
