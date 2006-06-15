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

#include "osxGraphicsBuffer.h"
#include "osxGraphicsStateGuardian.h"
#include "config_osxdisplay.h"
#include "osxGraphicsPipe.h"

#include "graphicsPipe.h"
#include "glgsg.h"
#include "pStatTimer.h"

TypeHandle osxGraphicsBuffer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
osxGraphicsBuffer::
osxGraphicsBuffer(GraphicsPipe *pipe,
                  const string &name,
                  const FrameBufferProperties &fb_prop,
                  const WindowProperties &win_prop,
                  int flags,
                  GraphicsStateGuardian *gsg,
                  GraphicsOutput *host) :
  GraphicsBuffer(pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  osxGraphicsPipe *osx_pipe;
  DCAST_INTO_V(osx_pipe, _pipe);
 
  _screenshot_buffer_type = _draw_buffer_type;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsBuffer::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
osxGraphicsBuffer::
~osxGraphicsBuffer() {
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsBuffer::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool osxGraphicsBuffer::
begin_frame(FrameMode mode, Thread *current_thread) {
  PStatTimer timer(_make_current_pcollector);

  begin_frame_spam();
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  osxGraphicsStateGuardian *osxgsg;
  DCAST_INTO_R(osxgsg, _gsg, false);
//  osxMakeCurrent(_display, _pbuffer, osxgsg->_context);

  osxgsg->reset_if_new();

  if (mode == FM_render) 
  {
    // begin_render_texture();
    clear_cube_map_selection();
  }
  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsBuffer::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void osxGraphicsBuffer::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam();
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

  if (mode == FM_render) 
  {
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
//     Function: osxGraphicsBuffer::close_buffer
//       Access: Protected, Virtual
//  Description: Closes the buffer right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void osxGraphicsBuffer::
close_buffer() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    //osxMakeCurrent(_display, None, NULL);
    _gsg.clear();
    _active = false;
  }
  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: osxGraphicsBuffer::open_buffer
//       Access: Protected, Virtual
//  Description: Opens the buffer right now.  Called from the window
//               thread.  Returns true if the buffer is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool osxGraphicsBuffer::
open_buffer() {

  if (_gsg == 0) {
    _gsg = new osxGraphicsStateGuardian(_pipe, NULL);
  }
  
  return false;
}

