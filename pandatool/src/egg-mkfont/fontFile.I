// Filename: fontFile.I
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


////////////////////////////////////////////////////////////////////
//     Function: FontFile::get_ds
//       Access: Public
//  Description: Returns the designed size of the font in points.
//               This is the basic height of a line of text in the
//               given font.
////////////////////////////////////////////////////////////////////
INLINE double FontFile::
get_ds() const {
  return _ds;
}

////////////////////////////////////////////////////////////////////
//     Function: FontFile::get_dpi
//       Access: Public
//  Description: Returns the resolution of the rasterized font in dots
//               per inch.  This is mainly useful for user
//               information; use get_vppp() and get_hppp() for more
//               precises measurements.
////////////////////////////////////////////////////////////////////
INLINE double FontFile::
get_dpi() const {
  return floor(_vppp * 72.27 + 0.5);
}

////////////////////////////////////////////////////////////////////
//     Function: FontFile::get_vppp
//       Access: Public
//  Description: Returns the vertical pixels per point of the
//               rasterized font characters.  This is the number of
//               pixels high each bitmap characters is per point of
//               font.
////////////////////////////////////////////////////////////////////
INLINE double FontFile::
get_vppp() const {
  return _vppp;
}

////////////////////////////////////////////////////////////////////
//     Function: FontFile::get_hppp
//       Access: Public
//  Description: Returns the horizontal pixels per point of the
//               rasterized font characters.  This is the number of
//               pixels wide each bitmap characters is per point of
//               font.
////////////////////////////////////////////////////////////////////
INLINE double FontFile::
get_hppp() const {
  return _hppp;
}

////////////////////////////////////////////////////////////////////
//     Function: FontFile::get_num_chars
//       Access: Public
//  Description: Returns the number of characters (glyphs) read from
//               the font.
////////////////////////////////////////////////////////////////////
INLINE int FontFile::
get_num_chars() const {
  return _chars.size();
}

////////////////////////////////////////////////////////////////////
//     Function: FontFile::get_char
//       Access: Public
//  Description: Returns the nth character (glyph) read from the font.
////////////////////////////////////////////////////////////////////
INLINE CharBitmap *FontFile::
get_char(int n) const {
  nassertr(n >= 0 && n < (int)_chars.size(), (CharBitmap *)NULL);
  return _chars[n];
}
