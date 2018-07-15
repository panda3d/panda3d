/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pnmTextGlyph.cxx
 * @author drose
 * @date 2002-04-03
 */

#include "pnmTextGlyph.h"
#include "indent.h"

using std::max;
using std::min;

/**
 *
 */
PNMTextGlyph::
PNMTextGlyph(double advance) :
  _advance(advance)
{
  _left = 0;
  _top = 0;
  _int_advance = (int)floor(_advance + 0.5);
}

/**
 *
 */
PNMTextGlyph::
~PNMTextGlyph() {
}

/**
 * Copies the glyph to the indicated destination image at the indicated
 * origin.  It colors the glyph pixels the indicated foreground color, blends
 * antialiased pixels with the appropriate amount of the foreground color and
 * the existing background color, and leaves other pixels alone.
 */
void PNMTextGlyph::
place(PNMImage &dest_image, int xp, int yp, const LColor &fg) {
  if (!_image.is_valid()) {
    // If we have no image, do nothing.
    return;
  }

  LColorf fgf = LCAST(float, fg);

  int left = xp + _left;
  int top = yp - _top;
  int right = left + _image.get_x_size();
  int bottom = top + _image.get_y_size();

  // Clip the glyph to the destination image.
  int cleft = max(left, 0);
  int ctop = max(top, 0);
  int cright = min(right, dest_image.get_x_size());
  int cbottom = min(bottom, dest_image.get_y_size());

  for (int y = ctop; y < cbottom; y++) {
    for (int x = cleft; x < cright; x++) {
      double gval = get_value(x - left, y - top);
      if (gval == 1.0) {
        dest_image.set_xel_a(x, y, fgf);

      } else if (gval > 0.0) {
        LColorf bg = dest_image.get_xel_a(x, y);
        dest_image.set_xel_a(x, y, fgf * gval + bg * (1.0 - gval));
      }
    }
  }
}

/**
 * This flavor of place() also fills in the interior color.  This requires
 * that determine_interior was called earlier.
 */
void PNMTextGlyph::
place(PNMImage &dest_image, int xp, int yp, const LColor &fg,
      const LColor &interior) {
  if (!_image.is_valid()) {
    // If we have no image, do nothing.
    return;
  }

  LColorf fgf = LCAST(float, fg);
  LColorf interiorf = LCAST(float, interior);

  int left = xp + _left;
  int top = yp - _top;
  int right = left + _image.get_x_size();
  int bottom = top + _image.get_y_size();

  // Clip the glyph to the destination image.
  int cleft = max(left, 0);
  int ctop = max(top, 0);
  int cright = min(right, dest_image.get_x_size());
  int cbottom = min(bottom, dest_image.get_y_size());

  for (int y = ctop; y < cbottom; y++) {
    for (int x = cleft; x < cright; x++) {
      double gval = get_value(x - left, y - top);
      if (gval == 1.0) {
        dest_image.set_xel_a(x, y, fgf);

      } else if (gval > 0.0) {
        bool is_interior = get_interior_flag(x - left, y - top);
        LColorf bg;
        if (is_interior) {
          bg = interiorf;
        } else {
          bg = dest_image.get_xel_a(x, y);
        }

        dest_image.set_xel_a(x, y, fgf * gval + bg * (1.0 - gval));
      } else { // gval == 0.0
        bool is_interior = get_interior_flag(x - left, y - top);
        if (is_interior) {
          dest_image.set_xel_a(x, y, interiorf);
        }
      }
    }
  }
}

/**
 * Once the glyph has been generated, but before it has been scaled down by
 * _scale_factor, walk through the glyph and try to determine which parts
 * represent the interior portions of a hollow font, and mark them so they may
 * be properly colored.
 */
void PNMTextGlyph::
determine_interior() {
  // We will use the red component as a working buffer.  First, we fill the
  // whole thing to maxval.
  int x_size = _image.get_x_size();
  int y_size = _image.get_y_size();
  xelval maxval = _image.get_maxval();
  for (int yi = 0; yi < y_size; yi++) {
    for (int xi = 0; xi < x_size; xi++) {
      _image.set_red_val(xi, yi, maxval);
    }
  }

  // Now we recursively analyze the image to determine the number of walls
  // between each pixel and any edge.  All outer edge pixels have a value of
  // 0; all dark pixels adjacent to those pixels have a value of 1, and light
  // pixels adjacent to those have a value of 2, and so on.
  _scan_interior_points.clear();
  for (int yi = 0; yi < y_size; yi++) {
    scan_interior(0, yi, 0, false, 0);
    scan_interior(x_size - 1, yi, 0, false, 0);
  }
  for (int xi = 0; xi < x_size; xi++) {
    scan_interior(xi, 0, 0, false, 0);
    scan_interior(xi, y_size - 1, 0, false, 0);
  }

  // Pick up any points that we couldn't visit recursively because of the lame
  // stack limit on Windows.
  while (!_scan_interior_points.empty()) {
    int index = _scan_interior_points.back();
    _scan_interior_points.pop_back();
    int y = index / _image.get_x_size();
    int x = index % _image.get_x_size();
    xelval new_code = _image.get_red_val(x, y);
    bool this_dark = (_image.get_blue_val(x, y) > 0);

    scan_interior(x - 1, y, new_code, this_dark, 0);
    scan_interior(x, y - 1, new_code, this_dark, 0);
    scan_interior(x + 1, y, new_code, this_dark, 0);
    scan_interior(x, y + 1, new_code, this_dark, 0);
  }
  _scan_interior_points.clear();

  // Finally, go back and set any pixel whose red value is two more than a
  // multiple of 4 to dark.  This indicates the interior part of a hollow
  // font.
  for (int yi = 0; yi < y_size; yi++) {
    for (int xi = 0; xi < x_size; xi++) {
      xelval code = _image.get_red_val(xi, yi);
      if (((code + 2) & 0x3) == 0) {
        _image.set_red_val(xi, yi, maxval);
      } else {
        _image.set_red_val(xi, yi, 0);
      }
    }
  }
}

/**
 * Recursively scans the image for interior pixels.  On completion, the
 * image's red channel will be filled with 0, 1, 2, etc., representing the
 * number of edges between each pixel and the border.
 */
void PNMTextGlyph::
scan_interior(int x, int y, xelval new_code, bool neighbor_dark,
              int recurse_level) {
  if (x < 0 || y < 0 || x >= _image.get_x_size() || y >= _image.get_y_size()) {
    return;
  }
  bool this_dark = (_image.get_blue_val(x, y) > 0);
  if (this_dark != neighbor_dark) {
    // If we just crossed an edge, we have to increment the code.
    if (new_code < _image.get_maxval()) {
      new_code++;
    }
    nassertv(new_code > 0);
  }

  if (new_code < _image.get_red_val(x, y)) {
    _image.set_red_val(x, y, new_code);
    recurse_level++;
    if (recurse_level > 1024) {
      // To cobble around a lame Windows limitation on the length of the
      // stack, we must prevent the recursion from going too deep.  But we
      // still need to remember this pixel so we can come back to it later.
      int index = y * _image.get_x_size() + x;
      _scan_interior_points.push_back(index);

    } else {
      scan_interior(x - 1, y, new_code, this_dark, recurse_level);
      scan_interior(x, y - 1, new_code, this_dark, recurse_level);
      scan_interior(x + 1, y, new_code, this_dark, recurse_level);
      scan_interior(x, y + 1, new_code, this_dark, recurse_level);
    }
  }
}

/**
 * After the image has been rendered large by FreeType, scales it small again
 * for placing.
 */
void PNMTextGlyph::
rescale(double scale_factor) {
  if (scale_factor == 1.0) {
    return;
  }
  nassertv(scale_factor != 0.0);
  _advance /= scale_factor;
  _int_advance = (int)floor(_advance + 0.5);

  if (_image.is_valid()) {
    int orig_x_size = _image.get_x_size();
    int orig_y_size = _image.get_y_size();
    int orig_left = _left;
    int orig_top = _top;

    // Pad the image by a few pixels all around to allow for antialiasing at
    // the edges.
    int extra_pad = (int)ceil(scale_factor);
    orig_x_size += 2*extra_pad;
    orig_y_size += 2*extra_pad;
    orig_left -= extra_pad;
    orig_top += extra_pad;

    // Now compute the reduced size.
    int new_x_size = (int)ceil(orig_x_size / scale_factor);
    int new_y_size = (int)ceil(orig_y_size / scale_factor);
    int new_left = (int)floor(orig_left / scale_factor);
    int new_top = (int)ceil(orig_top / scale_factor);

    // And scale those back up so we can determine the amount of additional
    // padding we need to make the pixels remain in the right place after the
    // integer reduction.
    int old_x_size = (int)(new_x_size * scale_factor + 0.5);
    int old_y_size = (int)(new_y_size * scale_factor + 0.5);
    int old_left = (int)(new_left * scale_factor + 0.5);
    int old_top = (int)(new_top * scale_factor + 0.5);

    int pad_left = orig_left - old_left;
    int pad_top = old_top - orig_top;

    // These shouldn't go negative.
    nassertv(extra_pad + pad_left >= 0 && extra_pad + pad_top >= 0);

    PNMImage enlarged(old_x_size, old_y_size, _image.get_num_channels());
    enlarged.copy_sub_image(_image, pad_left + extra_pad, pad_top + extra_pad);

    _image.clear(new_x_size, new_y_size, _image.get_num_channels());
    _image.quick_filter_from(enlarged);

    _left = new_left;
    _top = new_top;
  }
}
