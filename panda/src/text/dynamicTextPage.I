// Filename: dynamicTextPage.I
// Created by:  drose (09Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: DynamicTextPage::get_x_size
//       Access: Published
//  Description: Returns the x size of the page (texture), in pixels.
////////////////////////////////////////////////////////////////////
INLINE int DynamicTextPage::
get_x_size() const {
  return _x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicTextPage::get_y_size
//       Access: Published
//  Description: Returns the y size of the page (texture), in pixels.
////////////////////////////////////////////////////////////////////
INLINE int DynamicTextPage::
get_y_size() const {
  return _y_size;
}


////////////////////////////////////////////////////////////////////
//     Function: DynamicTextPage::is_empty
//       Access: Published
//  Description: Returns true if the page has no glyphs, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool DynamicTextPage::
is_empty() const {
  return _glyphs.empty();
}
