// Filename: graphicsOutput.I
// Created by:  drose (06Feb04)
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
//     Function: GraphicsOutput::get_gsg
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
INLINE GraphicsStateGuardian *GraphicsOutput::
get_gsg() const {
  return _gsg;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_pipe
//       Access: Published
//  Description: Returns the GraphicsPipe that this window is
//               associated with.  It is possible that the
//               GraphicsPipe might have been deleted while an
//               outstanding PT(GraphicsOutput) prevented all of its
//               children windows from also being deleted; in this
//               unlikely case, get_pipe() may return NULL.
////////////////////////////////////////////////////////////////////
INLINE GraphicsPipe *GraphicsOutput::
get_pipe() const {
  return _pipe;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_name
//       Access: Published
//  Description: Returns the name that was passed to the
//               GraphicsOutput constructor.
////////////////////////////////////////////////////////////////////
INLINE const string &GraphicsOutput::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::has_texture
//       Access: Published
//  Description: Returns true if the GraphicsOutput is set to render
//               into a texture, or false otherwise.
//
//               Normally, this will only be true for a GraphicsBuffer
//               object, and only when want_texture is passed in as
//               true to the GraphicsBuffer constructor.  If
//               show-buffers is true, this may also be set for a
//               GraphicsWindow, which is in this case serving in the
//               place of a GraphicsBuffer.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
has_texture() const {
  return !(_texture.is_null());
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_texture
//       Access: Published
//  Description: Returns the texture into which the GraphicsOutput
//               renders, if has_texture() is true, or NULL if
//               has_texture() is false.
//
//               If the texture is non-NULL, it may be applied to
//               geometry to be rendered for any other windows or
//               outputs that share the same GSG as this
//               GraphicsOutput.  The effect is undefined for windows
//               that share a different GSG; usually in these cases
//               the texture will be invalid.
////////////////////////////////////////////////////////////////////
INLINE Texture *GraphicsOutput::
get_texture() const {
  return _texture;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_x_size
//       Access: Published
//  Description: Returns the width of the graphics frame buffer, if it
//               is known.  In certain cases (e.g. fullscreen
//               windows), the size may not be known until after the
//               object has been fully created.  Check has_size()
//               first.
//
//               Certain objects (like windows) may change size
//               spontaneously; this method is not thread-safe.  To
//               get the size of a window in a thread-safe manner,
//               query get_properties().
////////////////////////////////////////////////////////////////////
INLINE int GraphicsOutput::
get_x_size() const {
  return _x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_y_size
//       Access: Published
//  Description: Returns the height of the graphics frame buffer, if it
//               is known.  In certain cases (e.g. fullscreen
//               windows), the size may not be known until after the
//               object has been fully created.  Check has_size()
//               first.
//
//               Certain objects (like windows) may change size
//               spontaneously; this method is not thread-safe.  To
//               get the size of a window in a thread-safe manner,
//               query get_properties().
////////////////////////////////////////////////////////////////////
INLINE int GraphicsOutput::
get_y_size() const {
  return _y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::has_size
//       Access: Published
//  Description: Returns true if the size of the window/frame buffer
//               is known, false otherwise.  In certain cases the size
//               may not be known until after the object has been
//               fully created.  Also, certain objects (like windows)
//               may change size spontaneously.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
has_size() const {
  return _has_size;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::is_valid
//       Access: Published
//  Description: Returns true if the output is fully created and ready
//               for rendering, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
is_valid() const {
  return _is_valid;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::get_sort
//       Access: Published
//  Description: Returns the sorting order of this particular
//               GraphicsOutput.  The various GraphicsOutputs within a
//               particular thread will be rendered in the indicated
//               order.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsOutput::
get_sort() const {
  return _sort;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::win_display_regions_changed
//       Access: Public
//  Description: Intended to be called when the active state on a
//               nested channel or layer or display region changes,
//               forcing the window to recompute its list of active
//               display regions.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
win_display_regions_changed() {
  _display_regions_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::operator <
//       Access: Public
//  Description: The sorting operator is used to order the
//               GraphicsOutput object in order by their sort number,
//               so that they will render in the correct order in the
//               GraphicsEngine.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsOutput::
operator < (const GraphicsOutput &other) const {
  return _sort < other._sort;
}


////////////////////////////////////////////////////////////////////
//     Function: GraphicsOutput::determine_display_regions
//       Access: Private
//  Description: Recomputes the list of active DisplayRegions within
//               the window, if they have changed recently.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsOutput::
determine_display_regions() const {
  // This function isn't strictly speaking const, but we pretend it is
  // because it only updates a transparent cache value.
  if (_display_regions_stale) {
    ((GraphicsOutput *)this)->do_determine_display_regions();
  }
}
