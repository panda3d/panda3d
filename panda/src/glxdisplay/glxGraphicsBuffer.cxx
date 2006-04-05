// Filename: glxGraphicsBuffer.cxx
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

#include "glxGraphicsBuffer.h"
#include "glxGraphicsStateGuardian.h"
#include "config_glxdisplay.h"
#include "glxGraphicsPipe.h"

#include "graphicsPipe.h"
#include "glgsg.h"
#include "pStatTimer.h"

#ifdef HAVE_GLXFBCONFIG
// This whole class doesn't make sense unless we have the GLXFBConfig
// and associated GLXPbuffer interfaces available.

TypeHandle glxGraphicsBuffer::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
glxGraphicsBuffer::
glxGraphicsBuffer(GraphicsPipe *pipe, 
                  const string &name,
                  const FrameBufferProperties &properties,
                  int x_size, int y_size, int flags,
                  GraphicsStateGuardian *gsg,
                  GraphicsOutput *host) :
  GraphicsBuffer(pipe, name, properties, x_size, y_size, flags, gsg, host)
{
  glxGraphicsPipe *glx_pipe;
  DCAST_INTO_V(glx_pipe, _pipe);
  _display = glx_pipe->get_display();
  _pbuffer = None;

  // Since the pbuffer never gets flipped, we get screenshots from the
  // same buffer we draw into.
  _screenshot_buffer_type = _draw_buffer_type;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsBuffer::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
glxGraphicsBuffer::
~glxGraphicsBuffer() {
  nassertv(_pbuffer == None);
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsBuffer::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool glxGraphicsBuffer::
begin_frame(FrameMode mode) {
  PStatTimer timer(_make_current_pcollector);

  begin_frame_spam();
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }

  glxGraphicsStateGuardian *glxgsg;
  DCAST_INTO_R(glxgsg, _gsg, false);
  glXMakeCurrent(_display, _pbuffer, glxgsg->_context);

  // Now that we have made the context current to a window, we can
  // reset the GSG state if this is the first time it has been used.
  // (We can't just call reset() when we construct the GSG, because
  // reset() requires having a current context.)
  glxgsg->reset_if_new();

  if (mode == FM_render) {
    // begin_render_texture();
    clear_cube_map_selection();
  }
  
  _gsg()->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsBuffer::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void glxGraphicsBuffer::
end_frame(FrameMode mode) {
  end_frame_spam();
  nassertv(_gsg != (GraphicsStateGuardian *)NULL);

  if (mode == FM_render) {
    // end_render_texture();
    copy_to_textures();
  }

  _gsg->end_frame();

  if (mode == FM_render) {
    trigger_flip();
    if (_one_shot) {
      prepare_for_deletion();
    }
    clear_cube_map_selection();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsBuffer::close_buffer
//       Access: Protected, Virtual
//  Description: Closes the buffer right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void glxGraphicsBuffer::
close_buffer() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    glXMakeCurrent(_display, None, NULL);
    _gsg.clear();
    _active = false;
  }

  if (_pbuffer != None) {
    glXDestroyPbuffer(_display, _pbuffer);
    _pbuffer = None;
  }

  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsBuffer::open_buffer
//       Access: Protected, Virtual
//  Description: Opens the buffer right now.  Called from the window
//               thread.  Returns true if the buffer is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool glxGraphicsBuffer::
open_buffer() {
  glxGraphicsPipe *glx_pipe;
  DCAST_INTO_R(glx_pipe, _pipe, false);
  glxGraphicsStateGuardian *glxgsg;
  DCAST_INTO_R(glxgsg, _gsg, false);

  if (glxgsg->_fbconfig == None) {
    // If we didn't use an fbconfig to create the GSG, we can't create
    // a PBuffer.
    return false;
  }

  static const int max_attrib_list = 32;
  int attrib_list[max_attrib_list];
  int n=0;

#ifdef HAVE_OFFICIAL_GLXFBCONFIG
  // The official GLX 1.3 version passes in the size in the attrib
  // list.
  attrib_list[n++] = GLX_PBUFFER_WIDTH;
  attrib_list[n++] = _x_size;
  attrib_list[n++] = GLX_PBUFFER_HEIGHT;
  attrib_list[n++] = _y_size;

  nassertr(n < max_attrib_list, false);
  attrib_list[n] = (int)None;
  _pbuffer = glXCreatePbuffer(glxgsg->_display, glxgsg->_fbconfig,
                              attrib_list);

#else
  // The SGI version passed in the size in the parameter list.
  nassertr(n < max_attrib_list, false);
  attrib_list[n] = (int)None;
  _pbuffer = glXCreateGLXPbufferSGIX(glxgsg->_display, glxgsg->_fbconfig,
                                     _x_size, _y_size, attrib_list);
#endif

  if (_pbuffer == None) {
    glxdisplay_cat.error()
      << "failed to create GLX pbuffer.\n";
    return false;
  }

  _is_valid = true;
  return true;
}


#endif  // HAVE_GLXFBCONFIG
