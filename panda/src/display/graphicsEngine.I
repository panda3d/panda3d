// Filename: graphicsEngine.I
// Created by:  drose (24Feb02)
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
//     Function: GraphicsEngine::set_auto_flip
//       Access: Published
//  Description: Set this flag true to indicate the GraphicsEngine
//               should automatically cause windows to sync and flip
//               as soon as they have finished drawing, rather than
//               waiting for all of the windows to finish drawing
//               first so they can flip together.
//
//               This only affects the timing of when the flip occurs.
//               If this is true (the default), the flip occurs before
//               render_frame() returns.  If this is false, the flip
//               occurs whenever flip_frame() is called, or at the
//               beginning of the next call to render_frame(), if
//               flip_frame() is never called.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsEngine::
set_auto_flip(bool auto_flip) {
  // We don't bother with the mutex here.  It's just a bool, after
  // all.
  _auto_flip = auto_flip;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::get_auto_flip
//       Access: Published
//  Description: Returns the current setting for the auto-flip flag.
//               See set_auto_flip.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsEngine::
get_auto_flip() const {
  // We don't bother with the mutex here.  It's just a bool, after
  // all.
  return _auto_flip;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::make_gsg
//       Access: Published
//  Description: Creates a new gsg using the indicated GraphicsPipe
//               and returns it.  The GraphicsEngine does not
//               officially own the pointer to the gsg; but if any
//               windows are created using this GSG, the
//               GraphicsEngine will own the pointers to these
//               windows, which in turn will own the pointer to the
//               GSG.
//
//               There is no explicit way to release a GSG, but it
//               will be destructed when all windows that reference it
//               are destructed, and the draw thread that owns the GSG
//               runs one more time.
//
//               This flavor of make_gsg() uses the default
//               frame buffer properties, specified via
//               set_frame_buffer_properties().
////////////////////////////////////////////////////////////////////
INLINE PT(GraphicsStateGuardian) GraphicsEngine::
make_gsg(GraphicsPipe *pipe) {
  return make_gsg(pipe, get_frame_buffer_properties(), NULL);
}


////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::close_gsg
//       Access: Published
//  Description: Calls GraphicsPipe::close_gsg() on the indicated pipe
//               and GSG.  This function mainly exists to allow
//               GraphicsEngine::WindowRenderer to call the protected
//               method GraphicsPipe::close_gsg().
////////////////////////////////////////////////////////////////////
INLINE void GraphicsEngine::
close_gsg(GraphicsPipe *pipe, GraphicsStateGuardian *gsg) {
  pipe->close_gsg(gsg);
}


////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::Callback::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE GraphicsEngine::Callback::
Callback(CallbackFunction *func, void *data) :
  _func(func),
  _data(data)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::Callback::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsEngine::Callback::
operator < (const GraphicsEngine::Callback &other) const {
  if (_func != other._func) {
    return _func < other._func;
  }
  return _data < other._data;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::Callback::do_callback
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void GraphicsEngine::Callback::
do_callback() const {
  (*_func)(_data);
}
