// Filename: graphicsWindow.I
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
//     Function: GraphicsWindow::is_closed
//       Access: Published
//  Description: Returns true if the window has not yet been opened,
//               or has been fully closed, false if it is open.  The
//               window is not opened immediately after
//               GraphicsEngine::make_window() is called; nor is it
//               closed immediately after
//               GraphicsEngine::remove_window() is called.  Either
//               operation may take a frame or two.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindow::
is_closed() const {
  return !_properties.get_open();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::is_active
//       Access: Published
//  Description: Returns true if the window is ready to be rendered
//               into, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindow::
is_active() const {
  // Make this smarter?
  return _properties.get_open() && !_properties.get_minimized();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::is_fullscreen
//       Access: Published
//  Description: Returns true if the window has been opened as a
//               fullscreen window, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindow::
is_fullscreen() const {
  return _properties.get_fullscreen();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_gsg
//       Access: Published
//  Description: Returns the GSG that is associated with this window.
//               There is a one-to-one association between windows and
//               GSG's.
//
//               This may return NULL if the graphics context has not
//               yet been created for the window, e.g. before the
//               first frame has rendered; or after the window has
//               been closed.
////////////////////////////////////////////////////////////////////
INLINE GraphicsStateGuardian *GraphicsWindow::
get_gsg() const {
  return _gsg;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_pipe
//       Access: Published
//  Description: Returns the GraphicsPipe that this window is
//               associated with.  It is possible that the
//               GraphicsPipe might have been deleted while an
//               outstanding PT(GraphicsWindow) prevented all of its
//               children windows from also being deleted; in this
//               unlikely case, get_pipe() may return NULL.
////////////////////////////////////////////////////////////////////
INLINE GraphicsPipe *GraphicsWindow::
get_pipe() const {
  return _pipe;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::win_display_regions_changed
//       Access: Public
//  Description: Intended to be called when the active state on a
//               nested channel or layer or display region changes,
//               forcing the window to recompute its list of active
//               display regions.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::
win_display_regions_changed() {
  _display_regions_stale = true;
}


////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::determine_display_regions
//       Access: Private
//  Description: Recomputes the list of active DisplayRegions within
//               the window, if they have changed recently.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::
determine_display_regions() const {
  // This function isn't strictly speaking const, but we pretend it is
  // because it only updates a transparent cache value.
  if (_display_regions_stale) {
    ((GraphicsWindow *)this)->do_determine_display_regions();
  }
}
