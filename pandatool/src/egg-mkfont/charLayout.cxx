// Filename: charLayout.cxx
// Created by:  drose (16Feb01)
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

#include "charLayout.h"
#include "charBitmap.h"

#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: CharLayout::reset
//       Access: Public
//  Description: Removes all the characters already placed on the
//               layout, and resets the parameters for a new attempt.
////////////////////////////////////////////////////////////////////
void CharLayout::
reset(int working_xsize, int working_ysize, int working_buffer_pixels) {
  _working_xsize = working_xsize;
  _working_ysize = working_ysize;
  _working_buffer_pixels = working_buffer_pixels;

  _placements.clear();

  _cx = _working_buffer_pixels;
  _cy = _working_buffer_pixels;
  _nexty = _cy;
}

////////////////////////////////////////////////////////////////////
//     Function: CharLayout::place_character
//       Access: Public
//  Description: Given a character bitmap and font metrics extracted
//               from the pk file, find a place for it on the layout.
//               Returns true if the character was placed, false if we
//               ran out of room.
////////////////////////////////////////////////////////////////////
bool CharLayout::
place_character(const CharBitmap *bm) {
  int width = bm->get_width() + _working_buffer_pixels;
  int height = bm->get_height() + _working_buffer_pixels;

  int x, y;
  if (find_hole(x, y, width, height)) {
    _placements.push_back(CharPlacement(bm, x, y, width, height));
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CharLayout::find_hole
//       Access: Private
//  Description: Searches for a hole of at least x_size by y_size
//               pixels somewhere within the layout.  If a
//               suitable hole is found, sets x and y to the top left
//               corner and returns true; otherwise, returns false.
////////////////////////////////////////////////////////////////////
bool CharLayout::
find_hole(int &x, int &y, int x_size, int y_size) const {
  y = _working_buffer_pixels;
  while (y + y_size <= _working_ysize) {
    int next_y = _working_ysize;
    // Scan along the row at 'y'.
    x = _working_buffer_pixels;
    while (x + x_size <= _working_xsize) {
      int next_x = x;

      // Consider the spot at x, y.
      const CharPlacement *overlap = find_overlap(x, y, x_size, y_size);

      if (overlap == (const CharPlacement *)NULL) {
        // Hooray!
        return true;
      }

      next_x = overlap->_x + overlap->_width;
      next_y = min(next_y, overlap->_y + overlap->_height);
      nassertr(next_x > x, false);
      x = next_x;
    }

    nassertr(next_y > y, false);
    y = next_y;
  }

  // Nope, wouldn't fit anywhere.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: CharLayout::find_overlap
//       Access: Private
//  Description: If the rectangle whose top left corner is x, y and
//               whose size is x_size, y_size describes an empty hole
//               that does not overlap any placed chars, returns
//               NULL; otherwise, returns the first placed texture
//               that the image does overlap.  It is assumed the
//               rectangle lies completely within the boundaries of
//               the image itself.
////////////////////////////////////////////////////////////////////
const CharPlacement *CharLayout::
find_overlap(int x, int y, int x_size, int y_size) const {
  Placements::const_iterator pi;
  for (pi = _placements.begin(); pi != _placements.end(); ++pi) {
    const CharPlacement &placement = (*pi);
    if (placement.intersects(x, y, x_size, y_size)) {
      return &placement;
    }
  }

  return (const CharPlacement *)NULL;
}
