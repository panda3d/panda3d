// Filename: osMesaGraphicsBuffer.cxx
// Created by:  drose (09Feb04)
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
OsMesaGraphicsBuffer(GraphicsEngine *engine, GraphicsPipe *pipe,
                     const string &name,
                     const FrameBufferProperties &fb_prop,
                     const WindowProperties &win_prop,
                     int flags,
                     GraphicsStateGuardian *gsg,
                     GraphicsOutput *host) :
  GraphicsBuffer(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  _type = GL_UNSIGNED_BYTE;
  _screenshot_buffer_type = _draw_buffer_type;
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
  begin_frame_spam(mode);
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  OSMesaGraphicsStateGuardian *mesagsg;
  DCAST_INTO_R(mesagsg, _gsg, false);
  OSMesaMakeCurrent(mesagsg->_context, _image.p(), _type,
                    _x_size, _y_size);

  mesagsg->reset_if_new();

  if (mode == FM_render) {
    CDLockedReader cdata(_cycler);
    for (size_t i = 0; i != cdata->_textures.size(); ++i) {
      const RenderTexture &rt = cdata->_textures[i];
      RenderTextureMode rtm_mode = rt._rtm_mode;
      if (rtm_mode == RTM_bind_or_copy) {
        CDWriter cdataw(_cycler, cdata, false);
        nassertr(cdata->_textures.size() == cdataw->_textures.size(), false);
        cdataw->_textures[i]._rtm_mode = RTM_copy_texture;
      }
    }
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
  end_frame_spam(mode);
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

  if (mode == FM_render) {
    copy_to_textures();
  }

  _gsg->end_frame(current_thread);

  if (mode == FM_render) {
    trigger_flip();
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
    _gsg = new OSMesaGraphicsStateGuardian(_engine, _pipe, NULL);
  }
  
  _image = PTA_uchar::empty_array(_x_size * _y_size * 4);
  _is_valid = true;

  // All OSMesa buffers (that we create) always have the same format.
  _fb_properties.clear();
  _fb_properties.set_rgb_color(1);
  _fb_properties.set_color_bits(24);
  _fb_properties.set_alpha_bits(8);
  _fb_properties.set_stencil_bits(8);
  _fb_properties.set_depth_bits(16);
  _fb_properties.set_accum_bits(8);
  _fb_properties.set_force_software(1);

  return true;
}
