#include <assert.h>
#include <errno.h>
#include <libgen.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>
#include <raymath.h>

#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

#define ARENA_IMPLEMENTATION
#include "external/arena.h"

#include "resources/font.h"

#include "explode.h"
#include "gif_load.h"
#include "util/string.h"

#define BACKGROUND_COLOR GetColor(0x181818FF)
#define HIGHLIGHTED_BACKGROUND_COLOR ColorBrightness(BACKGROUND_COLOR, .1f)
#define BUTTON_COLOR ColorBrightness(HIGHLIGHTED_BACKGROUND_COLOR, .1f)
#define BUTTON_HIGHLIGHT_COLOR ColorBrightness(BUTTON_COLOR, .1f)
#define BUTTON_PRESSED_COLOR ColorBrightness(BUTTON_HIGHLIGHT_COLOR, .1f)

#define MIN_FONT_SIZE 8
#define MAX_FONT_SIZE 80
#define FONT_ARRAY_SIZE (MAX_FONT_SIZE - MIN_FONT_SIZE + 1)
Font fonts[FONT_ARRAY_SIZE] = { 0 };

void fonts_load()
{
    for (size_t i = 0; i < FONT_ARRAY_SIZE; ++i) {
        size_t font_size = MIN_FONT_SIZE + i;
        fonts[i] = LoadFontFromMemory(".ttf", font_ttf_bytes, sizeof(font_ttf_bytes), font_size, NULL, 0);
    }
}

void fonts_unload()
{
    for (size_t i = 0; i < FONT_ARRAY_SIZE; ++i) {
        UnloadFont(fonts[i]);
    }
}

Image load_image(const char* filename)
{
    Image image;
    int channels;
    image.data = stbi_load(filename, &image.width, &image.height, &channels, 4);
    image.mipmaps = 1;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    return image;
}

void draw_text_centered_area(const char* text, int font_size, int y, Rectangle area)
{
    const Font font = fonts[(size_t)(Clamp(font_size, MIN_FONT_SIZE, MAX_FONT_SIZE) - MIN_FONT_SIZE)];

    const int font_spacing = 2;

    const Vector2 font_dims = MeasureTextEx(font, text, font_size, font_spacing);

    DrawTextEx(font,
               text,
               (Vector2) {
                   .x = area.x + area.width / 2.f - font_dims.x / 2.f,
                   .y = y == 0
                       ? area.y + area.height / 2.f - font_dims.y / 2.f
                       : y,
               },
               font_size,
               font_spacing,
               WHITE);
}

void draw_text_centered(const char* text, int font_size, int y)
{
    draw_text_centered_area(text, font_size, y, (Rectangle) {
                                                    .x = 0,
                                                    .y = 0,
                                                    .width = GetScreenWidth(),
                                                    .height = GetScreenHeight(),
                                                });
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

    Arena arena = { 0 };

    void** frames = arena_alloc(&arena, sizeof(void*) * frames_info.count);
    for (size_t i = 0; i < frames_info.count; ++i) {
        frames[i] = arena_alloc(&arena, frames_info.width * frames_info.height * sizeof(uint32_t));
    }
    gif_load(input_path, frames, frames_info.count);

    Textures textures = { 0 };
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
    if (textures.textures)
        free(textures.textures);
}

int selector(const char* options[], size_t options_count, size_t option_selected,
             const char* title, Rectangle area)
{
    int clicked_option = -1;

    DrawRectangleRounded(area, 0.1, 10, HIGHLIGHTED_BACKGROUND_COLOR);

    const float padding = 7;
    const int title_font_size = 40;

    const float title_y = area.y + padding;
    draw_text_centered_area(title, title_font_size, title_y, area);

    const float button_height = 20;
    float button_y = title_y + title_font_size + padding;
    for (size_t i = 0; i < options_count; ++i) {
        const char* button_label = options[i];
        if (i == option_selected) {
            button_label = string_append_prefix(" (selected)", options[i]);
        }

        Rectangle button_area = (Rectangle) {
            .x = area.x + padding,
            .y = button_y,
            .width = area.width - padding * 2,
            .height = button_height,
        };
        Color button_color = CheckCollisionPointRec(GetMousePosition(), button_area)
            ? (IsMouseButtonDown(MOUSE_BUTTON_LEFT)
                   ? BUTTON_PRESSED_COLOR
                   : BUTTON_HIGHLIGHT_COLOR)
            : BUTTON_COLOR;
        DrawRectangleRounded(button_area, 0.4, 10, button_color);
        button_y += button_height + padding;

        const float button_label_size = title_font_size * 0.4;
        draw_text_centered_area(button_label, button_label_size,
                                button_area.y + button_height / 2.f - button_label_size / 2.f,
                                button_area);

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)
            && CheckCollisionPointRec(GetMousePosition(), button_area))
            clicked_option = i;
    }

    return Clamp(clicked_option, -1, options_count);
}

typedef enum {
    EMOJI_KIND_EXPLODE = 0,
    EMOJI_KIND_IMPLODE,
    COUNT_EMOJI_KINDS,
} EmojiKind;

typedef enum {
    EMOJI_FORMAT_GIF = 0,
    EMOJI_FORMAT_PNG,
    COUNT_EMOJI_FORMATS,
} EmojiFormat;

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "Explode Generator");

    fonts_load();

    bool emoji_customized = false;
    EmojiFormat emoji_format = EMOJI_FORMAT_PNG;
    EmojiKind emoji_kind = EMOJI_KIND_EXPLODE;

    Textures animation_frames = { 0 };

    char* animation_path = NULL;

    double gif_animation_duration = 0;
    double gif_animation_start = 0;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);

        const int text_padding = 10;
        const int text_big_size = GetScreenWidth() * 0.05;
        const int text_medium_size = text_big_size * 0.75;
        const int text_small_size = text_big_size * 0.5;

        if (!emoji_customized) {
            draw_text_centered("Customize your emoji!", text_big_size, 10);

            const float padding = 10;
            const float selector_height = 150;
            const float selector_width = 400;

            // Area for the selector of emoji format
            const float format_selector_y
                = (GetScreenHeight() / 2.f) - ((selector_height + padding + selector_height) / 2.f);
            const Rectangle format_selector_area = (Rectangle) {
                .x = GetScreenWidth() / 2.f - selector_width / 2.f,
                .y = format_selector_y,
                .width = selector_width,
                .height = selector_height,
            };

            // Area for the selector of emoji kind
            const float kind_selector_y = format_selector_y + selector_height + padding;
            const Rectangle kind_selector_area = (Rectangle) {
                .x = GetScreenWidth() / 2.f - selector_width / 2.f,
                .y = kind_selector_y,
                .width = selector_width,
                .height = selector_height,
            };

            // Selector for emoji format
            int selected_format
                = selector((const char*[]) { "GIF", "Animated PNG" }, COUNT_EMOJI_FORMATS,
                           emoji_format, "Emoji format:", format_selector_area);
            if (selected_format != -1)
                emoji_format = selected_format;

            // Selector for emoji kind
            int selected_kind
                = selector((const char*[]) { "Explode", "Implode" }, COUNT_EMOJI_KINDS,
                           emoji_kind, "Emoji kind:", kind_selector_area);
            if (selected_kind != -1)
                emoji_kind = selected_kind;

            // Done button
            {
                const float done_button_distance_from_bottom = 40;
                const float done_button_width = 100;
                const float done_button_height = 40;
                Rectangle done_button_area = {
                    .x = GetScreenWidth() / 2.f - done_button_width / 2.f,
                    .y = GetScreenHeight() - done_button_distance_from_bottom - done_button_height,
                    .width = done_button_width,
                    .height = done_button_height,
                };

                Color done_button_color = CheckCollisionPointRec(GetMousePosition(), done_button_area)
                    ? (IsMouseButtonDown(MOUSE_BUTTON_LEFT)
                           ? BUTTON_PRESSED_COLOR
                           : BUTTON_HIGHLIGHT_COLOR)
                    : BUTTON_COLOR;
                DrawRectangleRounded(done_button_area, 0.4, 10, done_button_color);

                draw_text_centered_area("Done!", 20, 0, done_button_area);

                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)
                    && CheckCollisionPointRec(GetMousePosition(), done_button_area))
                    emoji_customized = true;
            }

            EndDrawing();
            continue;
        }

        /*                                  *
         *   Update: Handle dropped files   *
         *                                  */

        FilePathList dropped_files = LoadDroppedFiles();
        if (dropped_files.count > 0) {
            // Draw loading text
            draw_text_centered("Loading...", 40, 0);
            EndDrawing();
            BeginDrawing();

            // Get input and output path
            const char* input_path = dropped_files.paths[0];
            const char* output_path;
            switch (emoji_format) {
            case EMOJI_FORMAT_GIF:
                output_path = string_append_prefix("_out.gif", input_path);
                break;
            case EMOJI_FORMAT_PNG:
                output_path = string_append_prefix("_out.png", input_path);
                break;
            default:
                output_path = string_append_prefix("_out.png", input_path);
            }

            // Generate the gif
            {
                Image exploding_image = load_image(input_path);
                if (exploding_image.data == NULL) {
                    fprintf(stderr, "ERROR: failed to load file `%s`: %s\n", input_path, strerror(errno));
                    goto continue_after_dropped_files;
                }

                bool reverse_gif;
                switch (emoji_kind) {
                case EMOJI_KIND_EXPLODE:
                    reverse_gif = false;
                    break;
                case EMOJI_KIND_IMPLODE:
                    reverse_gif = true;
                    break;
                default:
                    reverse_gif = false;
                }
                image_to_explode_gif(exploding_image, output_path, reverse_gif);

                UnloadImage(exploding_image);
            }

            // Destroy old textures
            textures_destroy(animation_frames);

            // Replace with new ones
            animation_frames = textures_from_gif(output_path);
            animation_path = basename((char*)output_path);
            gif_animation_start = GetTime();
            gif_animation_duration = animation_frames.count * 1.f / 25.f;
        }
    continue_after_dropped_files:
        UnloadDroppedFiles(dropped_files);

        /*          *
         *   Draw   *
         *          */

        if (animation_frames.count != 0) {
            // Draw text
            draw_text_centered("Image generated!", text_big_size, text_padding);
            draw_text_centered(TextFormat("Image path: %s", animation_path), text_small_size,
                               text_padding + text_big_size + 3);
            draw_text_centered("Drag & Drop to generate a new image!", text_medium_size,
                               GetScreenHeight() - text_padding - text_medium_size);

            // Get animation frame
            float gif_animation_factor = (GetTime() - gif_animation_start) / gif_animation_duration;
            if (gif_animation_factor >= 1) {
                gif_animation_start = GetTime();
                gif_animation_factor = 0;
            }
            Texture animation_frame
                = animation_frames.textures[(int)floorf(gif_animation_factor * animation_frames.count)];

            // Draw frame background
            const float frame_background_padding = 10.f;
            Rectangle frame_background_rectangle = {
                .x = GetScreenWidth() / 2.f - animation_frame.width / 2.f - frame_background_padding,
                .y = GetScreenHeight() / 2.f - animation_frame.height / 2.f - frame_background_padding,
                .width = animation_frame.width + frame_background_padding * 2,
                .height = animation_frame.height + frame_background_padding * 2,
            };
            DrawRectangleRounded(frame_background_rectangle,
                                 0.1f, 10, HIGHLIGHTED_BACKGROUND_COLOR);

            // Draw frame
            Rectangle animation_frame_rectangle = {
                .x = GetScreenWidth() / 2.f - animation_frame.width / 2.f,
                .y = GetScreenHeight() / 2.f - animation_frame.height / 2.f,
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

        } else {
            draw_text_centered("Drag & Drop some image!", 40, 0);
        }

        EndDrawing();
    }

    textures_destroy(animation_frames);

    fonts_unload();

    CloseWindow();

    return 0;
}
