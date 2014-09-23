// Filename: glxGraphicsPixmap.cxx
// Created by:  drose (10Mar09)
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

#include "glxGraphicsPixmap.h"
#include "glxGraphicsWindow.h"
#include "glxGraphicsStateGuardian.h"
#include "config_glxdisplay.h"
#include "glxGraphicsPipe.h"

#include "graphicsPipe.h"
#include "glgsg.h"
#include "pStatTimer.h"

TypeHandle glxGraphicsPixmap::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPixmap::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
glxGraphicsPixmap::
glxGraphicsPixmap(GraphicsEngine *engine, GraphicsPipe *pipe, 
                  const string &name,
                  const FrameBufferProperties &fb_prop,
                  const WindowProperties &win_prop,
                  int flags,
                  GraphicsStateGuardian *gsg,
                  GraphicsOutput *host) :
  GraphicsBuffer(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  glxGraphicsPipe *glx_pipe;
  DCAST_INTO_V(glx_pipe, _pipe);
  _display = glx_pipe->get_display();
  _drawable = None;
  _x_pixmap = None;
  _glx_pixmap = None;

  // Since the pixmap never gets flipped, we get screenshots from the
  // same pixmap we draw into.
  _screenshot_buffer_type = _draw_buffer_type;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPixmap::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
glxGraphicsPixmap::
~glxGraphicsPixmap() {
  nassertv(_x_pixmap == None && _glx_pixmap == None);
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPixmap::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool glxGraphicsPixmap::
begin_frame(FrameMode mode, Thread *current_thread) {
  PStatTimer timer(_make_current_pcollector, current_thread);

  begin_frame_spam(mode);
  if (_gsg == (GraphicsStateGuardian *)NULL ||
      _glx_pixmap == None) {
    return false;
  }

  glxGraphicsStateGuardian *glxgsg;
  DCAST_INTO_R(glxgsg, _gsg, false);
  glXMakeCurrent(_display, _glx_pixmap, glxgsg->_context);

  // Now that we have made the context current to a window, we can
  // reset the GSG state if this is the first time it has been used.
  // (We can't just call reset() when we construct the GSG, because
  // reset() requires having a current context.)
  glxgsg->reset_if_new();

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
//     Function: glxGraphicsPixmap::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void glxGraphicsPixmap::
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
//     Function: glxGraphicsPixmap::close_buffer
//       Access: Protected, Virtual
//  Description: Closes the pixmap right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void glxGraphicsPixmap::
close_buffer() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    glXMakeCurrent(_display, None, NULL);
    _gsg.clear();
  }

  if (_glx_pixmap != None) {
    glXDestroyGLXPixmap(_display, _glx_pixmap);
    _glx_pixmap = None;
  }

  if (_x_pixmap != None) {
    XFreePixmap(_display, _x_pixmap);
    _x_pixmap = None;
  }

  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsPixmap::open_buffer
//       Access: Protected, Virtual
//  Description: Opens the pixmap right now.  Called from the window
//               thread.  Returns true if the pixmap is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool glxGraphicsPixmap::
open_buffer() {
  glxGraphicsPipe *glx_pipe;
  DCAST_INTO_R(glx_pipe, _pipe, false);

  // GSG Creation/Initialization
  glxGraphicsStateGuardian *glxgsg;
  if (_gsg == 0) {
    // There is no old gsg.  Create a new one.
    glxgsg = new glxGraphicsStateGuardian(_engine, _pipe, NULL);
    glxgsg->choose_pixel_format(_fb_properties, _display, glx_pipe->get_screen(), false, true);
    _gsg = glxgsg;
  } else {
    // If the old gsg has the wrong pixel format, create a
    // new one that shares with the old gsg.
    DCAST_INTO_R(glxgsg, _gsg, false);
    if (!glxgsg->_context_has_pixmap || 
        !glxgsg->get_fb_properties().subsumes(_fb_properties)) {
      glxgsg = new glxGraphicsStateGuardian(_engine, _pipe, glxgsg);
      glxgsg->choose_pixel_format(_fb_properties, _display, glx_pipe->get_screen(), false, true);
      _gsg = glxgsg;
    }
  }

  if (!glxgsg->_context_has_pixmap) {
    // Hmm, the GSG we created won't work.
    return false;
  }

  XVisualInfo *visual_info = glxgsg->_visual;
  if (visual_info == NULL) {
    // No X visual for this fbconfig; how can we create the pixmap?
    glxdisplay_cat.error()
      << "No X visual: cannot create pixmap.\n";
    return false;
  }

  _drawable = glx_pipe->get_root();
  if (_host != NULL) {
    if (_host->is_of_type(glxGraphicsWindow::get_class_type())) {
      glxGraphicsWindow *win = DCAST(glxGraphicsWindow, _host);
      _drawable = win->get_xwindow();
    } else if (_host->is_of_type(glxGraphicsPixmap::get_class_type())) {
      glxGraphicsPixmap *pix = DCAST(glxGraphicsPixmap, _host);
      _drawable = pix->_drawable;
    }
  }

  _x_pixmap = XCreatePixmap(_display, _drawable,
                            get_x_size(), get_y_size(), visual_info->depth);
  if (_x_pixmap == None) {
    glxdisplay_cat.error()
      << "Failed to create X pixmap.\n";
    close_buffer();
    return false;
  }

  if (glxgsg->_fbconfig) {
    // Use the FBConfig to create the pixmap.
    _glx_pixmap = glxgsg->_glXCreatePixmap(_display, glxgsg->_fbconfig, _x_pixmap, NULL);
  } else {
    // Use the XVisual to create the pixmap.
    _glx_pixmap = glXCreateGLXPixmap(_display, visual_info, _x_pixmap);
  }

  if (_glx_pixmap == None) {
    glxdisplay_cat.error()
      << "Failed to create GLX pixmap.\n";
    close_buffer();
    return false;
  }

  int error_count = x11GraphicsPipe::disable_x_error_messages();
  glXMakeCurrent(_display, _glx_pixmap, glxgsg->_context);
  if (x11GraphicsPipe::enable_x_error_messages() != error_count) {
    // An error was generated during the glXMakeCurrent() call.
    // Assume the worst.
    close_buffer();
    return false;
  }

  glxgsg->reset_if_new();
  if (!glxgsg->is_valid()) {
    close_buffer();
    return false;
  }
  if (!glxgsg->get_fb_properties().verify_hardware_software
      (_fb_properties, glxgsg->get_gl_renderer())) {
    close_buffer();
    return false;
  }
  _fb_properties = glxgsg->get_fb_properties();
  
  _is_valid = true;
  return true;
}
