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
OsMesaGraphicsBuffer(GraphicsPipe *pipe,
                     const string &name,
                     const FrameBufferProperties &properties,
                     int x_size, int y_size, int flags,
                     GraphicsStateGuardian *gsg,
                     GraphicsOutput *host) :
  GraphicsBuffer(pipe, name, properties, x_size, y_size, flags, gsg, host)
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
//     Function: OsMesaGraphicsBuffer::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool OsMesaGraphicsBuffer::
begin_frame(FrameMode mode, Thread *current_thread) {
  begin_frame_spam();
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  OSMesaGraphicsStateGuardian *mesagsg;
  DCAST_INTO_R(mesagsg, _gsg, false);
  OSMesaMakeCurrent(mesagsg->_context, _image.p(), _type,
                    _x_size, _y_size);

  mesagsg->reset_if_new();

  if (mode == FM_render) {
    // begin_render_texture();
    clear_cube_map_selection();
  }
  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: OsMesaGraphicsBuffer::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void OsMesaGraphicsBuffer::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam();
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

  if (mode == FM_render) {
    // end_render_texture();
    copy_to_textures();
  }

  _gsg->end_frame(current_thread);

  if (mode == FM_render) {
    trigger_flip();
    if (_one_shot) {
      prepare_for_deletion();
    }
    clear_cube_map_selection();
  }
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
  if (_gsg == 0) {
    _gsg = new OSMesaGraphicsStateGuardian(_pipe, NULL);
  }
  
  _image = PTA_uchar::empty_array(_x_size * _y_size * 4);
  _is_valid = true;
  return true;
}
