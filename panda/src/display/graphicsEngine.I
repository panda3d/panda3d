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
//     Function: GraphicsEngine::get_threading_model
//       Access: Published
//  Description: Returns the current threading model in use; see
//               set_threading_model().
////////////////////////////////////////////////////////////////////
INLINE GraphicsEngine::ThreadingModel GraphicsEngine::
get_threading_model() const {
  return _threading_model;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::start_cull
//       Access: Private
//  Description: Starts the cull stage for the current frame.  This
//               may or may not execute within a sub-thread, according
//               to the current threading model.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsEngine::
start_cull() {
  if (_cull_sorting) {
    cull_bin_draw();
  } else {
    cull_and_draw_together();
  }
}
