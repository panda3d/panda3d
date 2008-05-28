// Filename: graphicsEngine.I
// Created by:  drose (24Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
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
//     Function: GraphicsEngine::make_buffer
//       Access: Published
//  Description: Syntactic shorthand for make_output
////////////////////////////////////////////////////////////////////
INLINE GraphicsOutput *GraphicsEngine::
make_buffer(GraphicsStateGuardian *gsg, const string &name,
            int sort, int x_size, int y_size) {
  FrameBufferProperties fb_props = FrameBufferProperties::get_default();
  fb_props.set_back_buffers(0);
  fb_props.set_stereo(0);
  fb_props.set_accum_bits(0);
  fb_props.set_multisamples(0);
  fb_props.set_force_hardware(0);
  fb_props.set_force_software(0);
  GraphicsOutput *result = make_output(gsg->get_pipe(), name, sort,
                                       fb_props, 
                                       WindowProperties::size(x_size, y_size),
                                       GraphicsPipe::BF_refuse_window |
                                       GraphicsPipe::BF_fb_props_optional,
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
  GraphicsOutput *result = make_output(host->get_pipe(), name, sort,
                                       FrameBufferProperties(),
                                       WindowProperties::size(x_size, y_size),
                                       GraphicsPipe::BF_require_parasite |
                                       GraphicsPipe::BF_fb_props_optional,
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

