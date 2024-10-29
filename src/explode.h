#pragma once

#include <raylib.h>

void image_to_explode_gif(Image image, const char* output, bool reverse);
void image_explode(Image* image, float level);
