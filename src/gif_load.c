#include "gif_load.h"

#include <stdbool.h>

#include "util/magick.h"
#include "util/string.h"

#include <MagickWand/MagickWand.h>

GifFramesInfo gif_get_frames_info(const char* input_file)
{
    if (string_ends_with(input_file, ".apng") || string_ends_with(input_file, ".png")) {
        input_file = string_append_prefix(input_file, "APNG:");
    }

    GifFramesInfo info = {0};

    MagickWandGenesis();

    MagickWand* wand = NewMagickWand();
    MagickReadImage(wand, input_file);
    info.width = MagickGetImageWidth(wand);
    info.height = MagickGetImageHeight(wand);

    MagickWand* old_wand = wand;
    wand = MagickCoalesceImages(old_wand);
    old_wand = DestroyMagickWand(old_wand);

    MagickResetIterator(wand);
    while (MagickNextImage(wand) != MagickFalse) {
        info.count++;
    }

    wand = DestroyMagickWand(wand);

    MagickWandTerminus();

    return info;
}

bool gif_load(const char* input_file,
              void** frames, size_t frames_count)
{
    if (string_ends_with(input_file, ".apng") || string_ends_with(input_file, ".png")) {
        input_file = string_append_prefix(input_file, "APNG:");
    }

    MagickWandGenesis();

    MagickWand* wand = NewMagickWand();
    MagickReadImage(wand, input_file);
    int frame_width = MagickGetImageWidth(wand);
    int frame_height = MagickGetImageHeight(wand);

    MagickWand* old_wand = wand;
    wand = MagickCoalesceImages(old_wand);
    old_wand = DestroyMagickWand(old_wand);

    MagickResetIterator(wand);
    size_t i = 0;
    while ((MagickNextImage(wand) != MagickFalse) && i < frames_count) {
        MagickBooleanType export_status = MagickExportImagePixels(wand,
                                                                  0, 0, frame_width, frame_height,
                                                                  "RGBA", CharPixel,
                                                                  frames[i]);

        if (export_status != MagickTrue) {
            magick_log_wand_exception(wand);
            DestroyMagickWand(wand);
            MagickWandTerminus();
            return false;
        }

        i++;
    }

    wand = DestroyMagickWand(wand);

    MagickWandTerminus();

    printf("Loaded GIF file `%s`\n", input_file);

    return true;
}
