// Filename: eglGraphicsPipe.cxx
// Created by:  pro-rsoft (21May09)
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

#include "eglGraphicsBuffer.h"
#include "eglGraphicsPipe.h"
#include "eglGraphicsPixmap.h"
#include "eglGraphicsWindow.h"
#include "eglGraphicsStateGuardian.h"
#include "config_egldisplay.h"
#include "frameBufferProperties.h"

TypeHandle eglGraphicsPipe::_type_handle;

bool eglGraphicsPipe::_error_handlers_installed = false;
eglGraphicsPipe::ErrorHandlerFunc *eglGraphicsPipe::_prev_error_handler;
eglGraphicsPipe::IOErrorHandlerFunc *eglGraphicsPipe::_prev_io_error_handler;

LightReMutex eglGraphicsPipe::_x_mutex;

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsPipe::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
eglGraphicsPipe::
eglGraphicsPipe(const string &display) {
  string display_spec = display;
  if (display_spec.empty()) {
    display_spec = display_cfg;
  }
  if (display_spec.empty()) {
    display_spec = ExecutionEnvironment::get_environment_variable("DISPLAY");
  }
  if (display_spec.empty()) {
    display_spec = ":0.0";
  }

  // The X docs say we should do this to get international character
  // support from the keyboard.
  setlocale(LC_ALL, "");

  // But it's important that we use the "C" locale for numeric
  // formatting, since all of the internal Panda code assumes this--we
  // need a decimal point to mean a decimal point.
  setlocale(LC_NUMERIC, "C");

  _is_valid = false;
  _supported_types = OT_window | OT_buffer | OT_texture_buffer;
  _display = NULL;
  _screen = 0;
  _root = (X11_Window)NULL;
  _im = (XIM)NULL;
  _hidden_cursor = None;
  _egl_display = NULL;

  install_error_handlers();

  _display = XOpenDisplay(display_spec.c_str());
  if (!_display) {
    egldisplay_cat.error()
      << "Could not open display \"" << display_spec << "\".\n";
    return;
  }

  if (!XSupportsLocale()) {
    egldisplay_cat.warning()
      << "X does not support locale " << setlocale(LC_ALL, NULL) << "\n";
  }
  XSetLocaleModifiers("");

  _screen = DefaultScreen(_display);
  _root = RootWindow(_display, _screen);
  _display_width = DisplayWidth(_display, _screen);
  _display_height = DisplayHeight(_display, _screen);
  _is_valid = true;

  _egl_display = eglGetDisplay((NativeDisplayType) _display);
  if (!eglInitialize(_egl_display, NULL, NULL)) {
    egldisplay_cat.error()
      << "Couldn't initialize the EGL display: "
      << get_egl_error_string(eglGetError()) << "\n";
  }

  if (!eglBindAPI(EGL_OPENGL_ES_API)) {
    egldisplay_cat.error()
      << "Couldn't bind EGL to the OpenGL ES API: "
      << get_egl_error_string(eglGetError()) << "\n";
  }

  // Connect to an input method for supporting international text
  // entry.
  _im = XOpenIM(_display, NULL, NULL, NULL);
  if (_im == (XIM)NULL) {
    egldisplay_cat.warning()
      << "Couldn't open input method.\n";
  }

  // What styles does the current input method support?
  /*
  XIMStyles *im_supported_styles;
  XGetIMValues(_im, XNQueryInputStyle, &im_supported_styles, NULL);

  for (int i = 0; i < im_supported_styles->count_styles; i++) {
    XIMStyle style = im_supported_styles->supported_styles[i];
    cerr << "style " << i << ". " << hex << style << dec << "\n";
  }

  XFree(im_supported_styles);
  */

  // Get some X atom numbers.
  _wm_delete_window = XInternAtom(_display, "WM_DELETE_WINDOW", false);
  _net_wm_window_type = XInternAtom(_display, "_NET_WM_WINDOW_TYPE", false);
  _net_wm_window_type_splash = XInternAtom(_display, "_NET_WM_WINDOW_TYPE_SPLASH", false);
  _net_wm_window_type_fullscreen = XInternAtom(_display, "_NET_WM_WINDOW_TYPE_FULLSCREEN", false);
  _net_wm_state = XInternAtom(_display, "_NET_WM_STATE", false);
  _net_wm_state_fullscreen = XInternAtom(_display, "_NET_WM_STATE_FULLSCREEN", false);
  _net_wm_state_above = XInternAtom(_display, "_NET_WM_STATE_ABOVE", false);
  _net_wm_state_below = XInternAtom(_display, "_NET_WM_STATE_BELOW", false);
  _net_wm_state_add = XInternAtom(_display, "_NET_WM_STATE_ADD", false);
  _net_wm_state_remove = XInternAtom(_display, "_NET_WM_STATE_REMOVE", false);
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsPipe::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
eglGraphicsPipe::
~eglGraphicsPipe() {
  release_hidden_cursor();
  if (_im) {
    XCloseIM(_im);
  }
  if (_display) {
    XCloseDisplay(_display);
  }
  if (_egl_display) {
    if (!eglTerminate(_egl_display)) {
      egldisplay_cat.error() << "Failed to terminate EGL display: "
        << get_egl_error_string(eglGetError()) << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsPipe::get_interface_name
//       Access: Published, Virtual
//  Description: Returns the name of the rendering interface
//               associated with this GraphicsPipe.  This is used to
//               present to the user to allow him/her to choose
//               between several possible GraphicsPipes available on a
//               particular platform, so the name should be meaningful
//               and unique for a given platform.
////////////////////////////////////////////////////////////////////
string eglGraphicsPipe::
get_interface_name() const {
  return "OpenGL ES";
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsPipe::pipe_constructor
//       Access: Public, Static
//  Description: This function is passed to the GraphicsPipeSelection
//               object to allow the user to make a default
//               eglGraphicsPipe.
////////////////////////////////////////////////////////////////////
PT(GraphicsPipe) eglGraphicsPipe::
pipe_constructor() {
  return new eglGraphicsPipe;
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsPipe::get_preferred_window_thread
//       Access: Public, Virtual
//  Description: Returns an indication of the thread in which this
//               GraphicsPipe requires its window processing to be
//               performed: typically either the app thread (e.g. X)
//               or the draw thread (Windows).
////////////////////////////////////////////////////////////////////
GraphicsPipe::PreferredWindowThread
eglGraphicsPipe::get_preferred_window_thread() const {
  // Actually, since we're creating the graphics context in
  // open_window() now, it appears we need to ensure the open_window()
  // call is performed in the draw thread for now, even though X wants
  // all of its calls to be single-threaded.

  // This means that all X windows may have to be handled by the same
  // draw thread, which we didn't intend (though the global _x_mutex
  // may allow them to be technically served by different threads,
  // even though the actual X calls will be serialized).  There might
  // be a better way.

  return PWT_draw;
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsPipe::make_output
//       Access: Protected, Virtual
//  Description: Creates a new window on the pipe, if possible.
////////////////////////////////////////////////////////////////////
PT(GraphicsOutput) eglGraphicsPipe::
make_output(const string &name,
            const FrameBufferProperties &fb_prop,
            const WindowProperties &win_prop,
            int flags,
            GraphicsEngine *engine,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {

  if (!_is_valid) {
    return NULL;
  }

  eglGraphicsStateGuardian *eglgsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(eglgsg, gsg, NULL);
  }

  bool support_rtt;
  support_rtt = false;
  /*
    Currently, no support for eglGraphicsBuffer render-to-texture.
  if (eglgsg) {
     support_rtt =
      eglgsg -> get_supports_render_texture() &&
      support_render_texture;
  }
  */

  // First thing to try: an eglGraphicsWindow

  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return NULL;
    }
    return new eglGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
  }

  // Second thing to try: a GLES(2)GraphicsBuffer
  if (retry == 1) {
    if ((host==0)||
  //        (!gl_support_fbo)||
        ((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)) {
      return NULL;
    }
    // Early failure - if we are sure that this buffer WONT
    // meet specs, we can bail out early.
    if ((flags & BF_fb_props_optional)==0) {
      if ((fb_prop.get_indexed_color() > 0)||
          (fb_prop.get_back_buffers() > 0)||
          (fb_prop.get_accum_bits() > 0)||
          (fb_prop.get_multisamples() > 0)) {
        return NULL;
      }
    }
    // Early success - if we are sure that this buffer WILL
    // meet specs, we can precertify it.
    if ((eglgsg != 0) &&
        (eglgsg->is_valid()) &&
        (!eglgsg->needs_reset()) &&
        (eglgsg->_supports_framebuffer_object) &&
        (eglgsg->_glDrawBuffers != 0)&&
        (fb_prop.is_basic())) {
      precertify = true;
    }
#ifdef OPENGLES_2
    return new GLES2GraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                  flags, gsg, host);
#else
    return new GLESGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                  flags, gsg, host);
#endif
  }

  // Third thing to try: a eglGraphicsBuffer
  if (retry == 2) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)) {
      return NULL;
    }

    if (!support_rtt) {
      if (((flags&BF_rtt_cumulative)!=0)||
          ((flags&BF_can_bind_every)!=0)) {
        // If we require Render-to-Texture, but can't be sure we
        // support it, bail.
        return NULL;
      }
    }

    return new eglGraphicsBuffer(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
  }

  // Fourth thing to try: an eglGraphicsPixmap.
  if (retry == 3) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_require_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)) {
      return NULL;
    }

    if (((flags&BF_rtt_cumulative)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return NULL;
    }

    return new eglGraphicsPixmap(engine, this, name, fb_prop, win_prop,
                                 flags, gsg, host);
  }

  // Nothing else left to try.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsPipe::make_hidden_cursor
//       Access: Private
//  Description: Called once to make an invisible Cursor for return
//               from get_hidden_cursor().
////////////////////////////////////////////////////////////////////
void eglGraphicsPipe::
make_hidden_cursor() {
  nassertv(_hidden_cursor == None);

  unsigned int x_size, y_size;
  XQueryBestCursor(_display, _root, 1, 1, &x_size, &y_size);

  Pixmap empty = XCreatePixmap(_display, _root, x_size, y_size, 1);

  XColor black;
  memset(&black, 0, sizeof(black));

  _hidden_cursor = XCreatePixmapCursor(_display, empty, empty,
                                       &black, &black, x_size, y_size);
  XFreePixmap(_display, empty);
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsPipe::release_hidden_cursor
//       Access: Private
//  Description: Called once to release the invisible cursor created
//               by make_hidden_cursor().
////////////////////////////////////////////////////////////////////
void eglGraphicsPipe::
release_hidden_cursor() {
  if (_hidden_cursor != None) {
    XFreeCursor(_display, _hidden_cursor);
    _hidden_cursor = None;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsPipe::install_error_handlers
//       Access: Private, Static
//  Description: Installs new Xlib error handler functions if this is
//               the first time this function has been called.  These
//               error handler functions will attempt to reduce Xlib's
//               annoying tendency to shut down the client at the
//               first error.  Unfortunately, it is difficult to play
//               nice with the client if it has already installed its
//               own error handlers.
////////////////////////////////////////////////////////////////////
void eglGraphicsPipe::
install_error_handlers() {
  if (_error_handlers_installed) {
    return;
  }

  _prev_error_handler = (ErrorHandlerFunc *)XSetErrorHandler(error_handler);
  _prev_io_error_handler = (IOErrorHandlerFunc *)XSetIOErrorHandler(io_error_handler);
  _error_handlers_installed = true;
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsPipe::error_handler
//       Access: Private, Static
//  Description: This function is installed as the error handler for a
//               non-fatal Xlib error.
////////////////////////////////////////////////////////////////////
int eglGraphicsPipe::
error_handler(X11_Display *display, XErrorEvent *error) {
  static const int msg_len = 80;
  char msg[msg_len];
  XGetErrorText(display, error->error_code, msg, msg_len);
  egldisplay_cat.error()
    << msg << "\n";

  if (x_error_abort) {
    abort();
  }

  // We return to allow the application to continue running, unlike
  // the default X error handler which exits.
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsPipe::io_error_handler
//       Access: Private, Static
//  Description: This function is installed as the error handler for a
//               fatal Xlib error.
////////////////////////////////////////////////////////////////////
int eglGraphicsPipe::
io_error_handler(X11_Display *display) {
  egldisplay_cat.fatal()
    << "X fatal error on display " << (void *)display << "\n";

  // Unfortunately, we can't continue from this function, even if we
  // promise never to use X again.  We're supposed to terminate
  // without returning, and if we do return, the caller will exit
  // anyway.  Sigh.  Very poor design on X's part.
  return 0;
}
