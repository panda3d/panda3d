// Filename: graphicsEngine.I
// Created by:  drose (24Feb02)
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
//     Function: GraphicsEngine::make_window
//       Access: Published
//  Description: Creates a new window using the indicated GraphicsPipe
//               and returns it.  The GraphicsEngine becomes the owner
//               of the window; it will persist at least until
//               remove_window() is called later.
//
//               This flavor of make_window() uses the default
//               threading model, specified via set_threading_model().
////////////////////////////////////////////////////////////////////
INLINE GraphicsWindow *GraphicsEngine::
make_window(GraphicsPipe *pipe) {
  return make_window(pipe, get_threading_model());
}
