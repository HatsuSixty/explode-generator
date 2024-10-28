#include "explode.h"

#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <raylib.h>

// Implementation already defined in main file.
// #define ARENA_IMPLEMENTATION
#include "external/arena.h"

#include "resources/explode_frames.h"

#include "resize.h"
#include "gif_save.h"

static void* resize_pixels(Arena* arena,
                           void* pixels, int old_width, int old_height,
                           int new_width, int new_height)
{
    void* new_pixels = arena_alloc(arena, new_width * new_height * sizeof(uint32_t));

    image_resize(pixels, old_width, old_height,
                 new_pixels, new_width, new_height);

    return new_pixels;
}

static void* explode_image_and_copy(Arena* arena, Image image, float level)
{
    Image copy = image;
    copy.data = arena_alloc(arena, image.width * image.height * sizeof(uint32_t));
    memcpy(copy.data, image.data, image.width * image.height * sizeof(uint32_t));
    image_explode(&copy, level);
    return copy.data;
}

void image_explode(Image* image, float level)
{
    if (level <= 0)
        return;

    int width = image->width;
    int height = image->height;
    int cx = width / 2;
    int cy = height / 2;
    float max_radius = fmin(width, height) / 2.0f;

    uint32_t* original_data = (uint32_t*)malloc(width * height * sizeof(uint32_t));
    memcpy(original_data, image->data, width * height * sizeof(uint32_t));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float dx = (float)(x - cx);
            float dy = (float)(y - cy);
            float distance = sqrtf(dx * dx + dy * dy);

            float factor = 1.0f;
            if (distance < max_radius) {
                float normalized_distance = distance / max_radius;
                factor = powf(normalized_distance, level);
            }

            int distorted_x = cx + (int)(dx * factor);
            int distorted_y = cy + (int)(dy * factor);
            distorted_x = fmax(0, fmin(distorted_x, width - 1));
            distorted_y = fmax(0, fmin(distorted_y, height - 1));

            uint32_t color = original_data[distorted_y * width + distorted_x];
            ((uint32_t*)image->data)[y * image->width + x] = color;
        }
    }

    free(original_data);
}

void image_to_explode_gif(Image image, const char* output)
{
    Arena arena = { 0 };

    void* gif_frame_data[] = {
        image.data,
        explode_image_and_copy(&arena, image, 0.125f),
        explode_image_and_copy(&arena, image, 0.250f),
        explode_image_and_copy(&arena, image, 0.375f),
        explode_image_and_copy(&arena, image, 0.500f),
        explode_image_and_copy(&arena, image, 0.625f),
        explode_image_and_copy(&arena, image, 0.750f),
        explode_image_and_copy(&arena, image, 0.875f),
        explode_image_and_copy(&arena, image, 1.000f),
        resize_pixels(&arena,
                      explode_frame_00_data, explode_frame_00_width, explode_frame_00_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_01_data, explode_frame_01_width, explode_frame_01_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_02_data, explode_frame_02_width, explode_frame_02_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_03_data, explode_frame_03_width, explode_frame_03_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_04_data, explode_frame_04_width, explode_frame_04_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_05_data, explode_frame_05_width, explode_frame_05_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_06_data, explode_frame_06_width, explode_frame_06_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_07_data, explode_frame_07_width, explode_frame_07_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_08_data, explode_frame_08_width, explode_frame_08_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_09_data, explode_frame_09_width, explode_frame_09_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_10_data, explode_frame_10_width, explode_frame_10_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_11_data, explode_frame_11_width, explode_frame_11_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_12_data, explode_frame_12_width, explode_frame_12_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_13_data, explode_frame_13_width, explode_frame_13_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_14_data, explode_frame_14_width, explode_frame_14_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_15_data, explode_frame_15_width, explode_frame_15_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_16_data, explode_frame_16_width, explode_frame_16_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_17_data, explode_frame_17_width, explode_frame_17_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_18_data, explode_frame_18_width, explode_frame_18_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_19_data, explode_frame_19_width, explode_frame_19_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_20_data, explode_frame_20_width, explode_frame_20_height,
                      image.width, image.height),
        resize_pixels(&arena,
                      explode_frame_21_data, explode_frame_21_width, explode_frame_21_height,
                      image.width, image.height),
    };

    GifFrames gif_frames = {
        .frames = gif_frame_data,
        .frames_count = sizeof(gif_frame_data) / sizeof(gif_frame_data[0]),
        .width = image.width,
        .height = image.height,
    };

    gif_save(gif_frames, output);

    arena_free(&arena);
}
