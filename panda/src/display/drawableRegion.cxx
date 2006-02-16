// Filename: drawableRegion.cxx
// Created by:  drose (11Jul02)
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

#include "drawableRegion.h"


////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DrawableRegion::
~DrawableRegion() {
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::get_screenshot_buffer_type
//       Access: Public, Virtual
//  Description: Returns the RenderBuffer that should be used for
//               capturing screenshots from this particular
//               DrawableRegion.
////////////////////////////////////////////////////////////////////
int DrawableRegion::
get_screenshot_buffer_type() const {
  return _screenshot_buffer_type;
}

////////////////////////////////////////////////////////////////////
//     Function: DrawableRegion::get_draw_buffer_type
//       Access: Public, Virtual
//  Description: Returns the RenderBuffer into which the GSG should
//               issue draw commands.  Normally, this is the back
//               buffer for double-buffered windows, and the front
//               buffer for single-buffered windows.
////////////////////////////////////////////////////////////////////
int DrawableRegion::
get_draw_buffer_type() const {
  return _draw_buffer_type;
}
