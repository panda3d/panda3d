// Filename: pnmTextGlyph.cxx
// Created by:  drose (03Apr02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "pnmTextGlyph.h"

////////////////////////////////////////////////////////////////////
//     Function: PNMTextGlyph::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMTextGlyph::
PNMTextGlyph(double advance) : 
  _advance(advance) 
{
  _left = 0;
  _top = 0;
  _int_advance = (int)floor(_advance + 0.5);
}

////////////////////////////////////////////////////////////////////
//     Function: PNMTextGlyph::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PNMTextGlyph::
~PNMTextGlyph() {
}

////////////////////////////////////////////////////////////////////
//     Function: PNMTextGlyph::rescale
//       Access: Public
//  Description: After the image has been rendered large by FreeType,
//               scales it small again for placing.
////////////////////////////////////////////////////////////////////
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

    // Pad the image by a few pixels all around to allow for
    // antialiasing at the edges.
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

    // And scale those back up so we can determine the amount of
    // additional padding we need to make the pixels remain in the
    // right place after the integer reduction.
    int old_x_size = (int)(new_x_size * scale_factor + 0.5);
    int old_y_size = (int)(new_y_size * scale_factor + 0.5);
    int old_left = (int)(new_left * scale_factor + 0.5);
    int old_top = (int)(new_top * scale_factor + 0.5);

    int pad_left = orig_left - old_left;
    int pad_top = old_top - orig_top;

    // These shouldn't go negative.
    nassertv(extra_pad + pad_left >= 0 && extra_pad + pad_top >= 0);

    PNMImage enlarged(old_x_size, old_y_size, 1);
    enlarged.copy_sub_image(_image, pad_left + extra_pad, pad_top + extra_pad);

    _image.clear(new_x_size, new_y_size, 1);
    _image.quick_filter_from(enlarged);

    _left = new_left;
    _top = new_top;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMTextGlyph::place
//       Access: Public
//  Description: Copies the glyph to the indicated destination image
//               at the indicated origin.  It colors the glyph pixels
//               the indicated foreground color, blends antialiased
//               pixels with the appropriate amount of the foreground
//               color and the existing background color, and leaves
//               other pixels alone.
////////////////////////////////////////////////////////////////////
void PNMTextGlyph::
place(PNMImage &dest_image, int xp, int yp, const Colorf &fg) {
  if (!_image.is_valid()) {
    // If we have no image, do nothing.
    return;
  }
  RGBColord fg_rgb(fg[0], fg[1], fg[2]);
  double fg_alpha = fg[3];

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
        dest_image.set_xel(x, y, fg_rgb);
        if (dest_image.has_alpha()) {
          dest_image.set_alpha(x, y, fg_alpha);
        }

      } else if (gval > 0.0) {
        RGBColord bg_rgb = dest_image.get_xel(x, y);
        dest_image.set_xel(x, y, fg_rgb * gval + bg_rgb * (1.0 - gval));
        if (dest_image.has_alpha()) {
          double bg_alpha = dest_image.get_alpha(x, y);
          dest_image.set_alpha(x, y, fg_alpha * gval + bg_alpha * (1.0 - gval));
        }
      }
    }
  }
}
