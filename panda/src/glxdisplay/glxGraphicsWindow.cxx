/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glxGraphicsWindow.cxx
 * @author mike
 * @date 1997-01-09
 */

#include "glxGraphicsWindow.h"
#include "glxGraphicsStateGuardian.h"
#include "config_glxdisplay.h"
#include "glxGraphicsPipe.h"

#include "graphicsPipe.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "glgsg.h"
#include "clockObject.h"
#include "pStatTimer.h"
#include "textEncoder.h"
#include "throw_event.h"
#include "lightReMutexHolder.h"

#include <errno.h>
#include <sys/time.h>

TypeHandle glxGraphicsWindow::_type_handle;

/**
 *
 */
glxGraphicsWindow::
glxGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                  const std::string &name,
                  const FrameBufferProperties &fb_prop,
                  const WindowProperties &win_prop,
                  int flags,
                  GraphicsStateGuardian *gsg,
                  GraphicsOutput *host) :
  x11GraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool glxGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  PStatTimer timer(_make_current_pcollector, current_thread);

  begin_frame_spam(mode);
  if (_gsg == nullptr) {
    return false;
  }
  if (_awaiting_configure) {
    // Don't attempt to draw while we have just reconfigured the window and we
    // haven't got the notification back yet.
    return false;
  }

  glxGraphicsStateGuardian *glxgsg;
  DCAST_INTO_R(glxgsg, _gsg, false);
  {
    LightReMutexHolder holder(glxGraphicsPipe::_x_mutex);

    if (glXGetCurrentDisplay() == _display &&
        glXGetCurrentDrawable() == _xwindow &&
        glXGetCurrentContext() == glxgsg->_context) {
      // No need to make the context current again.  Short-circuit this
      // possibly-expensive call.
    } else {
      // Need to set the context.
      glXMakeCurrent(_display, _xwindow, glxgsg->_context);
    }
  }

  // Now that we have made the context current to a window, we can reset the
  // GSG state if this is the first time it has been used.  (We can't just
  // call reset() when we construct the GSG, because reset() requires having a
  // current context.)
  glxgsg->reset_if_new();

  if (mode == FM_render) {
    glxgsg->push_group_marker(std::string("glxGraphicsWindow ") + get_name());
    // begin_render_texture();
    clear_cube_map_selection();
  }

  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}


/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void glxGraphicsWindow::
end_frame(FrameMode mode, Thread *current_thread) {
  end_frame_spam(mode);
  nassertv(_gsg != nullptr);

  if (mode == FM_render) {
    // end_render_texture();
    copy_to_textures();
  }

  _gsg->end_frame(current_thread);

  if (mode == FM_render) {
    trigger_flip();
    clear_cube_map_selection();

    glxGraphicsStateGuardian *glxgsg;
    DCAST_INTO_V(glxgsg, _gsg);
    glxgsg->pop_group_marker();
  }
}

/**
 * This function will be called within the draw thread after begin_flip() has
 * been called on all windows, to finish the exchange of the front and back
 * buffers.
 *
 * This should cause the window to wait for the flip, if necessary.
 */
void glxGraphicsWindow::
end_flip() {
  if (_gsg != nullptr && _flip_ready) {

    // It doesn't appear to be necessary to ensure the graphics context is
    // current before flipping the windows, and insisting on doing so can be a
    // significant performance hit.

    // make_current();

    LightReMutexHolder holder(glxGraphicsPipe::_x_mutex);
    glXSwapBuffers(_display, _xwindow);
  }
  GraphicsWindow::end_flip();
}

/**
 * Closes the window right now.  Called from the window thread.
 */
void glxGraphicsWindow::
close_window() {
  LightReMutexHolder holder(glxGraphicsPipe::_x_mutex);

  if (_gsg != nullptr) {
    glXMakeCurrent(_display, None, nullptr);
    _gsg.clear();
  }

  x11GraphicsWindow::close_window();
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool glxGraphicsWindow::
open_window() {
  glxGraphicsPipe *glx_pipe;
  DCAST_INTO_R(glx_pipe, _pipe, false);

  // GSG CreationInitialization
  glxGraphicsStateGuardian *glxgsg;
  if (_gsg == nullptr) {
    // There is no old gsg.  Create a new one.
    glxgsg = new glxGraphicsStateGuardian(_engine, _pipe, nullptr);
    glxgsg->choose_pixel_format(_fb_properties, glx_pipe->get_display(), glx_pipe->get_screen(), false, false);
    _gsg = glxgsg;
  } else {
    // If the old gsg has the wrong pixel format, create a new one that shares
    // with the old gsg.
    DCAST_INTO_R(glxgsg, _gsg, false);
    if (!glxgsg->get_fb_properties().subsumes(_fb_properties)) {
      glxgsg = new glxGraphicsStateGuardian(_engine, _pipe, glxgsg);
      glxgsg->choose_pixel_format(_fb_properties, glx_pipe->get_display(), glx_pipe->get_screen(), false, false);
      _gsg = glxgsg;
    }
  }

  if (glxgsg->_context == nullptr) {
    // We're supposed to have a context at this point.
    glxdisplay_cat.error()
      << "No GLX context: cannot open window.\n";
    return false;
  }

  _visual_info = glxgsg->_visual;
  if (_visual_info == nullptr) {
    // No X visual for this fbconfig; how can we open the window?
    glxdisplay_cat.error()
      << "No X visual: cannot open window.\n";
    return false;
  }

  LightReMutexHolder holder(glxGraphicsPipe::_x_mutex);

  if (glxgsg->_fbconfig != None) {
    setup_colormap(glxgsg->_fbconfig);
  } else {
    setup_colormap(_visual_info);
  }

  if (!x11GraphicsWindow::open_window()) {
    return false;
  }

  glXMakeCurrent(_display, _xwindow, glxgsg->_context);
  glxgsg->reset_if_new();
  if (!glxgsg->is_valid()) {
    close_window();
    return false;
  }
  if (!glxgsg->get_fb_properties().verify_hardware_software
      (_fb_properties, glxgsg->get_gl_renderer())) {
    close_window();
    return false;
  }
  _fb_properties = glxgsg->get_fb_properties();

  return true;
}

/**
 * Allocates a colormap appropriate to the fbconfig and stores in in the
 * _colormap method.
 */
void glxGraphicsWindow::
setup_colormap(GLXFBConfig fbconfig) {
  glxGraphicsStateGuardian *glxgsg;
  DCAST_INTO_V(glxgsg, _gsg);
  nassertv(glxgsg->_supports_fbconfig);

  XVisualInfo *visual_info = glxgsg->_glXGetVisualFromFBConfig(_display, fbconfig);
  if (visual_info == nullptr) {
    // No X visual; no need to set up a colormap.
    return;
  }
  int visual_class = visual_info->c_class;
  Visual *visual = visual_info->visual;
  XFree(visual_info);

  glxGraphicsPipe *glx_pipe;
  DCAST_INTO_V(glx_pipe, _pipe);
  X11_Window root_window = glx_pipe->get_root();

  int rc, is_rgb;

  switch (visual_class) {
    case PseudoColor:
      rc = glxgsg->_glXGetFBConfigAttrib(_display, fbconfig, GLX_RGBA, &is_rgb);
      if (rc == 0 && is_rgb) {
        glxdisplay_cat.warning()
          << "mesa pseudocolor not supported.\n";
        // this is a terrible terrible hack, but it seems to work
        _colormap = (Colormap)0;

      } else {
        _colormap = XCreateColormap(_display, root_window,
                                    visual, AllocAll);
      }
      break;
    case TrueColor:
    case DirectColor:
      _colormap = XCreateColormap(_display, root_window,
                                  visual, AllocNone);
      break;
    case StaticColor:
    case StaticGray:
    case GrayScale:
      _colormap = XCreateColormap(_display, root_window,
                                  visual, AllocNone);
      break;
    default:
      glxdisplay_cat.error()
        << "Could not allocate a colormap for visual class "
        << visual_class << ".\n";
      break;
  }
}

/**
 * Allocates a colormap appropriate to the visual and stores in in the
 * _colormap method.
 */
void glxGraphicsWindow::
setup_colormap(XVisualInfo *visual) {
  glxGraphicsPipe *glx_pipe;
  DCAST_INTO_V(glx_pipe, _pipe);
  X11_Window root_window = glx_pipe->get_root();

  int visual_class = visual->c_class;
  int rc, is_rgb;

  switch (visual_class) {
    case PseudoColor:
      rc = glXGetConfig(_display, visual, GLX_RGBA, &is_rgb);
      if (rc == 0 && is_rgb) {
        glxdisplay_cat.warning()
          << "mesa pseudocolor not supported.\n";
        // this is a terrible terrible hack, but it seems to work
        _colormap = (Colormap)0;

      } else {
        _colormap = XCreateColormap(_display, root_window,
                                    visual->visual, AllocAll);
      }
      break;
    case TrueColor:
    case DirectColor:
      _colormap = XCreateColormap(_display, root_window,
                                  visual->visual, AllocNone);
      break;
    case StaticColor:
    case StaticGray:
    case GrayScale:
      _colormap = XCreateColormap(_display, root_window,
                                  visual->visual, AllocNone);
      break;
    default:
      glxdisplay_cat.error()
        << "Could not allocate a colormap for visual class "
        << visual_class << ".\n";
      break;
  }
}
