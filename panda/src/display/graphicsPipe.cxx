// Filename: graphicsPipe.cxx
// Created by:  mike (09Jan97)
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

#include "graphicsPipe.h"
#include "graphicsWindow.h"
#include "graphicsBuffer.h"
#include "config_display.h"
#include "mutexHolder.h"

TypeHandle GraphicsPipe::_type_handle;

// This array keeps the bitmasks of options that we pull out of the
// requested frame_buffer_mode, in order.  This is used by derived
// classes to find a suitable pixel format (or fbconfig, or whatever
// the backend calls it) that comes as close as possible to the
// requested configuration.

const int GraphicsPipe::strip_properties[] = {
  // One esoteric option removed.
  FrameBufferProperties::FM_multisample,
  FrameBufferProperties::FM_stencil,
  FrameBufferProperties::FM_accum,
  FrameBufferProperties::FM_alpha,
  FrameBufferProperties::FM_stereo,
  
  // Two esoteric options removed.
  FrameBufferProperties::FM_stencil | FrameBufferProperties::FM_multisample,
  FrameBufferProperties::FM_accum | FrameBufferProperties::FM_multisample,
  FrameBufferProperties::FM_alpha | FrameBufferProperties::FM_multisample,
  FrameBufferProperties::FM_stereo | FrameBufferProperties::FM_multisample,
  FrameBufferProperties::FM_stencil | FrameBufferProperties::FM_accum,
  FrameBufferProperties::FM_alpha | FrameBufferProperties::FM_stereo,
  FrameBufferProperties::FM_stencil | FrameBufferProperties::FM_accum | FrameBufferProperties::FM_multisample,
  FrameBufferProperties::FM_alpha | FrameBufferProperties::FM_stereo | FrameBufferProperties::FM_multisample,
  
  // All esoteric options removed.
  FrameBufferProperties::FM_stencil | FrameBufferProperties::FM_accum | FrameBufferProperties::FM_alpha | FrameBufferProperties::FM_stereo | FrameBufferProperties::FM_multisample,
  
  // All esoteric options, plus some we'd really really prefer,
  // removed.
  FrameBufferProperties::FM_stencil | FrameBufferProperties::FM_accum | FrameBufferProperties::FM_alpha | FrameBufferProperties::FM_stereo | FrameBufferProperties::FM_multisample | FrameBufferProperties::FM_double_buffer,
  
  // A zero marks the end of the array.
  0
};


////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsPipe::
GraphicsPipe() {
  // Initially, we assume the GraphicsPipe is valid.  A derived class
  // should set this to false if it determines otherwise.
  _is_valid = true;

  // A derived class must indicate the kinds of GraphicsOutput objects
  // it can create.
  _supported_types = 0;

  _display_width = 0;
  _display_height = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Copy Constructor
//       Access: Private
//  Description: Don't try to copy GraphicsPipes.
////////////////////////////////////////////////////////////////////
GraphicsPipe::
GraphicsPipe(const GraphicsPipe &) {
  _is_valid = false;
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Copy Assignment Operator
//       Access: Private
//  Description: Don't try to copy GraphicsPipes.
////////////////////////////////////////////////////////////////////
void GraphicsPipe::
operator = (const GraphicsPipe &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
GraphicsPipe::
~GraphicsPipe() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_num_hw_channels
//       Access: Public, Virtual
//  Description: Returns the number of hardware channels available for
//               pipes of this type.  See get_hw_channel().
////////////////////////////////////////////////////////////////////
int GraphicsPipe::
get_num_hw_channels() {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_hw_channel
//       Access: Public, Virtual
//  Description: Creates and returns an accessor to the
//               HardwareChannel at the given index number, which must
//               be in the range 0 <= index < get_num_hw_channels().
//               This function will return NULL if the index number is
//               out of range or the hardware channel at that index is
//               unavailable.
//
//               Most kinds of GraphicsPipes do not have any special
//               hardware channels available, and this function will
//               always return NULL.
////////////////////////////////////////////////////////////////////
HardwareChannel *GraphicsPipe::
get_hw_channel(GraphicsOutput *, int) {
  return (HardwareChannel*)0L;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::make_device
//       Access: Public, Virtual
//  Description: Creates a new device for the pipe. Only DirectX uses
//               this device, for other api's it is NULL.
////////////////////////////////////////////////////////////////////
PT(GraphicsDevice) GraphicsPipe::
make_device(void *scrn) {
  display_cat.error() 
    << "make_device() unimplemented by " << get_type() << "\n";
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::make_gsg
//       Access: Protected, Virtual
//  Description: Creates a new GSG to use the pipe (but no windows
//               have been created yet for the GSG).  This method will
//               be called in the draw thread for the GSG.
////////////////////////////////////////////////////////////////////
PT(GraphicsStateGuardian) GraphicsPipe::
make_gsg(const FrameBufferProperties &properties, 
         GraphicsStateGuardian *share_with) {
  display_cat.error()
    << "make_gsg() unimplemented by " << get_type() << "\n";
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::close_gsg
//       Access: Protected, Virtual
//  Description: This will be called in the draw thread (the same
//               thread in which the GSG was created via make_gsg,
//               above) to close the indicated GSG and free its
//               associated graphics objects just before it is
//               destructed.  This method exists to provide a hook for
//               the graphics pipe to do any necessary cleanup, if
//               any.
////////////////////////////////////////////////////////////////////
void GraphicsPipe::
close_gsg(GraphicsStateGuardian *gsg) {
  if (gsg != (GraphicsStateGuardian *)NULL) {
    gsg->close_gsg();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::make_window
//       Access: Protected, Virtual
//  Description: Creates a new window on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsWindow) GraphicsPipe::
make_window(GraphicsStateGuardian *, const string &) {
  display_cat.error()
    << get_type() << " cannot create onscreen windows.\n";
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::make_buffer
//       Access: Protected, Virtual
//  Description: Creates a new offscreen buffer on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsBuffer) GraphicsPipe::
make_buffer(GraphicsStateGuardian *, const string &, int, int, bool) {
  display_cat.error()
    << get_type() << " cannot create offscreen buffers.\n";
  return NULL;
}
