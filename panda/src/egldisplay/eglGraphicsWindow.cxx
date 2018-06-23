/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eglGraphicsWindow.cxx
 * @author rdb
 * @date 2009-05-21
 */

#include "eglGraphicsWindow.h"
#include "eglGraphicsStateGuardian.h"
#include "config_egldisplay.h"
#include "eglGraphicsPipe.h"

#include "graphicsPipe.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "clockObject.h"
#include "pStatTimer.h"
#include "textEncoder.h"
#include "throw_event.h"
#include "lightReMutexHolder.h"
#include "nativeWindowHandle.h"
#include "get_x11.h"

TypeHandle eglGraphicsWindow::_type_handle;

/**
 *
 */
eglGraphicsWindow::
eglGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                  const std::string &name,
                  const FrameBufferProperties &fb_prop,
                  const WindowProperties &win_prop,
                  int flags,
                  GraphicsStateGuardian *gsg,
                  GraphicsOutput *host) :
  x11GraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  eglGraphicsPipe *egl_pipe;
  DCAST_INTO_V(egl_pipe, _pipe);
  _egl_display = egl_pipe->_egl_display;
  _egl_surface = 0;
}

/**
 *
 */
eglGraphicsWindow::
~eglGraphicsWindow() {
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool eglGraphicsWindow::
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

  eglGraphicsStateGuardian *eglgsg;
  DCAST_INTO_R(eglgsg, _gsg, false);
  {
    LightReMutexHolder holder(eglGraphicsPipe::_x_mutex);

    if (eglGetCurrentDisplay() == _egl_display &&
        eglGetCurrentSurface(EGL_READ) == _egl_surface &&
        eglGetCurrentSurface(EGL_DRAW) == _egl_surface &&
        eglGetCurrentContext() == eglgsg->_context) {
      // No need to make the context current again.  Short-circuit this
      // possibly-expensive call.
    } else {
      // Need to set the context.
      if (!eglMakeCurrent(_egl_display, _egl_surface, _egl_surface, eglgsg->_context)) {
        egldisplay_cat.error() << "Failed to call eglMakeCurrent: "
          << get_egl_error_string(eglGetError()) << "\n";
      }
    }
  }

  // Now that we have made the context current to a window, we can reset the
  // GSG state if this is the first time it has been used.  (We can't just
  // call reset() when we construct the GSG, because reset() requires having a
  // current context.)
  eglgsg->reset_if_new();

  if (mode == FM_render) {
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
void eglGraphicsWindow::
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
  }
}

/**
 * This function will be called within the draw thread after begin_flip() has
 * been called on all windows, to finish the exchange of the front and back
 * buffers.
 *
 * This should cause the window to wait for the flip, if necessary.
 */
void eglGraphicsWindow::
end_flip() {
  if (_gsg != nullptr && _flip_ready) {

    // It doesn't appear to be necessary to ensure the graphics context is
    // current before flipping the windows, and insisting on doing so can be a
    // significant performance hit.

    // make_current();

    LightReMutexHolder holder(eglGraphicsPipe::_x_mutex);
    eglSwapBuffers(_egl_display, _egl_surface);
  }
  GraphicsWindow::end_flip();
}

/**
 * Closes the window right now.  Called from the window thread.
 */
void eglGraphicsWindow::
close_window() {
  if (_gsg != nullptr) {
    if (!eglMakeCurrent(_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
      egldisplay_cat.error() << "Failed to call eglMakeCurrent: "
        << get_egl_error_string(eglGetError()) << "\n";
    }
    _gsg.clear();
  }

  if (_ic != (XIC)nullptr) {
    XDestroyIC(_ic);
    _ic = (XIC)nullptr;
  }

  if (_egl_surface != 0) {
    if (!eglDestroySurface(_egl_display, _egl_surface)) {
      egldisplay_cat.error() << "Failed to destroy surface: "
        << get_egl_error_string(eglGetError()) << "\n";
    }
  }

  if (_xwindow != (X11_Window)nullptr) {
    XDestroyWindow(_display, _xwindow);
    _xwindow = (X11_Window)nullptr;

    // This may be necessary if we just closed the last X window in an
    // application, so the server hears the close request.
    XFlush(_display);
  }
  GraphicsWindow::close_window();
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool eglGraphicsWindow::
open_window() {
  eglGraphicsPipe *egl_pipe;
  DCAST_INTO_R(egl_pipe, _pipe, false);

  // GSG CreationInitialization
  eglGraphicsStateGuardian *eglgsg;
  if (_gsg == 0) {
    // There is no old gsg.  Create a new one.
    eglgsg = new eglGraphicsStateGuardian(_engine, _pipe, nullptr);
    eglgsg->choose_pixel_format(_fb_properties, egl_pipe->get_display(), egl_pipe->get_screen(), false, false);
    _gsg = eglgsg;
  } else {
    // If the old gsg has the wrong pixel format, create a new one that shares
    // with the old gsg.
    DCAST_INTO_R(eglgsg, _gsg, false);
    if (!eglgsg->get_fb_properties().subsumes(_fb_properties)) {
      eglgsg = new eglGraphicsStateGuardian(_engine, _pipe, eglgsg);
      eglgsg->choose_pixel_format(_fb_properties, egl_pipe->get_display(), egl_pipe->get_screen(), false, false);
      _gsg = eglgsg;
    }
  }

  _visual_info = eglgsg->_visual;
  if (_visual_info == nullptr) {
    // No X visual for this fbconfig; how can we open the window?
    egldisplay_cat.error()
      << "No X visual: cannot open window.\n";
    return false;
  }

  setup_colormap(_visual_info);

  if (!x11GraphicsWindow::open_window()) {
    return false;
  }

  _egl_surface = eglCreateWindowSurface(_egl_display, eglgsg->_fbconfig, (NativeWindowType) _xwindow, nullptr);
  if (eglGetError() != EGL_SUCCESS) {
    egldisplay_cat.error()
      << "Failed to create window surface.\n";
    return false;
  }

  if (!eglMakeCurrent(_egl_display, _egl_surface, _egl_surface, eglgsg->_context)) {
    egldisplay_cat.error() << "Failed to call eglMakeCurrent: "
      << get_egl_error_string(eglGetError()) << "\n";
  }
  eglgsg->reset_if_new();
  if (!eglgsg->is_valid()) {
    close_window();
    return false;
  }
  if (!eglgsg->get_fb_properties().verify_hardware_software
      (_fb_properties, eglgsg->get_gl_renderer())) {
    close_window();
    return false;
  }
  _fb_properties = eglgsg->get_fb_properties();

  return true;
}
