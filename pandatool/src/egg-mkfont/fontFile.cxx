// Filename: fontFile.cxx
// Created by:  drose (18Feb01)
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

#include "fontFile.h"
#include "charBitmap.h"

#include <algorithm>


// An STL function object to sort the characters in order from tallest
// to shortest.  This provides a more optimal packing into the
// resulting image.
class SortCharBitmap {
public:
  bool operator() (const CharBitmap *c1, const CharBitmap *c2) const {
    return (c1->get_height() > c2->get_height());
  }
};


////////////////////////////////////////////////////////////////////
//     Function: FontFile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FontFile::
FontFile() {
  // A standard design size for font types that don't specify
  // otherwise.
  _ds = 12.0;
}

////////////////////////////////////////////////////////////////////
//     Function: FontFile::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
FontFile::
~FontFile() {
}

////////////////////////////////////////////////////////////////////
//     Function: FontFile::sort_chars_by_height
//       Access: Public
//  Description: Sorts the array of characters in the font as returned
//               by get_char() so that the tallest ones appear first
//               in the list.
////////////////////////////////////////////////////////////////////
void FontFile::
sort_chars_by_height() {
  sort(_chars.begin(), _chars.end(), SortCharBitmap());
}
