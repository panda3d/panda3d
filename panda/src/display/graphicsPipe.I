// Filename: graphicsPipe.I
// Created by:  frang (07Mar99)
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
//     Function: GraphicsPipe::is_valid
//       Access: Published
//  Description: Returns false if this pipe is known to be invalid,
//               meaning that an attempt to create a GraphicsWindow
//               with the pipe will certainly fail.  Returns true if
//               the pipe is probably valid (is this case, an attempt
//               to create a GraphicsWindow should succeed, but might
//               still fail).
//
//               Use the GraphicsEngine class to create a
//               GraphicsWindow on a particular pipe.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsPipe::
is_valid() const {
  return _is_valid;
}
