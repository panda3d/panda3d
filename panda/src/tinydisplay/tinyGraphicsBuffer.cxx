// Filename: tinyGraphicsBuffer.cxx
// Created by:  drose (08Aug08)
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

#include "pandabase.h"

#include "tinyGraphicsBuffer.h"
#include "config_tinydisplay.h"
#include "tinyGraphicsStateGuardian.h"
#include "pStatTimer.h"

TypeHandle TinyGraphicsBuffer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TinyGraphicsBuffer::
TinyGraphicsBuffer(GraphicsEngine *engine, GraphicsPipe *pipe, 
                   const string &name,
                   const FrameBufferProperties &fb_prop,
                   const WindowProperties &win_prop,
                   int flags,
                   GraphicsStateGuardian *gsg,
                   GraphicsOutput *host) :
  GraphicsBuffer(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  _frame_buffer = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsBuffer::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
TinyGraphicsBuffer::
~TinyGraphicsBuffer() {
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsBuffer::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsBuffer::
begin_frame(FrameMode mode, Thread *current_thread) {
  begin_frame_spam(mode);
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  TinyGraphicsStateGuardian *tinygsg;
  DCAST_INTO_R(tinygsg, _gsg, false);

  tinygsg->_current_frame_buffer = _frame_buffer;
  tinygsg->reset_if_new();
  
  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsBuffer::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void TinyGraphicsBuffer::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

  if (mode == FM_render) {
    // end_render_texture();
    copy_to_textures();
  }

  _gsg->end_frame(current_thread);

  if (mode == FM_render) {
    trigger_flip();
    clear_cube_map_selection();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsBuffer::close_buffer
//       Access: Protected, Virtual
//  Description: Closes the buffer right now.  Called from the buffer
//               thread.
////////////////////////////////////////////////////////////////////
void TinyGraphicsBuffer::
close_buffer() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    TinyGraphicsStateGuardian *tinygsg;
    DCAST_INTO_V(tinygsg, _gsg);
    tinygsg->_current_frame_buffer = NULL;
    _gsg.clear();
  }

  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsBuffer::open_buffer
//       Access: Protected, Virtual
//  Description: Opens the buffer right now.  Called from the buffer
//               thread.  Returns true if the buffer is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool TinyGraphicsBuffer::
open_buffer() {
  // GSG Creation/Initialization
  TinyGraphicsStateGuardian *tinygsg;
  if (_gsg == 0) {
    // There is no old gsg.  Create a new one.
    tinygsg = new TinyGraphicsStateGuardian(_engine, _pipe, NULL);
    _gsg = tinygsg;
  } else {
    DCAST_INTO_R(tinygsg, _gsg, false);
  }
  
  create_frame_buffer();
  if (_frame_buffer == NULL) {
    tinydisplay_cat.error()
      << "Could not create frame buffer.\n";
    return false;
  }

  tinygsg->_current_frame_buffer = _frame_buffer;
  
  tinygsg->reset_if_new();
  if (!tinygsg->is_valid()) {
    close_buffer();
    return false;
  }

  _is_valid = true;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGraphicsBuffer::create_frame_buffer
//       Access: Private
//  Description: Creates a suitable frame buffer for the current
//               window size.
////////////////////////////////////////////////////////////////////
void TinyGraphicsBuffer::
create_frame_buffer() {
  if (_frame_buffer != NULL) {
    ZB_close(_frame_buffer);
    _frame_buffer = NULL;
  }

  _frame_buffer = ZB_open(get_fb_x_size(), get_fb_y_size(), ZB_MODE_RGBA, 0, 0, 0, 0);
}

