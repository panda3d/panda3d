// Filename: dynamicTextFont.I
// Created by:  drose (08Feb02)
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
//     Function: DynamicTextFont::set_margin
//       Access: Published
//  Description: Sets the number of pixels of padding that is added
//               around the border of each glyph before adding it to
//               the texture map.  This reduces the bleed in from
//               neighboring glyphs in the texture map.
////////////////////////////////////////////////////////////////////
INLINE void DynamicTextFont::
set_margin(int margin) {
  _margin = margin;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_margin
//       Access: Published
//  Description: Returns the number of pixels of padding that is added
//               around the border of each glyph.  See set_margin().
////////////////////////////////////////////////////////////////////
INLINE int DynamicTextFont::
get_margin() const {
  return _margin;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::set_page_size
//       Access: Published
//  Description: Sets the x, y size of the textures that are created
//               for the DynamicTextFont.
////////////////////////////////////////////////////////////////////
INLINE void DynamicTextFont::
set_page_size(int x_size, int y_size) {
  _page_x_size = x_size;
  _page_y_size = y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_page_x_size
//       Access: Published
//  Description: Returns the x size of the textures that are created
//               for the DynamicTextFont.  See set_page_size().
////////////////////////////////////////////////////////////////////
INLINE int DynamicTextFont::
get_page_x_size() const {
  return _page_x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextFont::get_page_y_size
//       Access: Published
//  Description: Returns the y size of the textures that are created
//               for the DynamicTextFont.  See set_page_size().
////////////////////////////////////////////////////////////////////
INLINE int DynamicTextFont::
get_page_y_size() const {
  return _page_y_size;
}
