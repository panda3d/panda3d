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

#include "get_tinyxml.h"
#include "binaryXml.h"
#include <time.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>

// Sleeps for a short time.
#define MILLISLEEP() \
  timespec ts; \
  ts.tv_sec = 0; ts.tv_nsec = 100000; \
  nanosleep(&ts, NULL);

// Clamps a value to two boundaries.
#define clamp(x, lb, hb) (x < lb ? lb : (x > hb ? hb : x))

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DX11SplashWindow::
P3DX11SplashWindow(P3DInstance *inst) : 
  P3DSplashWindow(inst)
{
  // Init for parent process
  _subprocess_pid = -1;

  // Init for subprocess
  _display = None;
  _window = None;
  _screen = 0;
  _graphics_context = None;
  _bar_context = None;
  _install_progress = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DX11SplashWindow::
~P3DX11SplashWindow() {
  stop_subprocess();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::set_wparams
//       Access: Public, Virtual
//  Description: Changes the window parameters, e.g. to resize or
//               reposition the window; or sets the parameters for the
//               first time, creating the initial window.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
set_wparams(const P3DWindowParams &wparams) {
  P3DSplashWindow::set_wparams(wparams);

  if (_subprocess_pid == -1) {
    start_subprocess();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::set_image_filename
//       Access: Public, Virtual
//  Description: Specifies the name of a JPEG image file that is
//               displayed in the center of the splash window.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
set_image_filename(const string &image_filename, ImagePlacement image_placement) {
  if (_subprocess_pid == -1) {
    return;
  }

  TiXmlDocument doc;
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "set_image_filename");
  xcommand->SetAttribute("image_filename", image_filename);
  xcommand->SetAttribute("image_placement", (int)image_placement);
  doc.LinkEndChild(xcommand);
  write_xml(_pipe_write, &doc, nout);

  check_stopped();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::set_install_label
//       Access: Public, Virtual
//  Description: Specifies the text that is displayed above the
//               install progress bar.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
set_install_label(const string &install_label) {
  if (_subprocess_pid == -1) {
    return;
  }

  TiXmlDocument doc;
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "set_install_label");
  xcommand->SetAttribute("install_label", install_label);
  doc.LinkEndChild(xcommand);
  write_xml(_pipe_write, &doc, nout);

  check_stopped();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::set_install_progress
//       Access: Public, Virtual
//  Description: Moves the install progress bar from 0.0 to 1.0.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
set_install_progress(double install_progress) {
  if (_subprocess_pid == -1) {
    return;
  }

  TiXmlDocument doc;
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "set_install_progress");
  xcommand->SetDoubleAttribute("install_progress", install_progress);
  doc.LinkEndChild(xcommand);
  write_xml(_pipe_write, &doc, nout);

  check_stopped();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::set_button_active
//       Access: Public, Virtual
//  Description: Sets whether the button should be visible and active
//               (true) or invisible and inactive (false).  If active,
//               the button image will be displayed in the window, and
//               a click event will be generated when the user clicks
//               the button.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
set_button_active(bool flag) {
  if (_subprocess_pid == -1) {
    return;
  }

  TiXmlDocument doc;
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "set_button_active");
  xcommand->SetAttribute("button_active", (int)flag);
  doc.LinkEndChild(xcommand);
  write_xml(_pipe_write, &doc, nout);

  check_stopped();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::button_click_detected
//       Access: Protected, Virtual
//  Description: Called when a button click by the user is detected in
//               set_mouse_data(), this method simply turns around and
//               notifies the instance.  It's a virtual method to give
//               subclasses a chance to redirect this message to the
//               main thread or process, as necessary.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
button_click_detected() {
  // This method is called in the child process, and must relay
  // the information to the parent process.
  cerr << "click detected\n";
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::start_subprocess
//       Access: Private
//  Description: Spawns the subprocess that runs the window.  We have
//               to use a subprocess instead of just a sub-thread, to
//               protect X11 against mutual access.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
start_subprocess() {
  assert(_subprocess_pid == -1);

  // Create a directional pipe to send messages to the sub-process.
  // (We don't need a receive pipe.)
  int to_fd[2];
  if (pipe(to_fd) < 0) {
    perror("failed to create pipe");
  }

  // Fork and exec.
  pid_t child = fork();
  if (child < 0) {
    close(to_fd[0]);
    close(to_fd[1]);
    perror("fork");
    return;
  }

  if (child == 0) {
    // Here we are in the child process.

    // Open the read end of the pipe, and close the write end.
    _pipe_read.open_read(to_fd[0]);
    close(to_fd[1]);

    subprocess_run();
    _exit(0);
  }

  // In the parent process.
  _subprocess_pid = child;
  _pipe_write.open_write(to_fd[1]);
  close(to_fd[0]);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::stop_subprocess
//       Access: Private
//  Description: Terminates the subprocess.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
stop_subprocess() {
  if (_subprocess_pid == -1) {
    // Already stopped.
    return;
  }

  // Ask the subprocess to stop.
  TiXmlDocument doc;
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "exit");
  doc.LinkEndChild(xcommand);
  write_xml(_pipe_write, &doc, nout);

  // Also close the pipe, to help underscore the point.
  _pipe_write.close();

  static const int max_wait_ms = 2000;
  
  // Wait for a certain amount of time for the process to stop by
  // itself.
  struct timeval start;
  gettimeofday(&start, NULL);
  int start_ms = start.tv_sec * 1000 + start.tv_usec / 1000;
  
  int status;
  pid_t result = waitpid(_subprocess_pid, &status, WNOHANG);
  while (result != _subprocess_pid) {
    if (result == -1) {
      perror("waitpid");
      break;
    }
    
    struct timeval now;
    gettimeofday(&now, NULL);
    int now_ms = now.tv_sec * 1000 + now.tv_usec / 1000;
    int elapsed = now_ms - start_ms;
    
    if (elapsed > max_wait_ms) {
      // Tired of waiting.  Kill the process.
      nout << "Force-killing splash window process, pid " << _subprocess_pid 
           << "\n";
      kill(_subprocess_pid, SIGKILL);
      start_ms = now_ms;
    }
    
    // Yield the timeslice and wait some more.
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1;
    select(0, NULL, NULL, NULL, &tv);
    result = waitpid(_subprocess_pid, &status, WNOHANG);
  }

  nout << "Splash window process has successfully stopped.\n";
  if (WIFEXITED(status)) {
    nout << "  exited normally, status = "
         << WEXITSTATUS(status) << "\n";
  } else if (WIFSIGNALED(status)) {
    nout << "  signalled by " << WTERMSIG(status) << ", core = " 
         << WCOREDUMP(status) << "\n";
  } else if (WIFSTOPPED(status)) {
    nout << "  stopped by " << WSTOPSIG(status) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::check_stopped
//       Access: Private
//  Description: Shuts down the instance if the window is closed
//               prematurely (for instance, due to user action).
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
check_stopped() {
  if (_subprocess_pid == -1) {
    // Already stopped.
    return;
  }

  int status;
  int result = waitpid(_subprocess_pid, &status, WNOHANG);
  if (result == 0) {
    // Process is still running.
    return;
  }

  if (result == -1) {
    // Error in waitpid.
    perror("waitpid");
    return;
  }

  // Process has stopped.
  assert(result == _subprocess_pid);

  nout << "Splash window process has stopped unexpectedly.\n";
  if (WIFEXITED(status)) {
    nout << "  exited normally, status = "
         << WEXITSTATUS(status) << "\n";
  } else if (WIFSIGNALED(status)) {
    nout << "  signalled by " << WTERMSIG(status) << ", core = " 
         << WCOREDUMP(status) << "\n";
  } else if (WIFSTOPPED(status)) {
    nout << "  stopped by " << WSTOPSIG(status) << "\n";
  }

  _subprocess_pid = -1;
  _inst->request_stop();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::subprocess_run
//       Access: Private
//  Description: The subprocess's main run method.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
subprocess_run() {
  // Since we're now isolated in a subprocess, we can safely make all
  // the X calls we like, and run independently of the browser
  // process.

  make_window();
  setup_gc();
  if (_graphics_context == None) {
    // No point in continuing if we couldn't get a graphics context.
    close_window();
    return;
  }

  ButtonState prev_bstate = BS_hidden;
  string prev_label;
  double prev_progress = 0.0;

  bool needs_redraw = true;
  bool needs_draw_label = false;
  bool needs_redraw_progress = false;
  bool needs_update_progress = false;

  _subprocess_continue = true;
  while (_subprocess_continue) {
    // First, scan for X events.
    XEvent event;
    while (XCheckWindowEvent(_display, _window, ~0, &event)) {
      switch (event.type) {
      case Expose:
      case GraphicsExpose:
        needs_redraw = true;
        break;

      case ConfigureNotify:
        if (event.xconfigure.width != _win_width ||
            event.xconfigure.height != _win_height) {
          _win_width = event.xconfigure.width;
          _win_height = event.xconfigure.height;
          
          // If the window changes size, we need to recompute all of the
          // resized images.
          _background_image.dump_resized_image();
          _button_ready_image.dump_resized_image();
          _button_rollover_image.dump_resized_image();
          _button_click_image.dump_resized_image();
        }
        needs_redraw = true;
        break;
        
      case MotionNotify:
        set_mouse_data(event.xmotion.x, event.xmotion.y, _mouse_down);
        break;

      case ButtonPress:
        set_mouse_data(_mouse_x, _mouse_y, true);
        break;
        
      case ButtonRelease:
        set_mouse_data(_mouse_x, _mouse_y, false);
        break;
      }
    }

    update_image(_background_image, needs_redraw);
    update_image(_button_ready_image, needs_redraw);
    update_image(_button_rollover_image, needs_redraw);
    update_image(_button_click_image, needs_redraw);

    if (_bstate != prev_bstate) {
      needs_redraw = true;
      prev_bstate = _bstate;
    }

    if (_install_label != prev_label) {
      needs_redraw = true;
      prev_label = _install_label;
    }
    
    if (_install_progress != prev_progress) {
      needs_update_progress = true;
      prev_progress = _install_progress;

      if (_install_progress == 0.0) {
        // If the progress bar drops to zero, repaint the screen to
        // take the progress bar away.
        needs_redraw = true;
      }
    }
    
    if (needs_redraw) {
      redraw();
      XFlush(_display);

      needs_redraw = false;
      needs_draw_label = true;
      needs_redraw_progress = true;
    }

    // Don't draw an install label or a progress bar unless we have
    // some nonzero progress.
    if (_install_progress != 0.0) {
      int bar_x, bar_y, bar_width, bar_height;
      get_bar_placement(bar_x, bar_y, bar_width, bar_height);

      if (needs_draw_label) {
        int text_width = _install_label.size() * 6;
        int text_height = 10;
        int text_x = (_win_width - text_width) / 2;
        int text_y = bar_y - 4;

        XClearArea(_display, _window,
                   text_x - 2, text_y - text_height - 2,
                   text_width + 4, text_height + 4, false);
        XDrawString(_display, _window, _graphics_context, text_x, text_y,
                    _install_label.c_str(), _install_label.size());
        
        needs_draw_label = false;
      }
      
      if (needs_redraw_progress) {
        XClearArea(_display, _window, 
                   bar_x, bar_y, bar_width, bar_height, false);
        XDrawRectangle(_display, _window, _graphics_context, 
                       bar_x, bar_y, bar_width, bar_height);
        needs_update_progress = true;
        needs_redraw_progress = false;
      }
      
      if (needs_update_progress) {
        int progress_width = (int)((bar_width - 2) * _install_progress);
        XFillRectangle(_display, _window, _bar_context, 
                       bar_x + 1, bar_y + 1,
                       progress_width + 1, bar_height - 1);
        needs_update_progress = false;
      }
    }


    // Now check for input from the parent.
    bool input_ready = _pipe_read.has_gdata();
    if (!input_ready) {
      int read_fd = _pipe_read.get_handle();
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(read_fd, &fds);
      
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 1;  // Sleep a bit to yield the timeslice if there's
      // nothing new.
      
      int result = select(read_fd + 1, &fds, NULL, NULL, &tv);
      if (result > 0) {
        // There is some noise on the pipe, so read it.
        input_ready = true;
      } else if (result == -1) {
        // Error in select.
        perror("select");
      }
    }

    if (input_ready) {
      receive_command();
    }
  }

  close_window();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::receive_command
//       Access: Private
//  Description: Receives a command from the parent.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
receive_command() {
  TiXmlDocument *doc = read_xml(_pipe_read, nout);
  if (doc == NULL) {
    // Pipe closed or something.
    _subprocess_continue = false;
    return;
  }

  TiXmlElement *xcommand = doc->FirstChildElement("command");
  if (xcommand != NULL) {
    const char *cmd = xcommand->Attribute("cmd");
    if (cmd != NULL) {
      if (strcmp(cmd, "exit") == 0) {
        _subprocess_continue = false;

      } else if (strcmp(cmd, "set_image_filename") == 0) {
        const string *image_filename = xcommand->Attribute(string("image_filename"));
        int image_placement;
        if (image_filename != NULL && 
            xcommand->QueryIntAttribute("image_placement", &image_placement) == TIXML_SUCCESS) {
          
          X11ImageData *image = NULL;
          switch ((ImagePlacement)image_placement) {
          case IP_background:
            image = &_background_image;
            break;
            
          case IP_button_ready:
            image = &_button_ready_image;
            set_button_range(_button_ready_image);
            break;
            
          case IP_button_rollover:
            image = &_button_rollover_image;
            break;
            
          case IP_button_click:
            image = &_button_click_image;
            break;
          }
          if (image != NULL) {
            if (image->_filename != *image_filename) {
              image->_filename = *image_filename;
              image->_filename_changed = true;
            }
          }
        }

      } else if (strcmp(cmd, "set_install_label") == 0) {
        const char *str = xcommand->Attribute("install_label");
        if (str != NULL) {
          if (_install_label != string(str)) {
            _install_label = str;
          }
        }

      } else if (strcmp(cmd, "set_install_progress") == 0) {
        double install_progress = 0.0;
        xcommand->Attribute("install_progress", &install_progress);

        _install_progress = install_progress;

      } else if (strcmp(cmd, "set_button_active") == 0) {
        int button_active = 0;
        xcommand->Attribute("button_active", &button_active);

        P3DSplashWindow::set_button_active(button_active != 0);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::redraw
//       Access: Private
//  Description: Redraws the window.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
redraw() {
  XClearWindow(_display, _window);

  paint_image(_background_image);

  switch (_bstate) {
  case BS_hidden:
    break;
  case BS_ready:
    paint_image(_button_ready_image);
    break;
  case BS_rollover:
    if (!paint_image(_button_rollover_image)) {
      paint_image(_button_ready_image);
    }
    break;
  case BS_click:
    if (!paint_image(_button_click_image)) {
      paint_image(_button_ready_image);
    }
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::paint_image
//       Access: Private
//  Description: Draws the indicated image, centered within the
//               window.  Returns true on success, false if the image
//               is not defined.
////////////////////////////////////////////////////////////////////
bool P3DX11SplashWindow::
paint_image(X11ImageData &image) {
  if (image._image == NULL) {
    return false;
  }

  // We have an image. Let's see how we can output it.
  if (image._width <= _win_width && image._height <= _win_height) {
    // It fits within the window - just draw it.
    XPutImage(_display, _window, _graphics_context, image._image, 0, 0, 
              (_win_width - image._width) / 2, (_win_height - image._height) / 2,
              image._width, image._height);
  } else if (image._resized_image != NULL) {
    // We have a resized image already, draw that one.
    XPutImage(_display, _window, _graphics_context, image._resized_image, 0, 0, 
              (_win_width - image._resized_width) / 2, (_win_height - image._resized_height) / 2,
              image._resized_width, image._resized_height);
  } else {
    // Yuck, the bad case - we need to scale it down.
    double scale = min((double) _win_width  / (double) image._width,
                       (double) _win_height / (double) image._height);
    image._resized_width = (int)(image._width * scale);
    image._resized_height = (int)(image._height * scale);
    char *new_data = (char*) malloc(4 * _win_width * _win_height);
    image._resized_image = XCreateImage(_display, CopyFromParent, DefaultDepth(_display, _screen), 
                                  ZPixmap, 0, (char *) new_data, _win_width, _win_height, 32, 0);
    double x_ratio = ((double) image._width) / ((double) image._resized_width);
    double y_ratio = ((double) image._height) / ((double) image._resized_height);
    for (int x = 0; x < _win_width; ++x) {
      for (int y = 0; y < _win_height; ++y) {
        XPutPixel(image._resized_image, x, y, 
                  XGetPixel(image._image,
                            (int)clamp(x * x_ratio, 0, image._width),
                            (int)clamp(y * y_ratio, 0, image._height)));
      }
    }
    XPutImage(_display, _window, _graphics_context, image._resized_image, 0, 0,
              (_win_width - image._resized_width) / 2, (_win_height - image._resized_height) / 2,
              image._resized_width, image._resized_height);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::make_window
//       Access: Private
//  Description: Creates the window for displaying progress.  Runs
//               within the sub-process.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
make_window() {
  int x = _wparams.get_win_x();
  int y = _wparams.get_win_y();
  
  _win_width = 320;
  _win_height = 240;
  if (_wparams.get_win_width() != 0 && _wparams.get_win_height() != 0) {
    _win_width = _wparams.get_win_width();
    _win_height = _wparams.get_win_height();
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

  int depth = DefaultDepth(_display, _screen);
  Visual *dvisual = DefaultVisual(_display, _screen);

  long event_mask =
    ButtonPressMask | ButtonReleaseMask |
    PointerMotionMask | StructureNotifyMask;

  // Initialize window attributes
  XSetWindowAttributes wa;
  wa.background_pixel = XWhitePixel(_display, _screen);
  wa.border_pixel = 0;
  wa.event_mask = event_mask;

  unsigned long attrib_mask = CWBackPixel | CWBorderPixel | CWEventMask;

  _window = XCreateWindow
    (_display, parent, x, y, _win_width, _win_height,
     0, depth, InputOutput, dvisual, attrib_mask, &wa);

  XMapWindow(_display, _window);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::setup_gc
//       Access: Private
//  Description: Sets up the graphics context for drawing the text.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
setup_gc() {
  if (_graphics_context != None) {
    return;
  }

  XFontStruct* fs = XLoadQueryFont(_display, "6x13");

  XGCValues gcval;
  gcval.font = fs->fid; 
  gcval.function = GXcopy;
  gcval.plane_mask = AllPlanes;
  gcval.foreground = BlackPixel(_display, _screen);
  gcval.background = WhitePixel(_display, _screen);
  _graphics_context = XCreateGC(_display, _window, 
    GCFont | GCFunction | GCPlaneMask | GCForeground | GCBackground, &gcval); 

  // Also create a gc for filling in the interior of the progress bar
  // in a pleasant blue color.
  XColor blue;
  blue.red = 27756;
  blue.green = 42405;
  blue.blue = 57568;
  blue.flags = DoRed | DoGreen | DoBlue;

  Colormap colormap = DefaultColormap(_display, _screen);
  if (XAllocColor(_display, colormap, &blue)) {
    _blue_pixel = blue.pixel;
    gcval.foreground = blue.pixel;
  }

  _bar_context = XCreateGC(_display, _window, 
    GCFont | GCFunction | GCPlaneMask | GCForeground | GCBackground, &gcval);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::close_window
//       Access: Private
//  Description: Closes the window created above.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
close_window() {
  _background_image.dump_image();
  _button_ready_image.dump_image();
  _button_rollover_image.dump_image();
  _button_click_image.dump_image();
  
  if (_bar_context != None) {
    if (_bar_context != _graphics_context) {
      XFreeGC(_display, _bar_context);
    }
    _bar_context = None;

    // Also free the color we allocated.
    Colormap colormap = DefaultColormap(_display, _screen);
    XFreeColors(_display, colormap, &_blue_pixel, 1, 0);
  }
  
  if (_graphics_context != None) {
    XFreeGC(_display, _graphics_context);
    _graphics_context = None;
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

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::update_image
//       Access: Private
//  Description: Loads the splash image, converts to to an XImage,
//               and stores it in _image.  Runs only in the
//               child process.
//
//               If the image is changed, sets needs_redraw to true.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
update_image(X11ImageData &image, bool &needs_redraw) {
  if (!image._filename_changed) {
    // No changes.
    return;
  }
  image._filename_changed = false;
  needs_redraw = true;

  // Clear the old image.
  image.dump_image();

  // Go read the image.
  string data;
  if (!read_image_data(image, data, image._filename)) {
    return;
  }

  Visual *dvisual = DefaultVisual(_display, _screen);
  double r_ratio = dvisual->red_mask / 255.0;
  double g_ratio = dvisual->green_mask / 255.0;
  double b_ratio = dvisual->blue_mask / 255.0;
  uint32_t *new_data = (uint32_t*)malloc(4 * image._width * image._height);

  if (image._num_channels == 4) {
    int j = 0;
    for (int i = 0; i < 4 * image._width * image._height; i += 4) {
      unsigned int r, g, b;
      r = (unsigned int)(data[i+0] * r_ratio);
      g = (unsigned int)(data[i+1] * g_ratio);
      b = (unsigned int)(data[i+2] * b_ratio);
      new_data[j++] = (r & dvisual->red_mask) |
                      (g & dvisual->green_mask) |
                      (b & dvisual->blue_mask);
    }
  } else if (image._num_channels == 3) {
    int j = 0;
    for (int i = 0; i < 3 * image._width * image._height; i += 3) {
      unsigned int r, g, b;
      r = (unsigned int)(data[i+0] * r_ratio);
      g = (unsigned int)(data[i+1] * g_ratio);
      b = (unsigned int)(data[i+2] * b_ratio);
      new_data[j++] = (r & dvisual->red_mask) |
                      (g & dvisual->green_mask) |
                      (b & dvisual->blue_mask);
    }
  } else if (image._num_channels == 1) {
    // A grayscale image.  Replicate out the channels.
    for (int i = 0; i < image._width * image._height; ++i) {
      unsigned int r, g, b;
      r = (unsigned int)(data[i] * r_ratio);
      g = (unsigned int)(data[i] * g_ratio);
      b = (unsigned int)(data[i] * b_ratio);
      new_data[i] = (r & dvisual->red_mask) |
                    (g & dvisual->green_mask) |
                    (b & dvisual->blue_mask);
    }
  }

  // Now load the image.
  image._image = XCreateImage(_display, CopyFromParent, DefaultDepth(_display, _screen), 
                              ZPixmap, 0, (char *)new_data, image._width, image._height, 32, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::X11ImageData::dump_image
//       Access: Public
//  Description: Frees the previous image data.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::X11ImageData::
dump_image() {
  if (_image != NULL) {
    XDestroyImage(_image);
    _image = NULL;
  }
  if (_resized_image != NULL) {
    XDestroyImage(_resized_image);
    _resized_image = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::X11ImageData::dump_resized_image
//       Access: Public
//  Description: Frees the previous resized image data only, retaining
//               the original image data.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::X11ImageData::
dump_resized_image() {
  if (_resized_image != NULL) {
    XDestroyImage(_resized_image);
    _resized_image = NULL;
  }
}

#endif  // HAVE_X11
