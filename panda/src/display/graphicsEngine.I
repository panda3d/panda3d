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
//     Function: GraphicsEngine::set_portal_cull
//       Access: Published
//  Description: Set this flag true to indicate the GraphicsEngine
//               should start portal culling
////////////////////////////////////////////////////////////////////
INLINE void GraphicsEngine::
set_portal_cull(bool value) {
  // We don't bother with the mutex here.  It's just a bool, after
  // all.
  _portal_enabled = value;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::get_portal_cull
//       Access: Published
//  Description: Returns the current setting for the portal culling flag.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsEngine::
get_portal_cull() const {
  // We don't bother with the mutex here.  It's just a bool, after
  // all.
  return _portal_enabled;
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
//     Function: GraphicsEngine::make_window
//       Access: Published
//  Description: Syntactic shorthand for make_output
////////////////////////////////////////////////////////////////////
INLINE GraphicsWindow *GraphicsEngine::
make_window(GraphicsStateGuardian *gsg, const string &name, int sort) {
  // The hardwired size here is never used.
  GraphicsOutput *result = make_output(gsg->get_pipe(), name, sort,
                                       gsg->get_default_properties(), 50, 50,
                                       GraphicsPipe::BF_require_window,
                                       gsg, NULL);
  return DCAST(GraphicsWindow, result);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::make_buffer
//       Access: Published
//  Description: Syntactic shorthand for make_output
////////////////////////////////////////////////////////////////////
INLINE GraphicsOutput *GraphicsEngine::
make_buffer(GraphicsStateGuardian *gsg, const string &name,
            int sort, int x_size, int y_size) {
  FrameBufferProperties props = gsg->get_default_properties();
  int clear =
        FrameBufferProperties::FM_buffer |
        FrameBufferProperties::FM_stereo |
        FrameBufferProperties::FM_accum |
        FrameBufferProperties::FM_multisample |
        FrameBufferProperties::FM_hardware |
        FrameBufferProperties::FM_software;
  props.set_frame_buffer_mode(props.get_frame_buffer_mode() & (~clear));
  GraphicsOutput *result = make_output(gsg->get_pipe(), name, sort,
                                       gsg->get_default_properties(), x_size, y_size,
                                       GraphicsPipe::BF_refuse_window,
                                       gsg, NULL);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsEngine::make_parasite
//       Access: Published
//  Description: Syntactic shorthand for make_buffer.
////////////////////////////////////////////////////////////////////
INLINE GraphicsOutput *GraphicsEngine::
make_parasite(GraphicsOutput *host, const string &name, 
              int sort, int x_size, int y_size) {
  FrameBufferProperties props;
  GraphicsOutput *result = make_output(host->get_pipe(), name, sort,
                                       props, x_size, y_size,
                                       GraphicsPipe::BF_require_parasite,
                                       host->get_gsg(), host);
  return result;
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

