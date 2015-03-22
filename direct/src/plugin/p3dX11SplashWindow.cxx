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
#include <stdint.h>

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
P3DX11SplashWindow::
P3DX11SplashWindow(P3DInstance *inst, bool make_visible) :
  P3DSplashWindow(inst, make_visible)
{
  // Init for parent process
  _subprocess_pid = -1;

  // Init for read thread
  _started_read_thread = false;
  INIT_THREAD(_read_thread);

  // Init for subprocess
  _composite_image = NULL;
  _needs_new_composite = false;
  _display = None;
  _window = None;
  _screen = 0;
  _graphics_context = None;
  _bar_context = None;
  _fg_pixel = -1;
  _bg_pixel = -1;
  _bar_pixel = -1;
  _own_display = false;
  _install_progress = 0.0;
  _progress_known = true;
  _received_data = 0;
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
//     Function: P3DX11SplashWindow::set_visible
//       Access: Public, Virtual
//  Description: Makes the splash window visible or invisible, so as
//               not to compete with the embedded Panda window in the
//               same space.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
set_visible(bool visible) {
  P3DSplashWindow::set_visible(visible);

  TiXmlDocument doc;
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "set_visible");
  xcommand->SetAttribute("visible", (int)_visible);
  doc.LinkEndChild(xcommand);
  write_xml(_pipe_write, &doc, nout);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::set_image_filename
//       Access: Public, Virtual
//  Description: Specifies the name of a JPEG image file that is
//               displayed in the center of the splash window.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
set_image_filename(const string &image_filename, ImagePlacement image_placement) {
  nout << "image_filename = " << image_filename << "\n";
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
set_install_progress(double install_progress,
                     bool is_progress_known, size_t received_data) {
  if (_subprocess_pid == -1) {
    return;
  }

  TiXmlDocument doc;
  TiXmlElement *xcommand = new TiXmlElement("command");
  xcommand->SetAttribute("cmd", "set_install_progress");
  xcommand->SetDoubleAttribute("install_progress", install_progress);
  xcommand->SetAttribute("progress_known", (int)is_progress_known);
  xcommand->SetAttribute("received_data", (int)received_data);
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
  TiXmlDocument doc;
  TiXmlElement *xcommand = new TiXmlElement("click");
  doc.LinkEndChild(xcommand);
  write_xml(_pipe_write, &doc, nout);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::set_bstate
//       Access: Protected, Virtual
//  Description: Changes the button state as the mouse interacts with
//               it.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
set_bstate(ButtonState bstate) {
  if (_bstate != bstate) {
    // When the button state changes, we need to remake the composite
    // image.
    _needs_new_composite = true;
    P3DSplashWindow::set_bstate(bstate);
  }
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

  // Create a bi-directional pipe to communicate with the sub-process.
  int to_fd[2];
  if (pipe(to_fd) < 0) {
    perror("failed to create pipe");
  }
  int from_fd[2];
  if (pipe(from_fd) < 0) {
    perror("failed to create pipe");
  }

  // Fork and exec.
  pid_t child = fork();
  if (child < 0) {
    close(to_fd[0]);
    close(to_fd[1]);
    close(from_fd[0]);
    close(from_fd[1]);
    perror("fork");
    return;
  }

  if (child == 0) {
    // Here we are in the child process.
    init_xml();

    // Open the read end of the pipe, and close the write end.
    _pipe_read.open_read(to_fd[0]);
    close(to_fd[1]);
    _pipe_write.open_write(from_fd[1]);
    close(from_fd[0]);

    subprocess_run();
    _exit(0);
  }

  // In the parent process.
  _subprocess_pid = child;
  _pipe_write.open_write(to_fd[1]);
  close(to_fd[0]);
  _pipe_read.open_read(from_fd[0]);
  close(from_fd[1]);

  spawn_read_thread();
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

  join_read_thread();
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
  join_read_thread();

  _inst->request_stop_main_thread();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::spawn_read_thread
//       Access: Private
//  Description: Starts the read thread.  We need this thread to
//               listen for feedback from the subprocess.  (At the
//               moment, the only kind of feedback we might receive is
//               whether the button has been clicked.)
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
spawn_read_thread() {
  SPAWN_THREAD(_read_thread, rt_thread_run, this);
  _started_read_thread = true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::join_read_thread
//       Access: Private
//  Description: Waits for the read thread to stop.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
join_read_thread() {
  if (!_started_read_thread) {
    return;
  }

  JOIN_THREAD(_read_thread);
  _started_read_thread = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::rt_thread_run
//       Access: Private
//  Description: The main function for the read thread.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
rt_thread_run() {
  while (true) {
    TiXmlDocument *doc = read_xml(_pipe_read, nout);
    if (doc == NULL) {
      // Some error on reading.  The splash window must have gone
      // away, e.g. because the user explicitly closed it; tell the
      // instance to exit.
      _inst->request_stop_sub_thread();
      return;
    }

    // Successfully read an XML document.
    rt_handle_request(doc);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::rt_handle_request
//       Access: Private
//  Description: Processes a single request or notification received
//               from an instance.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
rt_handle_request(TiXmlDocument *doc) {
  // Eh, don't even bother decoding the XML.  We know it can only be a
  // click notification.
  delete doc;

  P3DSplashWindow::button_click_detected();
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
  bool prev_progress_known = true;
  size_t prev_received_data = 0;

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

          set_button_range(_button_ready_image);

          // If the window changes size, we need to recompute the
          // composed image.
          _needs_new_composite = true;
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

    update_image(_background_image);
    update_image(_button_ready_image);
    update_image(_button_rollover_image);
    update_image(_button_click_image);

    if (_needs_new_composite) {
      needs_redraw = true;
      compose_image();
    }

    if (_bstate != prev_bstate) {
      needs_redraw = true;
      prev_bstate = _bstate;
    }

    if (_install_label != prev_label) {
      needs_redraw = true;
      prev_label = _install_label;
    }

    if (_progress_known != prev_progress_known) {
      needs_update_progress = true;
      needs_redraw_progress = true;
    } else if (_progress_known) {
      if (_install_progress != prev_progress) {
        needs_update_progress = true;
        if (_install_progress < prev_progress) {
          needs_redraw_progress = true;
        }
      }
    } else {
      if (_received_data != prev_received_data) {
        needs_update_progress = true;
        needs_redraw_progress = true;
      }
    }

    if (needs_update_progress) {
      prev_progress = _install_progress;
      prev_progress_known = _progress_known;
      prev_received_data = _received_data;

      if (_progress_known && _install_progress == 0.0) {
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
    if (!_progress_known || _install_progress != 0.0) {
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
        if (_progress_known) {
          int progress_width = (int)((bar_width - 2) * _install_progress + 0.5);
          XFillRectangle(_display, _window, _bar_context,
                         bar_x + 1, bar_y + 1,
                         progress_width + 1, bar_height - 1);
        } else {
          // Progress is unknown.  Draw a moving block, not a progress bar
          // filling up.
          int block_width = (int)(bar_width * 0.1 + 0.5);
          int block_travel = (bar_width - 2) - block_width;
          int progress = (int)(_received_data * _unknown_progress_rate);
          progress = progress % (block_travel * 2);
          if (progress > block_travel) {
            progress = block_travel * 2 - progress;
          }

          XFillRectangle(_display, _window, _bar_context,
                         bar_x + 1 + progress, bar_y + 1,
                         block_width + 1, bar_height - 1);
        }
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

      // Sleep a bit to yield the timeslice if there's nothing new.
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 1000;   // 1 usec is not enough.

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

    struct timespec req;
    req.tv_sec = 0;
    req.tv_nsec = 50000000;  // 50 ms
    nanosleep(&req, NULL);
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

      } else if (strcmp(cmd, "set_visible") == 0) {
        int visible = 0;
        if (xcommand->Attribute("visible", &visible) != NULL) {
          _visible = visible;
          if (_visible) {
            XMapWindow(_display, _window);
          } else {
            XUnmapWindow(_display, _window);
          }
        }

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

          case IP_none:
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
        int progress_known = 1;
        int received_data = 0;
        xcommand->Attribute("install_progress", &install_progress);
        xcommand->Attribute("progress_known", &progress_known);
        xcommand->Attribute("received_data", &received_data);

        _install_progress = install_progress;
        _progress_known = (progress_known != 0);
        _received_data = (size_t)received_data;

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
  if (_composite_image == NULL) {
    // Clear the whole window, if there's no image.
    XClearWindow(_display, _window);

  } else {
    // If we have an image, draw it.
    int xo = (_win_width - _composite_width) / 2;
    int yo = (_win_height - _composite_height) / 2;
    XPutImage(_display, _window, _graphics_context, _composite_image, 0, 0,
              xo, yo, _composite_width, _composite_height);

    // Then clear the rectangles around it carefully (rather than just
    // clearing the whole window first, to avoid flicking).
    if (yo != 0 && _win_width != 0) {
      // Top
      XClearArea(_display, _window, 0, 0, _win_width, yo, False);
      // Bottom
      XClearArea(_display, _window, 0, _win_height - yo, _win_width, yo, False);
    }
    if (xo != 0 && _composite_height != 0) {
      // Left
      XClearArea(_display, _window, 0, yo, xo, _composite_height, False);
      // Right
      XClearArea(_display, _window, _win_width - xo, yo, xo, _composite_height, False);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::make_window
//       Access: Private
//  Description: Creates the window for displaying progress.  Runs
//               within the sub-process.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
make_window() {
  _win_width = 320;
  _win_height = 240;
  if (_wparams.get_win_width() != 0 && _wparams.get_win_height() != 0) {
    _win_width = _wparams.get_win_width();
    _win_height = _wparams.get_win_height();
  }

  X11_Window parent = 0;

  // Hum, if we use the display provided by the browser,
  // it causes a crash in some browsers when you make an Xlib
  // call with the plugin window minimized.
  // So I kept XOpenDisplay until we have a better workaround.

  //_display = (X11_Display*) _wparams.get_parent_window()._xdisplay;
  //_own_display = false;
  //if (_display == 0) {
    _display = XOpenDisplay(NULL);
    _own_display = true;
  //}
  assert(_display != NULL);
  _screen = DefaultScreen(_display);

  int x = _wparams.get_win_x();
  int y = _wparams.get_win_y();
  if (x == -1) x = 0;
  if (y == -1) y = 0;
  if (x == -2) x = (int)(0.5 * (DisplayWidth(_display, _screen) - _win_width));
  if (y == -2) y = (int)(0.5 * (DisplayHeight(_display, _screen) - _win_height));

  if (_wparams.get_window_type() == P3D_WT_embedded) {
    // Create an embedded window.
    const P3D_window_handle &handle = _wparams.get_parent_window();
    assert(handle._window_handle_type == P3D_WHT_x11_window);
    parent = handle._handle._x11_window._xwindow;
  } else {
    // Create a toplevel window.
    parent = XRootWindow(_display, _screen);
  }

  assert(parent != None);

  int depth = DefaultDepth(_display, _screen);
  Visual *dvisual = DefaultVisual(_display, _screen);

  long event_mask =
    ButtonPressMask | ButtonReleaseMask |
    PointerMotionMask | StructureNotifyMask | ExposureMask;

  // Allocate the foreground and background colors.
  Colormap colormap = DefaultColormap(_display, _screen);

  XColor fg;
  fg.red = _fgcolor_r * 0x101;
  fg.green = _fgcolor_g * 0x101;
  fg.blue = _fgcolor_b * 0x101;
  fg.flags = DoRed | DoGreen | DoBlue;
  _fg_pixel = -1;
  if (XAllocColor(_display, colormap, &fg)) {
    _fg_pixel = fg.pixel;
  }

  XColor bg;
  bg.red = _bgcolor_r * 0x101;
  bg.green = _bgcolor_g * 0x101;
  bg.blue = _bgcolor_b * 0x101;
  bg.flags = DoRed | DoGreen | DoBlue;
  _bg_pixel = -1;
  if (XAllocColor(_display, colormap, &bg)) {
    _bg_pixel = bg.pixel;
  }

  // Initialize window attributes
  XSetWindowAttributes wa;
  wa.background_pixel = XWhitePixel(_display, _screen);
  if (_bg_pixel != -1) {
    wa.background_pixel = _bg_pixel;
  }
  wa.border_pixel = 0;
  wa.event_mask = event_mask;

  unsigned long attrib_mask = CWBackPixel | CWBorderPixel | CWEventMask;

  _window = XCreateWindow
    (_display, parent, x, y, _win_width, _win_height,
     0, depth, InputOutput, dvisual, attrib_mask, &wa);

  // Now hint the window manager about the window origin and size.
  // This is necessary because window managers are free to ignore
  // the window origin specified in the XCreateWindow call.
  XSizeHints *size_hints_p = XAllocSizeHints();
  if (_wparams.get_win_x() != -1 || _wparams.get_win_y() != -1) {
    // If the user requested (-1, -1), the default position, we let
    // the window manager choose a position by omitting the pos hint.
    size_hints_p->x = x;
    size_hints_p->y = y;
    size_hints_p->flags |= USPosition;
  }
  size_hints_p->width = _win_width;
  size_hints_p->height = _win_height;
  size_hints_p->flags |= USSize;
  XSetWMNormalHints(_display, _window, size_hints_p);
  XFree(size_hints_p);

  if (_visible) {
    XMapWindow(_display, _window);
  }
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
  if (_fg_pixel != -1) {
    gcval.foreground = _fg_pixel;
  }
  gcval.background = WhitePixel(_display, _screen);
  if (_bg_pixel != -1) {
    gcval.background = _bg_pixel;
  }
  _graphics_context = XCreateGC(_display, _window,
    GCFont | GCFunction | GCPlaneMask | GCForeground | GCBackground, &gcval);

  // Also create a gc for filling in the interior of the progress bar
  // in a pleasant blue color (or whatever color the user requested).
  XColor bar;
  bar.red = _barcolor_r * 0x101;
  bar.green = _barcolor_g * 0x101;
  bar.blue = _barcolor_b * 0x101;
  bar.flags = DoRed | DoGreen | DoBlue;

  Colormap colormap = DefaultColormap(_display, _screen);
  if (XAllocColor(_display, colormap, &bar)) {
    _bar_pixel = bar.pixel;
    gcval.foreground = bar.pixel;
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
  if (_composite_image != NULL) {
    XDestroyImage(_composite_image);
    _composite_image = NULL;
  }

  if (_bar_context != None) {
    if (_bar_context != _graphics_context) {
      XFreeGC(_display, _bar_context);
    }
    _bar_context = None;

    // Also free the color we allocated.
    Colormap colormap = DefaultColormap(_display, _screen);
    XFreeColors(_display, colormap, &_bar_pixel, 1, 0);
  }

  if (_fg_pixel != -1) {
    Colormap colormap = DefaultColormap(_display, _screen);
    XFreeColors(_display, colormap, &_fg_pixel, 1, 0);
  }

  if (_bg_pixel != -1) {
    Colormap colormap = DefaultColormap(_display, _screen);
    XFreeColors(_display, colormap, &_bg_pixel, 1, 0);
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
update_image(X11ImageData &image) {
  if (!image._filename_changed) {
    // No changes.
    return;
  }
  image._filename_changed = false;

  // We'll need to rebuild the composite image.
  _needs_new_composite = true;

  // Go read the image.
  if (!read_image_data(image, image._data, image._filename)) {
    return;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::compose_image
//       Access: Private
//  Description: Constructs the XImage to display onscreen.  It's a
//               composition of the background image and/or one of the
//               button images, scaled to fit the window.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
compose_image() {
  if (_composite_image != NULL) {
    XDestroyImage(_composite_image);
    _composite_image = NULL;
  }
  _needs_new_composite = false;

  vector<unsigned char> image1;
  int image1_width = 0, image1_height = 0;
  scale_image(image1, image1_width, image1_height, _background_image);

  vector<unsigned char> image2;
  int image2_width = 0, image2_height = 0;

  switch (_bstate) {
  case BS_hidden:
    break;
  case BS_ready:
    scale_image(image2, image2_width, image2_height, _button_ready_image);
    break;
  case BS_rollover:
    if (!scale_image(image2, image2_width, image2_height, _button_rollover_image)) {
      scale_image(image2, image2_width, image2_height, _button_ready_image);
    }
    break;
  case BS_click:
    if (!scale_image(image2, image2_width, image2_height, _button_click_image)) {
      scale_image(image2, image2_width, image2_height, _button_ready_image);
    }
    break;
  }

  if (image1.empty() && image2.empty()) {
    // We have no image.  Never mind.
    return;
  }

  if (image2.empty()) {
    // We have no button image; image1 will serve as the result.

  } else {
    // We do have a button image.  Compose the button image on top of
    // the background image (or on top of a white image if we have no
    // background).

    // We compose them here on the client, because X11 doesn't
    // natively provide an alpha-blending mechanism (at least, not
    // without the XRender extension).
    vector<unsigned char> image0;
    int image0_width, image0_height;
    compose_two_images(image0, image0_width, image0_height,
                       image1, image1_width, image1_height,
                       image2, image2_width, image2_height);
    image1.swap(image0);
    image1_width = image0_width;
    image1_height = image0_height;
  }

  // Now construct an XImage from the result.
  Visual *dvisual = DefaultVisual(_display, _screen);
  double r_ratio = dvisual->red_mask / 255.0;
  double g_ratio = dvisual->green_mask / 255.0;
  double b_ratio = dvisual->blue_mask / 255.0;
  int data_length = 4 * image1_width * image1_height;
  assert(data_length == image1.size());
  uint32_t *new_data = (uint32_t*)malloc(data_length);

  int j = 0;
  for (int i = 0; i < data_length; i += 4) {
    unsigned int r, g, b;
    r = (unsigned int)(image1[i+0] * r_ratio);
    g = (unsigned int)(image1[i+1] * g_ratio);
    b = (unsigned int)(image1[i+2] * b_ratio);
    new_data[j++] = ((r & dvisual->red_mask) |
                     (g & dvisual->green_mask) |
                     (b & dvisual->blue_mask));
  }

  // Now load the image.
  _composite_image = XCreateImage(_display, CopyFromParent, DefaultDepth(_display, _screen),
                                  ZPixmap, 0, (char *)new_data, image1_width, image1_height, 32, 0);
  _composite_width = image1_width;
  _composite_height = image1_height;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::scale_image
//       Access: Private
//  Description: Scales the image into the window size, and expands it
//               to four channels.  Returns true if the image is
//               valid, false if it is empty.
////////////////////////////////////////////////////////////////////
bool P3DX11SplashWindow::
scale_image(vector<unsigned char> &image0, int &image0_width, int &image0_height,
            X11ImageData &image) {
  if (image._data.empty()) {
    return false;
  }

  const unsigned char *orig_data = (const unsigned char *)image._data.data();
  int row_stride = image._width * image._num_channels;
  int data_length = image._height * row_stride;
  assert(data_length == image._data.size());

  if (image._width <= _win_width && image._height <= _win_height) {
    // It fits within the window - just keep it.
    image0_width = image._width;
    image0_height = image._height;
    int new_row_stride = image0_width * 4;
    int new_data_length = image0_height * new_row_stride;

    image0.clear();
    image0.reserve(new_data_length);
    image0.insert(image0.begin(), new_data_length, 0);
    unsigned char *new_data = &image0[0];

    if (image._num_channels == 4) {
      // Easy case.  Already four channels.
      assert(data_length == new_data_length && row_stride == new_row_stride);
      memcpy(new_data, orig_data, data_length);

    } else if (image._num_channels == 3) {
      // Expand three channels to four.
      for (int yi = 0; yi < image._height; ++yi) {
        const unsigned char *sp = orig_data + yi * row_stride;
        unsigned char *dp = new_data + yi * new_row_stride;
        for (int xi = 0; xi < image._width; ++xi) {
          dp[0] = sp[0];
          dp[1] = sp[1];
          dp[2] = sp[2];
          dp[3] = 0xff;
          sp += 3;
          dp += 4;
        }
      }
    } else if (image._num_channels == 1) {
      // A grayscale image.  Replicate out the channels.
      for (int yi = 0; yi < image._height; ++yi) {
        const unsigned char *sp = orig_data + yi * row_stride;
        unsigned char *dp = new_data + yi * new_row_stride;
        for (int xi = 0; xi < image._width; ++xi) {
          dp[0] = sp[0];
          dp[1] = sp[0];
          dp[2] = sp[0];
          dp[3] = 0xff;
          sp += 1;
          dp += 4;
        }
      }
    }

  } else {
    // Yuck, the bad case - we need to scale it down.
    double scale = min((double)_win_width  / (double)image._width,
                       (double)_win_height / (double)image._height);
    image0_width = (int)(image._width * scale);
    image0_height = (int)(image._height * scale);
    int new_row_stride = image0_width * 4;
    int new_data_length = image0_height * new_row_stride;

    image0.clear();
    image0.reserve(new_data_length);
    image0.insert(image0.begin(), new_data_length, 0);
    unsigned char *new_data = &image0[0];

    for (int yi = 0; yi < image0_height; ++yi) {
      int orig_yi = (yi * image._height) / image0_height;
      const unsigned char *sp = orig_data + orig_yi * row_stride;
      unsigned char *dp = new_data + yi * new_row_stride;
      for (int xi = 0; xi < image0_width; ++xi) {
        int orig_xi = (xi * image._width) / image0_width;
        const unsigned char *spx = sp + orig_xi * image._num_channels;
        if (image._num_channels == 4) {
          dp[0] = spx[0];
          dp[1] = spx[1];
          dp[2] = spx[2];
          dp[3] = spx[3];
        } else if (image._num_channels == 3) {
          dp[0] = spx[0];
          dp[1] = spx[1];
          dp[2] = spx[2];
          dp[3] = 0xff;
        } else if (image._num_channels == 1) {
          dp[0] = spx[0];
          dp[1] = spx[0];
          dp[2] = spx[0];
          dp[3] = 0xff;
        }
        dp += 4;
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DX11SplashWindow::compose_two_images
//       Access: Private
//  Description: Constructs into image0 the alpha-composite of image1
//               beneath image2.
////////////////////////////////////////////////////////////////////
void P3DX11SplashWindow::
compose_two_images(vector<unsigned char> &image0, int &image0_width, int &image0_height,
                   const vector<unsigned char> &image1, int image1_width, int image1_height,
                   const vector<unsigned char> &image2, int image2_width, int image2_height) {
  // First, the resulting image size is the larger of the two.
  image0_width = max(image1_width, image2_width);
  image0_height = max(image1_height, image2_height);

  int new_row_stride = image0_width * 4;
  int new_data_length = image0_height * new_row_stride;

  // Now copy in the first image.  If the first image exactly fills
  // the output image, this is easy.
  if (image1_width == image0_width && image1_height == image0_height) {
    image0 = image1;

  } else {
    // If the first image doesn't fill it, it's only a little bit more
    // work.  Start by finding the top-left pixel.
    int xo = (image0_width - image1_width) / 2;
    int yo = (image0_height - image1_height) / 2;

    // Initialize the image to all white pixels.
    image0.clear();
    image0.reserve(new_data_length);
    image0.insert(image0.begin(), new_data_length, 0xff);

    int image0_row_stride = image0_width * 4;
    int image1_row_stride = image1_width * 4;
    for (int yi = 0; yi < image1_height; ++yi) {
      const unsigned char *sp = &image1[0] + yi * image1_row_stride;
      unsigned char *dp = &image0[0] + (yi + yo) * image0_row_stride;
      memcpy(dp + xo * 4, sp, image1_row_stride);
    }
  }

  // Now blend in the second image.  Find the top-left pixel.
  int xo = (image0_width - image2_width) / 2;
  int yo = (image0_height - image2_height) / 2;

  int image0_row_stride = image0_width * 4;
  int image2_row_stride = image2_width * 4;

  for (int yi = 0; yi < image2_height; ++yi) {
    const unsigned char *sp = &image2[0] + yi * image2_row_stride;
    unsigned char *dp = &image0[0] + (yi + yo) * image0_row_stride;
    dp += xo * 4;
    for (int xi = 0; xi < image2_width; ++xi) {
      double alpha = (double)sp[3] / 255.0;
      dp[0] = (unsigned char)(dp[0] + alpha * (sp[0] - dp[0]));
      dp[1] = (unsigned char)(dp[1] + alpha * (sp[1] - dp[1]));
      dp[2] = (unsigned char)(dp[2] + alpha * (sp[2] - dp[2]));
      dp += 4;
      sp += 4;
    }
  }
}

#endif  // HAVE_X11
