#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#define ARENA_IMPLEMENTATION
#include "external/arena.h"

#include "explode.h"
#include "gif_load.h"

Image load_image(const char* filename)
{
    Image image;
    int channels;
    image.data = stbi_load(filename, &image.width, &image.height, &channels, 4);
    image.mipmaps = 1;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    return image;
}

typedef struct {
    Texture* textures;
    size_t count;
} Textures;

Textures textures_from_gif(const char* input_path)
{
    GifFramesInfo frames_info = gif_get_frames_info(input_path);
    printf("GIF info:\n");
    printf("  > Frame count: %zu\n", frames_info.count);
    printf("  > Frame width: %d\n", frames_info.width);
    printf("  > Frame height: %d\n", frames_info.height);

    Arena arena = {0};

    void** frames = arena_alloc(&arena, sizeof(void*) * frames_info.count);
    for (size_t i = 0; i < frames_info.count; ++i) {
        frames[i] = arena_alloc(&arena, frames_info.width * frames_info.height * sizeof(uint32_t));
    }
    gif_load(input_path, frames, frames_info.count);

    Textures textures = {0};
    textures.count = frames_info.count;
    textures.textures = malloc(sizeof(*textures.textures) * frames_info.count);
    for (size_t i = 0; i < frames_info.count; ++i) {
        textures.textures[i] = LoadTextureFromImage((Image) {
            .data = frames[i],
            .width = frames_info.width,
            .height = frames_info.height,
            .mipmaps = 1,
            .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        });
    }

    arena_free(&arena);

    return textures;
}

void textures_destroy(Textures textures)
{
    for (size_t i = 0; i < textures.count; ++i) {
        UnloadTexture(textures.textures[i]);
    }
    free(textures.textures);
}

int main(void)
{
    const char* input_path = "resources/neocat_floof.png";
    const char* output_path = "neocat_floof_explode.png";

    Image exploding_image = load_image(input_path);
    if (exploding_image.data == NULL) {
        fprintf(stderr, "ERROR: failed to load file `%s`: %s\n", input_path, strerror(errno));
        return 1;
    }
    image_to_explode_gif(exploding_image, output_path);
    UnloadImage(exploding_image);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(640, 480, "Explode Generator");

    Textures animation_frames = textures_from_gif(output_path);

    double gif_animation_duration = animation_frames.count * 1.f / 25.f;
    double gif_animation_start = GetTime();

    SetTraceLogLevel(LOG_WARNING);
    while (!WindowShouldClose()) {
        float gif_animation_factor = (GetTime() - gif_animation_start) / gif_animation_duration;
        if (gif_animation_factor >= 1) {
            gif_animation_start = GetTime();
            gif_animation_factor = 0;
        }

        Texture animation_frame
            = animation_frames.textures[(int)floorf(gif_animation_factor * animation_frames.count)];

        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));

        const float frame_background_padding = 10.f;
        Rectangle frame_background_rectangle = {
            .x = GetScreenWidth() / 2.f - animation_frame.width / 2.f - frame_background_padding,
            .y = GetScreenHeight() / 2.f - animation_frame.width / 2.f - frame_background_padding,
            .width = animation_frame.width + frame_background_padding * 2,
            .height = animation_frame.height + frame_background_padding * 2,
        };
        DrawRectangleRounded(frame_background_rectangle,
                             0.1f, 10, ColorBrightness(GetColor(0x181818FF), .1f));

        Rectangle animation_frame_rectangle = {
            .x = GetScreenWidth() / 2.f - animation_frame.width / 2.f,
            .y = GetScreenHeight() / 2.f - animation_frame.width / 2.f,
            .width = animation_frame.width,
            .height = animation_frame.height,
        };
        DrawTexturePro(animation_frame,
                       (Rectangle) {
                           0,
                           0,
                           animation_frame.width,
                           animation_frame.height,
                       },
                       animation_frame_rectangle,
                       (Vector2) { 0 },
                       0.0f, WHITE);

        EndDrawing();
    }
    SetTraceLogLevel(LOG_INFO);

    textures_destroy(animation_frames);

    CloseWindow();

    return 0;
}
