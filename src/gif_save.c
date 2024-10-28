#include "gif_save.h"

#include <stdbool.h>

#include "util/magick.h"
#include "util/string.h"

#include <MagickWand/MagickWand.h>

bool gif_save(GifFrames frames, const char* output_file)
{
    if (string_ends_with(output_file, ".apng") || string_ends_with(output_file, ".png")) {
        output_file = string_append_prefix(output_file, "APNG:");
    }

    MagickWandGenesis();

    MagickWand* wand = NewMagickWand();
    MagickSetSize(wand, frames.width, frames.height);

    for (size_t i = 0; i < frames.frames_count; ++i) {
        MagickWand* frame_wand = NewMagickWand();
        MagickSetSize(frame_wand, frames.width, frames.height);
        MagickReadImage(frame_wand, "xc:none");

        MagickBooleanType import_status = MagickImportImagePixels(frame_wand,
                                                                  0, 0, frames.width, frames.height,
                                                                  "RGBA", CharPixel,
                                                                  frames.frames[i]);

        if (import_status != MagickTrue) {
            magick_log_wand_exception(frame_wand);
            DestroyMagickWand(frame_wand);
            DestroyMagickWand(wand);
            MagickWandTerminus();
            return false;
        }

        // Not needed for .apng
        if (string_ends_with(output_file, ".gif")) {
            MagickSetImageDelay(frame_wand, 4);
        }

        MagickAddImage(wand, frame_wand);
        MagickSetLastIterator(wand);

        frame_wand = DestroyMagickWand(frame_wand);
    }

    MagickSetOption(wand, "loop", "0");

    if (MagickWriteImages(wand, output_file, MagickTrue) != MagickTrue) {
        magick_log_wand_exception(wand);
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return false;
    }

    DestroyMagickWand(wand);

    MagickWandTerminus();

    printf("Saved GIF file `%s`\n", output_file);

    return true;
}
