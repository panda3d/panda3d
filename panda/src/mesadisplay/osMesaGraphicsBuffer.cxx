// Filename: osMesaGraphicsBuffer.cxx
// Created by:  drose (09Feb04)
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

#include "osMesaGraphicsBuffer.h"
#include "config_mesadisplay.h"
#include "osMesaGraphicsPipe.h"
#include "osMesaGraphicsStateGuardian.h"

TypeHandle OsMesaGraphicsBuffer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
OsMesaGraphicsBuffer::
OsMesaGraphicsBuffer(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                     const string &name,
                     int x_size, int y_size) :
  GraphicsBuffer(pipe, gsg, name, x_size, y_size) 
{
  _type = GL_UNSIGNED_BYTE;
}

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsBuffer::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
OsMesaGraphicsBuffer::
~OsMesaGraphicsBuffer() {
}

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsBuffer::make_current
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               during begin_frame() to ensure the graphics context
//               is ready for drawing.
////////////////////////////////////////////////////////////////////
void OsMesaGraphicsBuffer::
make_current() {
  OSMesaGraphicsStateGuardian *mesagsg;
  DCAST_INTO_V(mesagsg, _gsg);
  OSMesaMakeCurrent(mesagsg->_context, _image.p(), _type,
                    _x_size, _y_size);

  mesagsg->reset_if_new();
}

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsBuffer::close_buffer
//       Access: Protected, Virtual
//  Description: Closes the buffer right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void OsMesaGraphicsBuffer::
close_buffer() {
  _image.clear();
  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsBuffer::open_buffer
//       Access: Protected, Virtual
//  Description: Opens the buffer right now.  Called from the window
//               thread.  Returns true if the buffer is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool OsMesaGraphicsBuffer::
open_buffer() {
  _image = PTA_uchar::empty_array(_x_size * _y_size * 4);
  _is_valid = true;
  return true;
}
