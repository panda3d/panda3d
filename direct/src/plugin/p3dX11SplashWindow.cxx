// Filename: p3dX11SplashWindow.cxx
// Created by:  pro-rsoft (08Jul09)
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

#include "p3dX11SplashWindow.h"

#ifdef HAVE_X11

#include <time.h>

// Sleeps for a short time.
#define MILLISLEEP() \
  timespec ts; \
  ts.tv_sec = 0; ts.tv_nsec = 100000; \
  nanosleep(&ts, NULL);

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DX11SplashWindow::
P3DX11SplashWindow(P3DInstance *inst) : 
  P3DSplashWindow(inst)
{
  INIT_THREAD(_thread);
  _display = None;
  _window = None;
  _screen = 0;
  _width = 0;
  _height = 0;
  _graphics_context = None;
  _thread_running = false;
  _got_install = false;
  _image_filename_changed = false;
  _image_filename_temp = false;
  _install_label_changed = false;
  _install_progress = 0.0;

  INIT_LOCK(_install_lock);

  start_thread();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DX11SplashWindow::
~P3DX11SplashWindow() {
  stop_thread();

  DESTROY_LOCK(_install_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::set_image_filename
//       Access: Public, Virtual
//               displayed in the center of the splash window.  If
//               image_filename_temp is true, the file is immediately
//               deleted after it has been read.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
set_image_filename(const string &image_filename,
                   bool image_filename_temp) {
  ACQUIRE_LOCK(_install_lock);
  if (_image_filename != image_filename) {
    _image_filename = image_filename;
    _image_filename_temp = image_filename_temp;
    _image_filename_changed = true;
  }
  RELEASE_LOCK(_install_lock);

  MILLISLEEP();

  if (!_thread_running && _thread_continue) {
    // The user must have closed the window.  Let's shut down the
    // instance, too.
    _inst->request_stop();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::set_install_label
//       Access: Public, Virtual
//  Description: Specifies the text that is displayed above the
//               install progress bar.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
set_install_label(const string &install_label) {
  ACQUIRE_LOCK(_install_lock);
  if (_install_label != install_label) {
    _install_label = install_label;
    _install_label_changed = true;
  }
  RELEASE_LOCK(_install_lock);

  MILLISLEEP();

  if (!_thread_running && _thread_continue) {
    // The user must have closed the window.  Let's shut down the
    // instance, too.
    _inst->request_stop();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::set_install_progress
//       Access: Public, Virtual
//  Description: Moves the install progress bar from 0.0 to 1.0.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
set_install_progress(double install_progress) {
  _got_install = true;

  ACQUIRE_LOCK(_install_lock);
  _install_progress = install_progress;
  RELEASE_LOCK(_install_lock);

  MILLISLEEP();

  if (!_thread_running && _thread_continue) {
    // The user must have closed the window.  Let's shut down the
    // instance, too.
    _inst->request_stop();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::start_thread
//       Access: Private
//  Description: Spawns the sub-thread.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
start_thread() {
  _thread_continue = true;
  INIT_THREAD(_thread);
  SPAWN_THREAD(_thread, thread_run, this);
  if (_thread != 0) {
    _thread_running = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::stop_thread
//       Access: Private
//  Description: Terminates and joins the sub-thread.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
stop_thread() {
  _thread_continue = false;
  MILLISLEEP();

  JOIN_THREAD(_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::thread_run
//       Access: Private
//  Description: The sub-thread's main run method.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
thread_run() {
  make_window();
  setup_gc();

  XEvent event;
  XSelectInput(_display, _window, ExposureMask);
  
  bool override = true, have_event = false;
  string prev_label;
  double prev_progress;
  
  while (_thread_continue) {
    have_event = XCheckTypedWindowEvent(_display, _window, Expose, &event)
              || XCheckTypedWindowEvent(_display, _window, GraphicsExpose, &event);
    
    ACQUIRE_LOCK(_install_lock);
    double install_progress = _install_progress;
    
    if (have_event || _install_label != prev_label) {
      redraw(_install_label);
      override = false;
    }
    if (_install_progress != prev_progress) {
      XFillRectangle(_display, _window, _graphics_context, 12, _height - 18,
                                      install_progress * (_width - 24), 7);
    }
    prev_label = _install_label;
    
    RELEASE_LOCK(_install_lock);
    prev_progress = install_progress;
    MILLISLEEP();
  }

  close_window();
  _thread_running = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::redraw
//       Access: Private
//  Description: Redraws the window.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
redraw(string label) {
  if (_graphics_context == NULL) return;
  
  XClearWindow(_display, _window);
  XDrawString(_display, _window, _graphics_context, _width / 2 - label.size() * 3,
                                        _height - 30, label.c_str(), label.size());
  XDrawRectangle(_display, _window, _graphics_context, 10, _height - 20, _width - 20, 10);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::make_window
//       Access: Private
//  Description: Creates the window for displaying progress.  Runs
//               within the sub-thread.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
make_window() {
  int x = _wparams.get_win_x();
  int y = _wparams.get_win_y();
  
  _width = 320;
  _height = 240;
  if (_wparams.get_win_width() != 0 && _wparams.get_win_height() != 0) {
    _width = _wparams.get_win_width();
    _height = _wparams.get_win_height();
  }

  Window parent = 0;
  
  // Hum, if we use the display provided by the browser,
  // it causes a crash in some browsers when you make an Xlib
  // call with the plugin window minimized.
  // So I kept XOpenDisplay until we have a better workaround.
  
  //_display = (Display*) _wparams.get_parent_window()._xdisplay;
  //_own_display = false;
  //if (_display == 0) {
    _display = XOpenDisplay(NULL);
    _own_display = true;
  //}
  _screen = DefaultScreen(_display);

  if (_wparams.get_window_type() == P3D_WT_embedded) {
    // Create an embedded window.
    parent = _wparams.get_parent_window()._xwindow;
  } else {
    // Create a toplevel window.
    parent = XRootWindow(_display, _screen);
  }
  
  assert(_display != NULL);
  assert(parent != None);
  _window = XCreateSimpleWindow(_display, parent, x, y, _width, _height, 0, 0, -1);
  XMapWindow(_display, _window);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::setup_gc
//       Access: Private
//  Description: Sets up the graphics context for drawing the text.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
setup_gc() {
  if (_graphics_context != NULL) {
    return;
  }

  ACQUIRE_LOCK(_install_lock);
  string install_label = _install_label;
  double install_progress = _install_progress;
  _install_label_changed = false;
  RELEASE_LOCK(_install_lock);
  
  XFontStruct* fs = XLoadQueryFont(_display, "6x13");
  XGCValues gcval;
  gcval.font = fs->fid; 
  gcval.function = GXcopy;
  gcval.plane_mask = AllPlanes;
  gcval.foreground = BlackPixel(_display, _screen);
  gcval.background = WhitePixel(_display, _screen);
  _graphics_context = XCreateGC(_display, _window, 
    GCFont | GCFunction | GCPlaneMask | GCForeground | GCBackground, &gcval); 
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::close_window
//       Access: Private
//  Description: Closes the window created above.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
close_window() {
  if (_graphics_context != None) {
    XFreeGC(_display, _graphics_context);
  }
  
  if (_window != None) {
    XDestroyWindow(_display, _window);
    _window = None;
  }

  if (_display != None && _own_display) {
    XCloseDisplay(_display);
    _display = None;
  }
}

#endif  // HAVE_X11
