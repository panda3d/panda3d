// Filename: fltVertex.I
// Created by:  drose (30Aug00)
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
//     Function: FltVertex::has_color
//       Access: Public
//  Description: Returns true if the vertex has a primary color
//               indicated, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FltVertex::
has_color() const {
  // Even if the no_color bit is not set, if the color_index is -1,
  // the vertex doesn't have a color (unless we've got packed color).
  return ((_flags & F_no_color) == 0 &&
          (_color_index != -1 || ((_flags & F_packed_color) != 0)));
}

////////////////////////////////////////////////////////////////////
//     Function: FltVertex::set_color
//       Access: Public
//  Description: Sets the color of the vertex, using the packed
//               color convention.  The alpha component is ignored.
////////////////////////////////////////////////////////////////////
INLINE void FltVertex::
set_color(const Colorf &color) {
  set_rgb(RGBColorf(color[0], color[1], color[2]));
}
