// Filename: mesaGraphicsBuffer.cxx
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

#include "mesaGraphicsBuffer.h"
#include "config_mesadisplay.h"
#include "mesaGraphicsPipe.h"
#include "mesaGraphicsStateGuardian.h"

TypeHandle MesaGraphicsBuffer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MesaGraphicsBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
MesaGraphicsBuffer::
MesaGraphicsBuffer(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                   int x_size, int y_size, bool want_texture) :
  GraphicsBuffer(pipe, gsg, x_size, y_size, want_texture) 
{
  _type = GL_UNSIGNED_BYTE;
}

////////////////////////////////////////////////////////////////////
//     Function: MesaGraphicsBuffer::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
MesaGraphicsBuffer::
~MesaGraphicsBuffer() {
}

////////////////////////////////////////////////////////////////////
//     Function: MesaGraphicsBuffer::make_current
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               during begin_frame() to ensure the graphics context
//               is ready for drawing.
////////////////////////////////////////////////////////////////////
void MesaGraphicsBuffer::
make_current() {
  MesaGraphicsStateGuardian *mesagsg;
  DCAST_INTO_V(mesagsg, _gsg);
  OSMesaMakeCurrent(mesagsg->_context, _image.p(), _type,
                    _x_size, _y_size);

  mesagsg->reset_if_new();
}

////////////////////////////////////////////////////////////////////
//     Function: MesaGraphicsBuffer::begin_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after end_frame() has been called on all windows, to
//               initiate the exchange of the front and back buffers.
//
//               This should instruct the window to prepare for the
//               flip at the next video sync, but it should not wait.
//
//               We have the two separate functions, begin_flip() and
//               end_flip(), to make it easier to flip all of the
//               windows at the same time.
////////////////////////////////////////////////////////////////////
void MesaGraphicsBuffer::
begin_flip() {
  if (has_texture()) {
    // Use glCopyTexImage2D to copy the framebuffer to the texture.
    // This appears to be the only way to "render to a texture" in
    // OpenGL; there's no interface to make the offscreen buffer
    // itself be a texture.
    DisplayRegion dr(_x_size, _y_size);
    get_texture()->copy(_gsg, &dr, _gsg->get_render_buffer(RenderBuffer::T_back));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MesaGraphicsBuffer::close_buffer
//       Access: Protected, Virtual
//  Description: Closes the buffer right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void MesaGraphicsBuffer::
close_buffer() {
  _image.clear();
  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: MesaGraphicsBuffer::open_buffer
//       Access: Protected, Virtual
//  Description: Opens the buffer right now.  Called from the window
//               thread.  Returns true if the buffer is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool MesaGraphicsBuffer::
open_buffer() {
  _image = PTA_uchar::empty_array(_x_size * _y_size * 4);
  _is_valid = true;
  return true;
}
