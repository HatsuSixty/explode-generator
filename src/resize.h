#pragma once

#include <stdbool.h>

bool image_resize(void* inp_pixels, int old_width, int old_height,
                  void* out_pixels, int new_width, int new_height);
