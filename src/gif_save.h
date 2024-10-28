#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    void** frames;
    size_t frames_count;
    int width;
    int height;
} GifFrames;

bool gif_save(GifFrames frames, const char* output_file);
