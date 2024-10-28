#include "resize.h"

#include <stdbool.h>

#include <MagickWand/MagickWand.h>

#define log_wand_exception(wand)                                                       \
    {                                                                                  \
        ExceptionType severity;                                                        \
        char* description = MagickGetException(wand, &severity);                       \
                                                                                       \
        fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, __func__, description); \
                                                                                       \
        description = (char*)MagickRelinquishMemory(description);                      \
    }

bool image_resize(void* inp_pixels, int old_width, int old_height,
                  void* out_pixels, int new_width, int new_height)
{
    MagickWandGenesis();

    MagickWand* wand = NewMagickWand();
    MagickSetSize(wand, old_width, old_height);
    MagickReadImage(wand, "xc:none");

    MagickBooleanType import_status = MagickImportImagePixels(wand,
                                                              0, 0, old_width, old_height,
                                                              "RGBA", CharPixel,
                                                              inp_pixels);

    if (import_status != MagickTrue) {
        log_wand_exception(wand);
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return false;
    }

    MagickResizeImage(wand, new_width, new_height, CubicFilter);

    MagickExportImagePixels(wand,
                            0, 0, new_width, new_height,
                            "RGBA", CharPixel, out_pixels),

    DestroyMagickWand(wand);

    MagickWandTerminus();

    return true;
}
