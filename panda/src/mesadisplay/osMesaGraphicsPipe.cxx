// Filename: osMesaGraphicsPipe.cxx
// Created by:  drose (09Feb04)
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

#include "osMesaGraphicsPipe.h"
#include "osMesaGraphicsBuffer.h"
#include "osMesaGraphicsStateGuardian.h"
#include "config_mesadisplay.h"
#include "frameBufferProperties.h"
#include "mutexHolder.h"

TypeHandle OsMesaGraphicsPipe::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
OsMesaGraphicsPipe::
OsMesaGraphicsPipe() {
  _supported_types = OT_buffer | OT_texture_buffer;
  _is_valid = true;
}

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsPipe::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
OsMesaGraphicsPipe::
~OsMesaGraphicsPipe() {
}

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsPipe::get_interface_name
//       Access: Published, Virtual
//  Description: Returns the name of the rendering interface
//               associated with this GraphicsPipe.  This is used to
//               present to the user to allow him/her to choose
//               between several possible GraphicsPipes available on a
//               particular platform, so the name should be meaningful
//               and unique for a given platform.
////////////////////////////////////////////////////////////////////
string OsMesaGraphicsPipe::
get_interface_name() const {
  return "Offscreen Mesa";
}

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsPipe::pipe_constructor
//       Access: Public, Static
//  Description: This function is passed to the GraphicsPipeSelection
//               object to allow the user to make a default
//               OsMesaGraphicsPipe.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) OsMesaGraphicsPipe::
pipe_constructor() {
  return new OsMesaGraphicsPipe;
}

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsPipe::make_gsg
//       Access: Protected, Virtual
//  Description: Creates a new GSG to use the pipe (but no windows
//               have been created yet for the GSG).  This method will
//               be called in the draw thread for the GSG.
////////////////////////////////////////////////////////////////////
PT(GraphicsStateGuardian) OsMesaGraphicsPipe::
make_gsg(const FrameBufferProperties &properties) {
  return new OSMesaGraphicsStateGuardian(properties);
}

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsPipe::make_buffer
//       Access: Protected, Virtual
//  Description: Creates a new offscreen buffer on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsBuffer) OsMesaGraphicsPipe::
make_buffer(GraphicsStateGuardian *gsg, int x_size, int y_size, bool want_texture) {
  return new OsMesaGraphicsBuffer(this, gsg, x_size, y_size, want_texture);
}
