/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmReaderEmscripten.cxx
 * @author rdb
 * @date 2021-02-05
 */

#include "pnmReaderEmscripten.h"

#ifdef __EMSCRIPTEN__

#include "config_pnmimage.h"

#include <emscripten.h>

/**
 *
 */
PNMReaderEmscripten::
PNMReaderEmscripten(PNMFileType *type, Filename fullpath, int width, int height) :
  PNMReader(type, nullptr, false),
  _fullpath(std::move(fullpath))
{
  _x_size = width;
  _y_size = height;
  _num_channels = 4;
  _maxval = 255;
}

/**
 * Reads in an entire image all at once, storing it in the pre-allocated
 * _x_size * _y_size array and alpha pointers.  (If the image type has no
 * alpha channel, alpha is ignored.)  Returns the number of rows correctly
 * read.
 *
 * Derived classes need not override this if they instead provide
 * supports_read_row() and read_row(), below.
 */
int PNMReaderEmscripten::
read_data(xel *array, xelval *alpha) {
  if (!is_valid()) {
    return 0;
  }

  int width, height;
  unsigned char *data = (unsigned char *)emscripten_get_preloaded_image_data(_fullpath.c_str(), &width, &height);
  nassertr(data != nullptr, 0);
  nassertr(width == _x_size, 0);
  if (height > _x_size) {
    height = _x_size;
  }

  size_t total_size = (size_t)width * (size_t)height;
  for (int i = 0; i < total_size; ++i) {
    array[i].r = data[i * 4 + 0];
    array[i].g = data[i * 4 + 1];
    array[i].b = data[i * 4 + 2];
    alpha[i] = data[i * 4 + 3];
  }

  free(data);

  return height;
}


#endif  // __EMSCRIPTEN__
