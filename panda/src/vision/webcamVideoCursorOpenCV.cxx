/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file webcamVideoCursorOpenCV.cxx
 * @author drose
 * @date 2010-10-20
 */

#include "webcamVideoCursorOpenCV.h"

#ifdef HAVE_OPENCV

#include "webcamVideoOpenCV.h"
#include "movieVideoCursor.h"

#include "pStatTimer.h"

#include <opencv2/highgui/highgui.hpp>

TypeHandle WebcamVideoCursorOpenCV::_type_handle;

/**
 *
 */
WebcamVideoCursorOpenCV::
WebcamVideoCursorOpenCV(WebcamVideoOpenCV *src) : MovieVideoCursor(src) {
  _size_x = src->_size_x;
  _size_y = src->_size_y;
  _num_components = 3;
  _length = 1.0E10;
  _can_seek = false;
  _can_seek_fast = false;
  _aborted = false;
  _streaming = true;
  _ready = false;

  _capture = cvCaptureFromCAM(src->_camera_index);
  if (_capture != nullptr) {
    _size_x = (int)cvGetCaptureProperty(_capture, CV_CAP_PROP_FRAME_WIDTH);
    _size_y = (int)cvGetCaptureProperty(_capture, CV_CAP_PROP_FRAME_HEIGHT);
    _ready = true;
  }
}

/**
 *
 */
WebcamVideoCursorOpenCV::
~WebcamVideoCursorOpenCV() {
  if (_capture != nullptr) {
    cvReleaseCapture(&_capture);
    _capture = nullptr;
  }
}

/**
 *
 */
PT(MovieVideoCursor::Buffer) WebcamVideoCursorOpenCV::
fetch_buffer() {
  if (!_ready) {
    return nullptr;
  }

  PT(Buffer) buffer = get_standard_buffer();
  unsigned char *dest = buffer->_block;
  int num_components = get_num_components();
  nassertr(num_components == 3, nullptr);
  int dest_x_pitch = num_components;  // Assume component_width == 1
  int dest_y_pitch = _size_x * dest_x_pitch;

  const unsigned char *r, *g, *b;
  int x_pitch, y_pitch;
  if (get_frame_data(r, g, b, x_pitch, y_pitch)) {
    if (num_components == 3 && x_pitch == 3) {
      // The easy case--copy the whole thing in, row by row.
      int copy_bytes = _size_x * dest_x_pitch;
      nassertr(copy_bytes <= dest_y_pitch && copy_bytes <= abs(y_pitch), nullptr);

      for (int y = 0; y < _size_y; ++y) {
        memcpy(dest, r, copy_bytes);
        dest += dest_y_pitch;
        r += y_pitch;
      }

    } else {
      // The harder case--interleave in the color channels, pixel by pixel.

      for (int y = 0; y < _size_y; ++y) {
        int dx = 0;
        int sx = 0;
        for (int x = 0; x < _size_x; ++x) {
          dest[dx] = r[sx];
          dest[dx + 1] = g[sx];
          dest[dx + 2] = b[sx];
          dx += dest_x_pitch;
          sx += x_pitch;
        }
        dest += dest_y_pitch;
        r += y_pitch;
        g += y_pitch;
        b += y_pitch;
      }
    }
  }

  return buffer;
}

/**
 * Gets the data needed to traverse through the decompressed buffer.  Returns
 * true on success, false on failure.
 *
 * In the case of a success indication (true return value), the three pointers
 * r, g, b are loaded with the addresses of the three components of the
 * bottom-left pixel of the image.  (They will be adjacent in memory in the
 * case of an interleaved image, and separated in the case of a separate-
 * channel image.)  The x_pitch value is filled with the amount to add to each
 * pointer to advance to the pixel to the right; and the y_pitch value is
 * filled with the amount to add to each pointer to advance to the pixel
 * above.  Note that these values may be negative (particularly in the case of
 * a top-down image).
 */
bool WebcamVideoCursorOpenCV::
get_frame_data(const unsigned char *&r,
               const unsigned char *&g,
               const unsigned char *&b,
               int &x_pitch, int &y_pitch) {
  nassertr(ready(), false);

  IplImage *image = cvQueryFrame(_capture);
  if (image == nullptr) {
    return false;
  }

  r = (const unsigned char *)image->imageData;
  g = r + 1;
  b = g + 1;
  x_pitch = 3;
  y_pitch = image->widthStep;

  if (image->dataOrder == 1) {
    // Separate channel images.  That means a block of r, followed by a block
    // of g, followed by a block of b.
    x_pitch = 1;
    g = r + image->height * y_pitch;
    b = g + image->height * y_pitch;
  }

  if (image->origin == 0) {
    // The image data starts with the top row and ends with the bottom row--
    // the opposite of Texture::_ram_data's storage convention.  Therefore, we
    // must increment the initial pointers to the last row, and count
    // backwards.
    r += (image->height - 1) * y_pitch;
    g += (image->height - 1) * y_pitch;
    b += (image->height - 1) * y_pitch;
    y_pitch = -y_pitch;
  }

  return true;
}

#endif
