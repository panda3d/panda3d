/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file x11GraphicsWindow.cxx
 * @author rdb
 * @date 2009-07-07
 */

#include "x11GraphicsWindow.h"
#include "config_x11display.h"
#include "x11GraphicsPipe.h"

#include "graphicsPipe.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "buttonMap.h"
#include "clockObject.h"
#include "pStatTimer.h"
#include "textEncoder.h"
#include "throw_event.h"
#include "lightReMutexHolder.h"
#include "nativeWindowHandle.h"
#include "virtualFileSystem.h"
#include "get_x11.h"
#include "pnmImage.h"
#include "pnmFileTypeRegistry.h"
#include "evdevInputDevice.h"

#include <sys/time.h>
#include <fcntl.h>

using std::istream;
using std::ostringstream;
using std::string;

struct _XcursorFile {
  void *closure;
  int (*read)(XcursorFile *, unsigned char *, int);
  int (*write)(XcursorFile *, unsigned char *, int);
  int (*seek)(XcursorFile *, long, int);
};

typedef struct _XcursorImage {
  unsigned int version;
  unsigned int size;
  unsigned int width;
  unsigned int height;
  unsigned int xhot;
  unsigned int yhot;
  unsigned int delay;
  unsigned int *pixels;
} XcursorImage;

static int xcursor_read(XcursorFile *file, unsigned char *buf, int len) {
  istream* str = (istream*) file->closure;
  str->read((char*) buf, len);
  return str->gcount();
}

static int xcursor_write(XcursorFile *file, unsigned char *buf, int len) {
  // Not implemented, we don't need it.
  nassertr_always(false, 0);
  return 0;
}

static int xcursor_seek(XcursorFile *file, long offset, int whence) {
  istream* str = (istream*) file->closure;
  switch (whence) {
  case SEEK_SET:
    str->seekg(offset, istream::beg);
    break;
  case SEEK_CUR:
    str->seekg(offset, istream::cur);
    break;
  case SEEK_END:
    str->seekg(offset, istream::end);
  }

  return str->tellg();
}

TypeHandle x11GraphicsWindow::_type_handle;

/**
 *
 */
x11GraphicsWindow::
x11GraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                  const string &name,
                  const FrameBufferProperties &fb_prop,
                  const WindowProperties &win_prop,
                  int flags,
                  GraphicsStateGuardian *gsg,
                  GraphicsOutput *host) :
  GraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  x11GraphicsPipe *x11_pipe;
  DCAST_INTO_V(x11_pipe, _pipe);
  _display = x11_pipe->get_display();
  _screen = x11_pipe->get_screen();
  _xwindow = (X11_Window)nullptr;
  _ic = (XIC)nullptr;
  _visual_info = nullptr;
  _orig_size_id = -1;

  if (x11_pipe->_have_xrandr) {
    // We may still need these functions after the pipe is already destroyed,
    // so we copy them into the x11GraphicsWindow.
    _XRRGetScreenInfo = x11_pipe->_XRRGetScreenInfo;
    _XRRSetScreenConfig = x11_pipe->_XRRSetScreenConfig;
  }

  _awaiting_configure = false;
  _dga_mouse_enabled = false;
  _override_redirect = False;
  _wm_delete_window = x11_pipe->_wm_delete_window;

  PT(GraphicsWindowInputDevice) device = GraphicsWindowInputDevice::pointer_and_keyboard(this, "keyboard_mouse");
  add_input_device(device);
  _input = device;
}

/**
 *
 */
x11GraphicsWindow::
~x11GraphicsWindow() {
  if (!_cursor_filenames.empty()) {
    LightReMutexHolder holder(x11GraphicsPipe::_x_mutex);
    for (auto item : _cursor_filenames) {
      XFreeCursor(_display, item.second);
    }
  }
}

/**
 * Returns the MouseData associated with the nth input device's pointer.  This
 * is deprecated; use get_pointer_device().get_pointer() instead, or for raw
 * mice, use the InputDeviceManager interface.
 */
MouseData x11GraphicsWindow::
get_pointer(int device) const {
  MouseData result;
  {
    LightMutexHolder holder(_input_lock);
    nassertr(device >= 0 && device < (int)_input_devices.size(), MouseData());

    result = ((const GraphicsWindowInputDevice *)_input_devices[device].p())->get_pointer();

    // We recheck this immediately to get the most up-to-date value, but we
    // won't bother waiting for the lock if we can't.
    if (device == 0 && !_dga_mouse_enabled && result._in_window &&
        x11GraphicsPipe::_x_mutex.try_lock()) {
      XEvent event;
      if (_xwindow != None &&
          XQueryPointer(_display, _xwindow, &event.xbutton.root,
          &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root,
          &event.xbutton.x, &event.xbutton.y, &event.xbutton.state)) {
        double time = ClockObject::get_global_clock()->get_real_time();
        result._xpos = event.xbutton.x;
        result._ypos = event.xbutton.y;
        ((GraphicsWindowInputDevice *)_input_devices[0].p())->set_pointer_in_window(result._xpos, result._ypos, time);
      }
      x11GraphicsPipe::_x_mutex.release();
    }
  }
  return result;
}

/**
 * Forces the pointer to the indicated position within the window, if
 * possible.
 *
 * Returns true if successful, false on failure.  This may fail if the mouse
 * is not currently within the window, or if the API doesn't support this
 * operation.
 */
bool x11GraphicsWindow::
move_pointer(int device, int x, int y) {
  // Note: this is not thread-safe; it should be called only from App.
  // Probably not an issue.
  if (device == 0) {
    // Move the system mouse pointer.
    PointerData md = _input->get_pointer();
    if (!_properties.get_foreground() || !md.get_in_window()) {
      // If the window doesn't have input focus, or the mouse isn't currently
      // within the window, forget it.
      return false;
    }

    if (!md.get_in_window() || md.get_x() != x || md.get_y() != y) {
      if (!_dga_mouse_enabled) {
        LightReMutexHolder holder(x11GraphicsPipe::_x_mutex);
        XWarpPointer(_display, None, _xwindow, 0, 0, 0, 0, x, y);
      }
      _input->set_pointer_in_window(x, y);
    }
    return true;
  } else {
    // Can't move a raw mouse.
    return false;
  }
}

/**
 * Clears the entire framebuffer before rendering, according to the settings
 * of get_color_clear_active() and get_depth_clear_active() (inherited from
 * DrawableRegion).
 *
 * This function is called only within the draw thread.
 */
void x11GraphicsWindow::
clear(Thread *current_thread) {
  if (is_any_clear_active()) {
    // Evidently the NVIDIA driver may call glXCreateNewContext inside
    // prepare_display_region, so we need to hold the X11 lock.
    LightReMutexHolder holder(x11GraphicsPipe::_x_mutex);
    GraphicsOutput::clear(current_thread);
  }
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool x11GraphicsWindow::
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

  // Reset the GSG state if this is the first time it has been used.  (We
  // can't just call reset() when we construct the GSG, because reset()
  // requires having a current context.)
  _gsg->reset_if_new();

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
void x11GraphicsWindow::
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
 * Do whatever processing is necessary to ensure that the window responds to
 * user events.  Also, honor any requests recently made via
 * request_properties()
 *
 * This function is called only within the window thread.
 */
void x11GraphicsWindow::
process_events() {
  LightReMutexHolder holder(x11GraphicsPipe::_x_mutex);

  GraphicsWindow::process_events();

  if (_xwindow == (X11_Window)0) {
    return;
  }

  XEvent event;
  XKeyEvent keyrelease_event;
  bool got_keyrelease_event = false;

  XConfigureEvent configure_event;
  bool got_configure_event = false;

  WindowProperties properties;
  bool changed_properties = false;

  while (XCheckIfEvent(_display, &event, check_event, (char *)this)) {
    if (XFilterEvent(&event, None)) {
      continue;
    }

    if (got_keyrelease_event) {
      // If a keyrelease event is immediately followed by a matching keypress
      // event, that's just key repeat and we should treat the two events
      // accordingly.  It would be nice if X provided a way to differentiate
      // between keyrepeat and explicit keypresses more generally.
      got_keyrelease_event = false;

      if (event.type == KeyPress &&
          event.xkey.keycode == keyrelease_event.keycode &&
          (event.xkey.time - keyrelease_event.time <= 1)) {
        // In particular, we only generate down messages for the repeated
        // keys, not down-and-up messages.
        handle_keystroke(event.xkey);

        // We thought about not generating the keypress event, but we need
        // that repeat for backspace.  Rethink later.
        handle_keypress(event.xkey);
        continue;

      } else {
        // This keyrelease event is not immediately followed by a matching
        // keypress event, so it's a genuine release.
        handle_keyrelease(keyrelease_event);
      }
    }

    ButtonHandle button;

    switch (event.type) {
    case ReparentNotify:
      break;

    case ConfigureNotify:
      // When resizing or moving the window, multiple ConfigureNotify events
      // may be sent in rapid succession.  We only respond to the last one.
      configure_event = event.xconfigure;
      got_configure_event = true;
      break;

    case ButtonPress:
      // This refers to the mouse buttons.
      button = get_mouse_button(event.xbutton);
      if (!_dga_mouse_enabled) {
        _input->set_pointer_in_window(event.xbutton.x, event.xbutton.y);
      }
      _input->button_down(button);
      break;

    case ButtonRelease:
      button = get_mouse_button(event.xbutton);
      if (!_dga_mouse_enabled) {
        _input->set_pointer_in_window(event.xbutton.x, event.xbutton.y);
      }
      _input->button_up(button);
      break;

    case MotionNotify:
      if (_dga_mouse_enabled) {
        PointerData md = _input->get_pointer();
        _input->set_pointer_in_window(md.get_x() + event.xmotion.x_root, md.get_y() + event.xmotion.y_root);
      } else {
        _input->set_pointer_in_window(event.xmotion.x, event.xmotion.y);
      }
      break;

    case KeyPress:
      handle_keystroke(event.xkey);
      handle_keypress(event.xkey);
      break;

    case KeyRelease:
      // The KeyRelease can't be processed immediately, because we have to
      // check first if it's immediately followed by a matching KeyPress
      // event.
      keyrelease_event = event.xkey;
      got_keyrelease_event = true;
      break;

    case EnterNotify:
      if (_dga_mouse_enabled) {
        PointerData md = _input->get_pointer();
        _input->set_pointer_in_window(md.get_x(), md.get_y());
      } else {
        _input->set_pointer_in_window(event.xcrossing.x, event.xcrossing.y);
      }
      break;

    case LeaveNotify:
      _input->set_pointer_out_of_window();
      break;

    case FocusIn:
      properties.set_foreground(true);
      changed_properties = true;
      break;

    case FocusOut:
      _input->focus_lost();
      properties.set_foreground(false);
      changed_properties = true;
      break;

    case UnmapNotify:
      properties.set_minimized(true);
      changed_properties = true;
      break;

    case MapNotify:
      properties.set_minimized(false);
      changed_properties = true;

      // Auto-focus the window when it is mapped.
      XSetInputFocus(_display, _xwindow, RevertToPointerRoot, CurrentTime);
      break;

    case ClientMessage:
      if ((Atom)(event.xclient.data.l[0]) == _wm_delete_window) {
        // This is a message from the window manager indicating that the user
        // has requested to close the window.
        string close_request_event = get_close_request_event();
        if (!close_request_event.empty()) {
          // In this case, the app has indicated a desire to intercept the
          // request and process it directly.
          throw_event(close_request_event);

        } else {
          // In this case, the default case, the app does not intend to
          // service the request, so we do by closing the window.

          // TODO: don't release the gsg in the window thread.
          close_window();
          properties.set_open(false);
          system_changed_properties(properties);
        }
      }
      break;

    case DestroyNotify:
      // Apparently, we never get a DestroyNotify on a toplevel window.
      // Instead, we rely on hints from the window manager (see above).
      x11display_cat.info()
        << "DestroyNotify\n";
      break;

    default:
      x11display_cat.warning()
        << "unhandled X event type " << event.type << "\n";
    }
  }

  if (got_configure_event) {
    // Now handle the last configure event we found.
    _awaiting_configure = false;

    // Is this the inner corner or the outer corner?  The Xlib docs say it
    // should be the outer corner, but it appears to be the inner corner on my
    // own implementation, which is inconsistent with XConfigureWindow.
    // (Panda really wants to work with the inner corner, anyway, but that
    // means we need to fix XConfigureWindow too.)
    properties.set_origin(configure_event.x, configure_event.y);
    properties.set_size(configure_event.width, configure_event.height);

    if (_properties.get_fixed_size()) {
      // If the window properties indicate a fixed size only, undo any attempt
      // by the user to change them.  In X, there doesn't appear to be a way
      // to universally disallow this directly (although we do set the
      // min_size and max_size to the same value, which seems to work for most
      // window managers.)
      if (configure_event.width != _fixed_size.get_x() ||
          configure_event.height != _fixed_size.get_y()) {
        XWindowChanges changes;
        changes.width = _fixed_size.get_x();
        changes.height = _fixed_size.get_y();
        int value_mask = (CWWidth | CWHeight);
        XConfigureWindow(_display, _xwindow, value_mask, &changes);
      }
    }

    // If the window was reconfigured, we may need to re-confine the mouse
    // pointer.  See GitHub bug #280.
    if (_properties.get_mouse_mode() == WindowProperties::M_confined) {
      X11_Cursor cursor = None;
      if (_properties.get_cursor_hidden()) {
        x11GraphicsPipe *x11_pipe;
        DCAST_INTO_V(x11_pipe, _pipe);
        cursor = x11_pipe->get_hidden_cursor();
      }

      XGrabPointer(_display, _xwindow, True, 0, GrabModeAsync, GrabModeAsync,
                   _xwindow, cursor, CurrentTime);
    }

    changed_properties = true;
  }

  if (properties.has_foreground() && (
        _properties.get_mouse_mode() == WindowProperties::M_confined ||
        _dga_mouse_enabled)) {
       x11GraphicsPipe *x11_pipe;
       DCAST_INTO_V(x11_pipe, _pipe);

      // Focus has changed, let's let go of the pointer if we've grabbed or re-grab it if needed
      if (properties.get_foreground()) {
        // Window is going to the foreground, re-grab the pointer
        X11_Cursor cursor = None;
        if (_properties.get_cursor_hidden()) {
            cursor = x11_pipe->get_hidden_cursor();
        }

        XGrabPointer(_display, _xwindow, True, 0, GrabModeAsync, GrabModeAsync,
                    _xwindow, cursor, CurrentTime);
        if (_dga_mouse_enabled) {
          x11_pipe->enable_relative_mouse();
        }
      }
      else {
        // window is leaving the foreground, ungrab the pointer
        if (_dga_mouse_enabled) {
          x11_pipe->disable_relative_mouse();
        } else if (_properties.get_mouse_mode() == WindowProperties::M_confined) {
          XUngrabPointer(_display, CurrentTime);
        }
      }
  }

  if (changed_properties) {
    system_changed_properties(properties);
  }

  if (got_keyrelease_event) {
    // This keyrelease event is not immediately followed by a matching
    // keypress event, so it's a genuine release.
    handle_keyrelease(keyrelease_event);
  }
}

/**
 * Applies the requested set of properties to the window, if possible, for
 * instance to request a change in size or minimization status.
 *
 * The window properties are applied immediately, rather than waiting until
 * the next frame.  This implies that this method may *only* be called from
 * within the window thread.
 *
 * The return value is true if the properties are set, false if they are
 * ignored.  This is mainly useful for derived classes to implement extensions
 * to this function.
 */
void x11GraphicsWindow::
set_properties_now(WindowProperties &properties) {
  if (_pipe == nullptr) {
    // If the pipe is null, we're probably closing down.
    GraphicsWindow::set_properties_now(properties);
    return;
  }

  x11GraphicsPipe *x11_pipe;
  DCAST_INTO_V(x11_pipe, _pipe);

  LightReMutexHolder holder(x11GraphicsPipe::_x_mutex);

  // We're either going into or out of fullscreen, or are in fullscreen and
  // are changing the resolution.
  bool is_fullscreen = _properties.has_fullscreen() && _properties.get_fullscreen();
  bool want_fullscreen = properties.has_fullscreen() ? properties.get_fullscreen() : is_fullscreen;

  if (want_fullscreen && properties.has_origin()) {
    // If we're fullscreen, reject changes to the origin.
    properties.clear_origin();
  }

  if (is_fullscreen != want_fullscreen || (is_fullscreen && properties.has_size())) {
    if (want_fullscreen) {
      // OK, first figure out which CRTC the window is on.  It may be on more
      // than one, actually, so grab a point in the center in order to figure
      // out which one it's more-or-less mostly on.
      LPoint2i center(0, 0);
      if (_properties.has_origin()) {
        center = _properties.get_origin();
        if (_properties.has_size()) {
          center += _properties.get_size() / 2;
        }
      }
      int x, y, width, height;
      x11_pipe->find_fullscreen_crtc(center, x, y, width, height);

      // Which size should we go fullscreen in?
      int reqsizex, reqsizey;
      if (properties.has_size()) {
        reqsizex = properties.get_x_size();
        reqsizey = properties.get_y_size();
      } else if (_properties.has_size()) {
        reqsizex = _properties.get_x_size();
        reqsizey = _properties.get_y_size();
      } else {
        reqsizex = width;
        reqsizey = height;
      }

      // Are we passing in pipe.display_width/height?  This is actually the
      // size of the virtual desktop, which may not be a real resolution, so
      // if that is passed in, we have to assume that the user means to just
      // fullscreen without changing the screen resolution.
      if ((reqsizex == x11_pipe->get_display_width() &&
           reqsizey == x11_pipe->get_display_height())
          || (width == reqsizex && height == reqsizey)
          || !x11_pipe->_have_xrandr) {

        // Cover the current CRTC.
        properties.set_origin(x, y);
        properties.set_size(width, height);

        if (x11display_cat.is_debug()) {
          x11display_cat.debug()
            << "Setting window to fullscreen on CRTC "
            << width << "x" << height << "+" << x << "+" << y << "\n";
        }
      } else {
        // We may need to change the screen resolution.  The code below is
        // suboptimal; in the future, we probably want to only touch the CRTC
        // that the window is on.
        XRRScreenConfiguration *conf = _XRRGetScreenInfo(_display, _xwindow ? _xwindow : x11_pipe->get_root());
        SizeID old_size_id = x11_pipe->_XRRConfigCurrentConfiguration(conf, &_orig_rotation);
        SizeID new_size_id = (SizeID) -1;
        int num_sizes = 0;

        XRRScreenSize *xrrs;
        xrrs = x11_pipe->_XRRSizes(_display, 0, &num_sizes);
        for (int i = 0; i < num_sizes; ++i) {
          if (xrrs[i].width == reqsizex &&
              xrrs[i].height == reqsizey) {
            new_size_id = i;
          }
        }
        if (new_size_id == (SizeID) -1) {
          x11display_cat.error()
            << "Videocard has no supported display resolutions at specified res ("
            << reqsizex << " x " << reqsizey << ")\n";

          // Just go fullscreen at native resolution, then.
          properties.set_origin(x, y);
          properties.set_size(width, height);
        } else {
          if (x11display_cat.is_debug()) {
            x11display_cat.debug()
              << "Switching to fullscreen with resolution "
              << reqsizex << "x" << reqsizey << "\n";
          }

          if (new_size_id != old_size_id) {
            _XRRSetScreenConfig(_display, conf, x11_pipe->get_root(), new_size_id, _orig_rotation, CurrentTime);
            if (_orig_size_id == (SizeID) -1) {
              // Remember the original resolution so we can switch back to it.
              _orig_size_id = old_size_id;
            }

            // Since the above changes the entire screen configuration, we
            // have to set the origin to 0, 0.
            properties.set_origin(0, 0);
          }
        }
      }
    } else {
      // Change the resolution back to what it was.  Don't remove the SizeID
      // typecast!
      if (_orig_size_id != (SizeID) -1) {
        XRRScreenConfiguration *conf = _XRRGetScreenInfo(_display, x11_pipe->get_root());
        _XRRSetScreenConfig(_display, conf, x11_pipe->get_root(), _orig_size_id, _orig_rotation, CurrentTime);
        _orig_size_id = (SizeID) -1;
      }
      // Set the origin back to what it was
      if (!properties.has_origin() && _properties.has_origin()) {
        properties.set_origin(_properties.get_x_origin(), _properties.get_y_origin());
      }
    }
  }

  if (properties.has_origin()) {
    // A coordinate of -2 means to center the window on screen.
    if (properties.get_x_origin() == -2 || properties.get_y_origin() == -2) {
      int x_origin = properties.get_x_origin();
      int y_origin = properties.get_y_origin();
      if (properties.has_size()) {
        if (x_origin == -2) {
          x_origin = 0.5 * (x11_pipe->get_display_width() - properties.get_x_size());
        }
        if (y_origin == -2) {
          y_origin = 0.5 * (x11_pipe->get_display_height() - properties.get_y_size());
        }
      } else {
        if (x_origin == -2) {
          x_origin = 0.5 * (x11_pipe->get_display_width() - _properties.get_x_size());
        }
        if (y_origin == -2) {
          y_origin = 0.5 * (x11_pipe->get_display_height() - _properties.get_y_size());
        }
      }
      properties.set_origin(x_origin, y_origin);
    }
  }

  GraphicsWindow::set_properties_now(properties);
  if (!properties.is_any_specified()) {
    // The base class has already handled this case.
    return;
  }

  // The window is already open; we are limited to what we can change on the
  // fly.

  // We'll pass some property requests on as a window manager hint.
  set_wm_properties(properties, true);

  // The window title may be changed by issuing another hint request.  Assume
  // this will be honored.
  if (properties.has_title()) {
    _properties.set_title(properties.get_title());
    properties.clear_title();
  }

  // Same for fullscreen.
  if (properties.has_fullscreen()) {
    _properties.set_fullscreen(properties.get_fullscreen());
    properties.clear_fullscreen();
  }

  // The size and position of an already-open window are changed via explicit
  // X calls.  These may still get intercepted by the window manager.  Rather
  // than changing _properties immediately, we'll wait for the ConfigureNotify
  // message to come back.
  XWindowChanges changes;
  int value_mask = 0;

  if (_properties.get_fullscreen()) {
    changes.x = 0;
    changes.y = 0;
    value_mask |= CWX | CWY;
    properties.clear_origin();

  } else if (properties.has_origin()) {
    changes.x = properties.get_x_origin();
    changes.y = properties.get_y_origin();
    if (changes.x != -1) value_mask |= CWX;
    if (changes.y != -1) value_mask |= CWY;
    properties.clear_origin();
  }

  // This, too.  But we can't currently change out of fixed_size mode.
  if (properties.has_fixed_size() && properties.get_fixed_size()) {
    _properties.set_fixed_size(properties.get_fixed_size());
    properties.clear_fixed_size();
    _fixed_size = _properties.get_size();
  }

  if (properties.has_size()) {
    changes.width = properties.get_x_size();
    changes.height = properties.get_y_size();
    value_mask |= (CWWidth | CWHeight);

    if (_properties.get_fixed_size()) {
      _fixed_size = properties.get_size();
    }
    properties.clear_size();
  }

  if (properties.has_z_order()) {
    // We'll send the classic stacking request through the standard interface,
    // for users of primitive window managers; but we'll also send it as a
    // window manager hint, for users of modern window managers.
    _properties.set_z_order(properties.get_z_order());
    switch (properties.get_z_order()) {
    case WindowProperties::Z_bottom:
      changes.stack_mode = Below;
      break;

    case WindowProperties::Z_normal:
      changes.stack_mode = TopIf;
      break;

    case WindowProperties::Z_top:
      changes.stack_mode = Above;
      break;
    }

    value_mask |= (CWStackMode);
    properties.clear_z_order();
  }

  // We hide the cursor by setting it to an invisible pixmap.  We can also
  // load a custom cursor from a file.
  if (properties.has_cursor_hidden() || properties.has_cursor_filename()) {
    if (properties.has_cursor_hidden()) {
      _properties.set_cursor_hidden(properties.get_cursor_hidden());
      properties.clear_cursor_hidden();
    }
    Filename cursor_filename;
    if (properties.has_cursor_filename()) {
      cursor_filename = properties.get_cursor_filename();
      _properties.set_cursor_filename(cursor_filename);
      properties.clear_cursor_filename();
    }
    Filename filename = properties.get_cursor_filename();
    _properties.set_cursor_filename(filename);

    if (_properties.get_cursor_hidden()) {
      XDefineCursor(_display, _xwindow, x11_pipe->get_hidden_cursor());

    } else if (!cursor_filename.empty()) {
      // Note that if the cursor fails to load, cursor will be None
      X11_Cursor cursor = get_cursor(cursor_filename);
      XDefineCursor(_display, _xwindow, cursor);

    } else {
      XDefineCursor(_display, _xwindow, None);
    }

    // Regrab the mouse if we changed the cursor, otherwise it won't update.
    if (!properties.has_mouse_mode() &&
        _properties.get_mouse_mode() != WindowProperties::M_absolute) {
      properties.set_mouse_mode(_properties.get_mouse_mode());
    }
  }

  if (properties.has_foreground()) {
    if (properties.get_foreground()) {
      XSetInputFocus(_display, _xwindow, RevertToPointerRoot, CurrentTime);
    } else {
      XSetInputFocus(_display, PointerRoot, RevertToPointerRoot, CurrentTime);
    }
    properties.clear_foreground();
  }

  if (properties.has_mouse_mode()) {
    switch (properties.get_mouse_mode()) {
    case WindowProperties::M_absolute:
      XUngrabPointer(_display, CurrentTime);
      if (_dga_mouse_enabled) {
        x11_pipe->disable_relative_mouse();
        _dga_mouse_enabled = false;
      }
      _properties.set_mouse_mode(WindowProperties::M_absolute);
      properties.clear_mouse_mode();
      break;

    case WindowProperties::M_relative:
      if (!_dga_mouse_enabled) {
        if (x11_pipe->supports_relative_mouse()) {
          X11_Cursor cursor = None;
          if (_properties.get_cursor_hidden()) {
            x11GraphicsPipe *x11_pipe;
            DCAST_INTO_V(x11_pipe, _pipe);
            cursor = x11_pipe->get_hidden_cursor();
          }

          if (XGrabPointer(_display, _xwindow, True, 0, GrabModeAsync,
              GrabModeAsync, _xwindow, cursor, CurrentTime) != GrabSuccess) {
            x11display_cat.error() << "Failed to grab pointer!\n";
          } else {
            x11_pipe->enable_relative_mouse();

            _properties.set_mouse_mode(WindowProperties::M_relative);
            properties.clear_mouse_mode();
            _dga_mouse_enabled = true;

            // Get the real mouse position, so we can addsubtract our relative
            // coordinates later.
            XEvent event;
            XQueryPointer(_display, _xwindow, &event.xbutton.root,
              &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root,
              &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
            _input->set_pointer_in_window(event.xbutton.x, event.xbutton.y);
          }
        } else {
          x11display_cat.warning()
            << "XF86DGA extension not available, cannot enable relative mouse mode\n";
          _dga_mouse_enabled = false;
        }
      }
      break;

    case WindowProperties::M_confined:
      {
        x11GraphicsPipe *x11_pipe;
        DCAST_INTO_V(x11_pipe, _pipe);

        if (_dga_mouse_enabled) {
          x11_pipe->disable_relative_mouse();
          _dga_mouse_enabled = false;
        }
        X11_Cursor cursor = None;
        if (_properties.get_cursor_hidden()) {
          cursor = x11_pipe->get_hidden_cursor();
        }

        if (XGrabPointer(_display, _xwindow, True, 0, GrabModeAsync,
            GrabModeAsync, _xwindow, cursor, CurrentTime) != GrabSuccess) {
          x11display_cat.error() << "Failed to grab pointer!\n";
        } else {
          _properties.set_mouse_mode(WindowProperties::M_confined);
          properties.clear_mouse_mode();
        }
      }
      break;
    }
  }

  if (value_mask != 0) {
    // We must call this after changing the WM properties, otherwise we may
    // get misleading ConfigureNotify events in the wrong order.
    XReconfigureWMWindow(_display, _xwindow, _screen, value_mask, &changes);

    // Don't draw anything until this is done reconfiguring.
    _awaiting_configure = true;
  }
}

/**
 * Overridden from GraphicsWindow.
 */
void x11GraphicsWindow::
mouse_mode_absolute() {
  // unused: remove in 1.10!
}

/**
 * Overridden from GraphicsWindow.
 */
void x11GraphicsWindow::
mouse_mode_relative() {
  // unused: remove in 1.10!
}

/**
 * Closes the window right now.  Called from the window thread.
 */
void x11GraphicsWindow::
close_window() {
  if (_gsg != nullptr) {
    _gsg.clear();
  }

  LightReMutexHolder holder(x11GraphicsPipe::_x_mutex);
  if (_ic != (XIC)nullptr) {
    XDestroyIC(_ic);
    _ic = (XIC)nullptr;
  }

  if (_xwindow != (X11_Window)nullptr) {
    XDestroyWindow(_display, _xwindow);
    _xwindow = (X11_Window)nullptr;

    // This may be necessary if we just closed the last X window in an
    // application, so the server hears the close request.
    XFlush(_display);
  }

  // Change the resolution back to what it was.  Don't remove the SizeID
  // typecast!
  if (_orig_size_id != (SizeID) -1) {
    X11_Window root;
    if (_pipe != nullptr) {
      x11GraphicsPipe *x11_pipe;
      DCAST_INTO_V(x11_pipe, _pipe);
      root = x11_pipe->get_root();
    } else {
      // Oops.  Looks like the pipe was destroyed before the window gets
      // closed.  Oh well, let's get the root window by ourselves.
      root = RootWindow(_display, _screen);
    }
    XRRScreenConfiguration *conf = _XRRGetScreenInfo(_display, root);
    _XRRSetScreenConfig(_display, conf, root, _orig_size_id, _orig_rotation, CurrentTime);
    _orig_size_id = -1;
  }

  GraphicsWindow::close_window();
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool x11GraphicsWindow::
open_window() {
  if (_visual_info == nullptr) {
    // No X visual for this fbconfig; how can we open the window?
    x11display_cat.error()
      << "No X visual: cannot open window.\n";
    return false;
  }

  x11GraphicsPipe *x11_pipe;
  DCAST_INTO_R(x11_pipe, _pipe, false);

  if (!_properties.has_origin()) {
    _properties.set_origin(0, 0);
  }
  if (!_properties.has_size()) {
    _properties.set_size(100, 100);
  }

  // Make sure we are not making X11 calls from other threads.
  LightReMutexHolder holder(x11GraphicsPipe::_x_mutex);

  X11_Window parent_window = x11_pipe->get_root();
  WindowHandle *window_handle = _properties.get_parent_window();
  if (window_handle != nullptr) {
    x11display_cat.info()
      << "Got parent_window " << *window_handle << "\n";
    WindowHandle::OSHandle *os_handle = window_handle->get_os_handle();
    if (os_handle != nullptr) {
      x11display_cat.info()
        << "os_handle type " << os_handle->get_type() << "\n";

      if (os_handle->is_of_type(NativeWindowHandle::X11Handle::get_class_type())) {
        NativeWindowHandle::X11Handle *x11_handle = DCAST(NativeWindowHandle::X11Handle, os_handle);
        parent_window = x11_handle->get_handle();
      } else if (os_handle->is_of_type(NativeWindowHandle::IntHandle::get_class_type())) {
        NativeWindowHandle::IntHandle *int_handle = DCAST(NativeWindowHandle::IntHandle, os_handle);
        parent_window = (X11_Window)int_handle->get_handle();
      }
    }
  }
  _parent_window_handle = window_handle;

  _event_mask =
    ButtonPressMask | ButtonReleaseMask |
    KeyPressMask | KeyReleaseMask |
    EnterWindowMask | LeaveWindowMask |
    PointerMotionMask |
    FocusChangeMask | StructureNotifyMask;

  // Initialize window attributes
  XSetWindowAttributes wa;
  wa.background_pixel = XBlackPixel(_display, _screen);
  wa.border_pixel = 0;
  wa.colormap = _colormap;
  wa.event_mask = _event_mask;
  wa.override_redirect = _override_redirect;

  unsigned long attrib_mask =
    CWBackPixel | CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect;

  _xwindow = XCreateWindow
    (_display, parent_window,
     _properties.get_x_origin(), _properties.get_y_origin(),
     _properties.get_x_size(), _properties.get_y_size(),
     0, _visual_info->depth, InputOutput,
     _visual_info->visual, attrib_mask, &wa);

  if (_xwindow == (X11_Window)0) {
    x11display_cat.error()
      << "failed to create X window.\n";
    return false;
  }

  if (_properties.get_fixed_size()) {
    _fixed_size = _properties.get_size();
  }

  set_wm_properties(_properties, false);

  // We don't specify any fancy properties of the XIC.  It would be nicer if
  // we could support fancy IM's that want preedit callbacks, etc., but that
  // can wait until we have an X server that actually supports these to test
  // it on.
  XIM im = x11_pipe->get_im();
  _ic = nullptr;
  if (im) {
    _ic = XCreateIC
      (im,
       XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
       nullptr);
    if (_ic == (XIC)nullptr) {
      x11display_cat.warning()
        << "Couldn't create input context.\n";
    }
  }

  if (_properties.get_cursor_hidden()) {
    XDefineCursor(_display, _xwindow, x11_pipe->get_hidden_cursor());

  } else if (_properties.has_cursor_filename() && !_properties.get_cursor_filename().empty()) {
    // Note that if the cursor fails to load, cursor will be None
    X11_Cursor cursor = get_cursor(_properties.get_cursor_filename());
    XDefineCursor(_display, _xwindow, cursor);
  }

  XMapWindow(_display, _xwindow);

  if (_properties.get_raw_mice()) {
    open_raw_mice();
  } else {
    if (x11display_cat.is_debug()) {
      x11display_cat.debug()
        << "Raw mice not requested.\n";
    }
  }

  // Create a WindowHandle for ourselves
  _window_handle = NativeWindowHandle::make_x11(_xwindow);

  // And tell our parent window that we're now its child.
  if (_parent_window_handle != nullptr) {
    _parent_window_handle->attach_child(_window_handle);
  }

  return true;
}

/**
 * Asks the window manager to set the appropriate properties.  In X, these
 * properties cannot be specified directly by the application; they must be
 * requested via the window manager, which may or may not choose to honor the
 * request.
 *
 * If already_mapped is true, the window has already been mapped (manifested)
 * on the display.  This means we may need to use a different action in some
 * cases.
 *
 * Assumes the X11 lock is held.
 */
void x11GraphicsWindow::
set_wm_properties(const WindowProperties &properties, bool already_mapped) {
  x11GraphicsPipe *x11_pipe;
  DCAST_INTO_V(x11_pipe, _pipe);

  // Name the window if there is a name
  XTextProperty window_name;
  XTextProperty *window_name_p = nullptr;
  if (properties.has_title()) {
    const char *name = properties.get_title().c_str();
    if (XStringListToTextProperty((char **)&name, 1, &window_name) != 0) {
      window_name_p = &window_name;
    }
  }

  // The size hints request a window of a particular size andor a particular
  // placement onscreen.
  XSizeHints *size_hints_p = nullptr;
  if (properties.has_origin() || properties.has_size()) {
    size_hints_p = XAllocSizeHints();
    if (size_hints_p != nullptr) {
      if (properties.has_origin()) {
        size_hints_p->x = properties.get_x_origin();
        size_hints_p->y = properties.get_y_origin();
        size_hints_p->flags |= USPosition;
      }
      LVecBase2i size = _properties.get_size();
      if (properties.has_size()) {
        size = properties.get_size();
        size_hints_p->width = size.get_x();
        size_hints_p->height = size.get_y();
        size_hints_p->flags |= USSize;
      }
      if (properties.get_fixed_size()) {
        size_hints_p->min_width = size.get_x();
        size_hints_p->min_height = size.get_y();
        size_hints_p->max_width = size.get_x();
        size_hints_p->max_height = size.get_y();
        size_hints_p->flags |= (PMinSize | PMaxSize);
      }
    }
  }

  // The window manager hints include requests to the window manager other
  // than those specific to window geometry.
  XWMHints *wm_hints_p = nullptr;
  wm_hints_p = XAllocWMHints();
  if (wm_hints_p != nullptr) {
    if (properties.has_minimized() && properties.get_minimized()) {
      wm_hints_p->initial_state = IconicState;
    } else {
      wm_hints_p->initial_state = NormalState;
    }
    wm_hints_p->flags = StateHint;
  }

  // Two competing window manager interfaces have evolved.  One of them allows
  // to set certain properties as a "type"; the other one as a "state".  We'll
  // try to honor both.
  static const int max_type_data = 32;
  int32_t type_data[max_type_data];
  int next_type_data = 0;

  static const int max_state_data = 32;
  int32_t state_data[max_state_data];
  int next_state_data = 0;

  static const int max_set_data = 32;
  class SetAction {
  public:
    inline SetAction() { }
    inline SetAction(Atom state, Atom action) : _state(state), _action(action) { }
    Atom _state;
    Atom _action;
  };
  SetAction set_data[max_set_data];
  int next_set_data = 0;

  if (properties.has_fullscreen()) {
    if (properties.get_fullscreen()) {
      // For a "fullscreen" request, we pass this through, hoping the window
      // manager will support EWMH.
      type_data[next_type_data++] = x11_pipe->_net_wm_window_type_fullscreen;

      // We also request it as a state.
      state_data[next_state_data++] = x11_pipe->_net_wm_state_fullscreen;
      // Don't ask me why this has to be 10 and not _net_wm_state_add.  It
      // doesn't seem to work otherwise.
      set_data[next_set_data++] = SetAction(x11_pipe->_net_wm_state_fullscreen, 1);

    } else {
      set_data[next_set_data++] = SetAction(x11_pipe->_net_wm_state_fullscreen, 0);
    }
  }

  // If we asked for a window without a border, there's no excellent way to
  // arrange that.  For users whose window managers follow the EWMH
  // specification, we can ask for a "splash" screen, which is usually
  // undecorated.  It's not exactly right, but the spec doesn't give us an
  // exactly-right option.

  // For other users, we'll totally punt and just set the window's Class to
  // "Undecorated", and let the user configure hisher window manager not to
  // put a border around windows of this class.
  XClassHint *class_hints_p = nullptr;
  if (!x_wm_class.empty()) {
    // Unless the user wanted to use his own WM_CLASS, of course.
    class_hints_p = XAllocClassHint();
    class_hints_p->res_class = (char*) x_wm_class.c_str();
    if (!x_wm_class_name.empty()) {
      class_hints_p->res_name = (char*) x_wm_class_name.c_str();
    }

  } else if (properties.get_undecorated() || properties.get_fullscreen()) {
    class_hints_p = XAllocClassHint();
    class_hints_p->res_class = (char*) "Undecorated";
  }

  if (properties.get_undecorated() && !properties.get_fullscreen()) {
    type_data[next_type_data++] = x11_pipe->_net_wm_window_type_splash;
  }

  if (properties.has_z_order()) {
    switch (properties.get_z_order()) {
    case WindowProperties::Z_bottom:
      state_data[next_state_data++] = x11_pipe->_net_wm_state_below;
      set_data[next_set_data++] = SetAction(x11_pipe->_net_wm_state_below,
                                            x11_pipe->_net_wm_state_add);
      set_data[next_set_data++] = SetAction(x11_pipe->_net_wm_state_above,
                                            x11_pipe->_net_wm_state_remove);
      break;

    case WindowProperties::Z_normal:
      set_data[next_set_data++] = SetAction(x11_pipe->_net_wm_state_below,
                                            x11_pipe->_net_wm_state_remove);
      set_data[next_set_data++] = SetAction(x11_pipe->_net_wm_state_above,
                                            x11_pipe->_net_wm_state_remove);
      break;

    case WindowProperties::Z_top:
      state_data[next_state_data++] = x11_pipe->_net_wm_state_above;
      set_data[next_set_data++] = SetAction(x11_pipe->_net_wm_state_below,
                                            x11_pipe->_net_wm_state_remove);
      set_data[next_set_data++] = SetAction(x11_pipe->_net_wm_state_above,
                                            x11_pipe->_net_wm_state_add);
      break;
    }
  }

  nassertv(next_type_data < max_type_data);
  nassertv(next_state_data < max_state_data);
  nassertv(next_set_data < max_set_data);

  // Add the process ID as a convenience for other applications.
  int32_t pid = getpid();
  XChangeProperty(_display, _xwindow, x11_pipe->_net_wm_pid,
                  XA_CARDINAL, 32, PropModeReplace,
                  (unsigned char *)&pid, 1);

  // Disable compositing effects in fullscreen mode.
  if (properties.has_fullscreen()) {
    int32_t compositor = properties.get_fullscreen() ? 1 : 0;
    XChangeProperty(_display, _xwindow, x11_pipe->_net_wm_bypass_compositor,
                    XA_CARDINAL, 32, PropModeReplace,
                    (unsigned char *)&compositor, 1);
  }

  XChangeProperty(_display, _xwindow, x11_pipe->_net_wm_window_type,
                  XA_ATOM, 32, PropModeReplace,
                  (unsigned char *)type_data, next_type_data);

  // Request the state properties all at once.
  XChangeProperty(_display, _xwindow, x11_pipe->_net_wm_state,
                  XA_ATOM, 32, PropModeReplace,
                  (unsigned char *)state_data, next_state_data);

  if (already_mapped) {
    // We have to request state changes differently when the window has been
    // mapped.  To do this, we need to send a client message to the root
    // window for each change.

    x11GraphicsPipe *x11_pipe;
    DCAST_INTO_V(x11_pipe, _pipe);

    for (int i = 0; i < next_set_data; ++i) {
      XClientMessageEvent event;
      memset(&event, 0, sizeof(event));
      event.type = ClientMessage;
      event.send_event = True;
      event.display = _display;
      event.window = _xwindow;
      event.message_type = x11_pipe->_net_wm_state;
      event.format = 32;
      event.data.l[0] = set_data[i]._action;
      event.data.l[1] = set_data[i]._state;
      event.data.l[2] = 0;
      event.data.l[3] = 1;

      XSendEvent(_display, x11_pipe->get_root(), True, SubstructureNotifyMask | SubstructureRedirectMask, (XEvent *)&event);
    }
  }

  XSetWMProperties(_display, _xwindow, window_name_p, window_name_p,
                   nullptr, 0, size_hints_p, wm_hints_p, class_hints_p);

  if (size_hints_p != nullptr) {
    XFree(size_hints_p);
  }
  if (wm_hints_p != nullptr) {
    XFree(wm_hints_p);
  }
  if (class_hints_p != nullptr) {
    XFree(class_hints_p);
  }

  // Also, indicate to the window manager that we'd like to get a chance to
  // close our windows cleanly, rather than being rudely disconnected from the
  // X server if the user requests a window close.
  Atom protocols[] = {
    _wm_delete_window,
  };

  XSetWMProtocols(_display, _xwindow, protocols,
                  sizeof(protocols) / sizeof(Atom));
}

/**
 * Allocates a colormap appropriate to the visual and stores in in the
 * _colormap method.
 */
void x11GraphicsWindow::
setup_colormap(XVisualInfo *visual) {
  x11GraphicsPipe *x11_pipe;
  DCAST_INTO_V(x11_pipe, _pipe);
  X11_Window root_window = x11_pipe->get_root();

  _colormap = XCreateColormap(_display, root_window,
                              visual->visual, AllocNone);
}

/**
 * Adds raw mice to the _input_devices list.
 * @deprecated obtain raw devices via the device manager instead.
 */
void x11GraphicsWindow::
open_raw_mice() {
#ifdef PHAVE_LINUX_INPUT_H
  bool any_present = false;
  bool any_mice = false;

  for (int i=0; i<64; i++) {
    ostringstream fnb;
    fnb << "/dev/input/event" << i;
    string fn = fnb.str();
    int fd = open(fn.c_str(), O_RDONLY | O_NONBLOCK, 0);
    if (fd >= 0) {
      EvdevInputDevice *device = new EvdevInputDevice(nullptr, fd);
      nassertd(device != NULL) continue;

      if (device->has_pointer()) {
        add_input_device(device);

        x11display_cat.info()
          << "Raw mouse " << _input_devices.size()
          << " detected: " << device->get_name() << "\n";

        any_mice = true;
        any_present = true;
      }
    } else {
      if (errno == ENOENT || errno == ENOTDIR) {
        break;
      } else {
        any_present = true;
        x11display_cat.error()
          << "Opening raw mice: " << strerror(errno) << " " << fn << "\n";
      }
    }
  }

  if (any_mice) {
    _properties.set_raw_mice(true);

  } else if (!any_present) {
    x11display_cat.error() <<
      "Opening raw mice: files not found: /dev/input/event*\n";

  } else {
    x11display_cat.error() <<
      "Opening raw mice: no mouse devices detected in /dev/input/event*\n";
  }
#else
  x11display_cat.error() <<
    "Opening raw mice: panda not compiled with raw mouse support.\n";
#endif
}

/**
 * Generates a keystroke corresponding to the indicated X KeyPress event.
 */
void x11GraphicsWindow::
handle_keystroke(XKeyEvent &event) {
  if (!_dga_mouse_enabled) {
    _input->set_pointer_in_window(event.x, event.y);
  }

  if (_ic) {
    // First, get the keystroke as a wide-character sequence.
    static const int buffer_size = 256;
    wchar_t buffer[buffer_size];
    Status status;
    int len = XwcLookupString(_ic, &event, buffer, buffer_size, nullptr,
                              &status);
    if (status == XBufferOverflow) {
      x11display_cat.error()
        << "Overflowed input buffer.\n";
    }

    // Now each of the returned wide characters represents a keystroke.
    for (int i = 0; i < len; i++) {
      _input->keystroke(buffer[i]);
    }

  } else {
    // Without an input context, just get the ascii keypress.
    ButtonHandle button = get_button(event, true);
    if (button.has_ascii_equivalent()) {
      _input->keystroke(button.get_ascii_equivalent());
    }
  }
}

/**
 * Generates a keypress corresponding to the indicated X KeyPress event.
 */
void x11GraphicsWindow::
handle_keypress(XKeyEvent &event) {
  if (!_dga_mouse_enabled) {
    _input->set_pointer_in_window(event.x, event.y);
  }

  // Now get the raw unshifted button.
  ButtonHandle button = get_button(event, false);
  if (button != ButtonHandle::none()) {
    if (button == KeyboardButton::lcontrol() || button == KeyboardButton::rcontrol()) {
      _input->button_down(KeyboardButton::control());
    }
    if (button == KeyboardButton::lshift() || button == KeyboardButton::rshift()) {
      _input->button_down(KeyboardButton::shift());
    }
    if (button == KeyboardButton::lalt() || button == KeyboardButton::ralt()) {
      _input->button_down(KeyboardButton::alt());
    }
    if (button == KeyboardButton::lmeta() || button == KeyboardButton::rmeta()) {
      _input->button_down(KeyboardButton::meta());
    }
    _input->button_down(button);
  }

  if (event.keycode >= 9 && event.keycode <= 135) {
    ButtonHandle raw_button = map_raw_button(event.keycode);
    if (raw_button != ButtonHandle::none()) {
      _input->raw_button_down(raw_button);
    }
  }
}

/**
 * Generates a keyrelease corresponding to the indicated X KeyRelease event.
 */
void x11GraphicsWindow::
handle_keyrelease(XKeyEvent &event) {
  if (!_dga_mouse_enabled) {
    _input->set_pointer_in_window(event.x, event.y);
  }

  // Now get the raw unshifted button.
  ButtonHandle button = get_button(event, false);
  if (button != ButtonHandle::none()) {
    if (button == KeyboardButton::lcontrol() || button == KeyboardButton::rcontrol()) {
      _input->button_up(KeyboardButton::control());
    }
    if (button == KeyboardButton::lshift() || button == KeyboardButton::rshift()) {
      _input->button_up(KeyboardButton::shift());
    }
    if (button == KeyboardButton::lalt() || button == KeyboardButton::ralt()) {
      _input->button_up(KeyboardButton::alt());
    }
    if (button == KeyboardButton::lmeta() || button == KeyboardButton::rmeta()) {
      _input->button_up(KeyboardButton::meta());
    }
    _input->button_up(button);
  }

  if (event.keycode >= 9 && event.keycode <= 135) {
    ButtonHandle raw_button = map_raw_button(event.keycode);
    if (raw_button != ButtonHandle::none()) {
      _input->raw_button_up(raw_button);
    }
  }
}

/**
 * Returns the Panda ButtonHandle corresponding to the keyboard button
 * indicated by the given key event.
 */
ButtonHandle x11GraphicsWindow::
get_button(XKeyEvent &key_event, bool allow_shift) {
  KeySym key = XLookupKeysym(&key_event, 0);

  if ((key_event.state & Mod2Mask) != 0) {
    // Mod2Mask corresponds to NumLock being in effect.  In this case, we want
    // to get the alternate keysym associated with any keypad keys.  Weird
    // system.
    KeySym k2;
    ButtonHandle button;
    switch (key) {
    case XK_KP_Space:
    case XK_KP_Tab:
    case XK_KP_Enter:
    case XK_KP_F1:
    case XK_KP_F2:
    case XK_KP_F3:
    case XK_KP_F4:
    case XK_KP_Equal:
    case XK_KP_Multiply:
    case XK_KP_Add:
    case XK_KP_Separator:
    case XK_KP_Subtract:
    case XK_KP_Divide:
    case XK_KP_Left:
    case XK_KP_Up:
    case XK_KP_Right:
    case XK_KP_Down:
    case XK_KP_Begin:
    case XK_KP_Prior:
    case XK_KP_Next:
    case XK_KP_Home:
    case XK_KP_End:
    case XK_KP_Insert:
    case XK_KP_Delete:
    case XK_KP_0:
    case XK_KP_1:
    case XK_KP_2:
    case XK_KP_3:
    case XK_KP_4:
    case XK_KP_5:
    case XK_KP_6:
    case XK_KP_7:
    case XK_KP_8:
    case XK_KP_9:
      k2 = XLookupKeysym(&key_event, 1);
      button = map_button(k2);
      if (button != ButtonHandle::none()) {
        return button;
      }
      // If that didn't produce a button we know, just fall through and handle
      // the normal, un-numlocked key.
      break;

    default:
      break;
    }
  }

  if (allow_shift) {
    // If shift is held down, get the shifted keysym.
    if ((key_event.state & ShiftMask) != 0) {
      KeySym k2 = XLookupKeysym(&key_event, 1);
      ButtonHandle button = map_button(k2);
      if (button != ButtonHandle::none()) {
        return button;
      }
    }

    // If caps lock is down, shift lowercase letters to uppercase.  We can do
    // this in just the ASCII set, because we handle international keyboards
    // elsewhere (via an input context).
    if ((key_event.state & (ShiftMask | LockMask)) != 0) {
      if (key >= XK_a && key <= XK_z) {
        key += (XK_A - XK_a);
      }
    }
  }

  return map_button(key);
}

/**
 * Maps from a single X keysym to Panda's ButtonHandle.  Called by
 * get_button(), above.
 */
ButtonHandle x11GraphicsWindow::
map_button(KeySym key) const {
  switch (key) {
  case NoSymbol:
    return ButtonHandle::none();
  case XK_BackSpace:
    return KeyboardButton::backspace();
  case XK_Tab:
  case XK_KP_Tab:
    return KeyboardButton::tab();
  case XK_Return:
  case XK_KP_Enter:
    return KeyboardButton::enter();
  case XK_Escape:
    return KeyboardButton::escape();
  case XK_KP_Space:
  case XK_space:
    return KeyboardButton::space();
  case XK_exclam:
    return KeyboardButton::ascii_key('!');
  case XK_quotedbl:
    return KeyboardButton::ascii_key('"');
  case XK_numbersign:
    return KeyboardButton::ascii_key('#');
  case XK_dollar:
    return KeyboardButton::ascii_key('$');
  case XK_percent:
    return KeyboardButton::ascii_key('%');
  case XK_ampersand:
    return KeyboardButton::ascii_key('&');
  case XK_apostrophe: // == XK_quoteright
  case XK_dead_acute: // on int'l keyboards
    return KeyboardButton::ascii_key('\'');
  case XK_parenleft:
    return KeyboardButton::ascii_key('(');
  case XK_parenright:
    return KeyboardButton::ascii_key(')');
  case XK_asterisk:
  case XK_KP_Multiply:
    return KeyboardButton::ascii_key('*');
  case XK_plus:
  case XK_KP_Add:
    return KeyboardButton::ascii_key('+');
  case XK_comma:
  case XK_KP_Separator:
    return KeyboardButton::ascii_key(',');
  case XK_minus:
  case XK_KP_Subtract:
    return KeyboardButton::ascii_key('-');
  case XK_period:
  case XK_KP_Decimal:
    return KeyboardButton::ascii_key('.');
  case XK_slash:
  case XK_KP_Divide:
    return KeyboardButton::ascii_key('/');
  case XK_0:
  case XK_KP_0:
    return KeyboardButton::ascii_key('0');
  case XK_1:
  case XK_KP_1:
    return KeyboardButton::ascii_key('1');
  case XK_2:
  case XK_KP_2:
    return KeyboardButton::ascii_key('2');
  case XK_3:
  case XK_KP_3:
    return KeyboardButton::ascii_key('3');
  case XK_4:
  case XK_KP_4:
    return KeyboardButton::ascii_key('4');
  case XK_5:
  case XK_KP_5:
    return KeyboardButton::ascii_key('5');
  case XK_6:
  case XK_KP_6:
    return KeyboardButton::ascii_key('6');
  case XK_7:
  case XK_KP_7:
    return KeyboardButton::ascii_key('7');
  case XK_8:
  case XK_KP_8:
    return KeyboardButton::ascii_key('8');
  case XK_9:
  case XK_KP_9:
    return KeyboardButton::ascii_key('9');
  case XK_colon:
    return KeyboardButton::ascii_key(':');
  case XK_semicolon:
    return KeyboardButton::ascii_key(';');
  case XK_less:
    return KeyboardButton::ascii_key('<');
  case XK_equal:
  case XK_KP_Equal:
    return KeyboardButton::ascii_key('=');
  case XK_greater:
    return KeyboardButton::ascii_key('>');
  case XK_question:
    return KeyboardButton::ascii_key('?');
  case XK_at:
    return KeyboardButton::ascii_key('@');
  case XK_A:
    return KeyboardButton::ascii_key('A');
  case XK_B:
    return KeyboardButton::ascii_key('B');
  case XK_C:
    return KeyboardButton::ascii_key('C');
  case XK_D:
    return KeyboardButton::ascii_key('D');
  case XK_E:
    return KeyboardButton::ascii_key('E');
  case XK_F:
    return KeyboardButton::ascii_key('F');
  case XK_G:
    return KeyboardButton::ascii_key('G');
  case XK_H:
    return KeyboardButton::ascii_key('H');
  case XK_I:
    return KeyboardButton::ascii_key('I');
  case XK_J:
    return KeyboardButton::ascii_key('J');
  case XK_K:
    return KeyboardButton::ascii_key('K');
  case XK_L:
    return KeyboardButton::ascii_key('L');
  case XK_M:
    return KeyboardButton::ascii_key('M');
  case XK_N:
    return KeyboardButton::ascii_key('N');
  case XK_O:
    return KeyboardButton::ascii_key('O');
  case XK_P:
    return KeyboardButton::ascii_key('P');
  case XK_Q:
    return KeyboardButton::ascii_key('Q');
  case XK_R:
    return KeyboardButton::ascii_key('R');
  case XK_S:
    return KeyboardButton::ascii_key('S');
  case XK_T:
    return KeyboardButton::ascii_key('T');
  case XK_U:
    return KeyboardButton::ascii_key('U');
  case XK_V:
    return KeyboardButton::ascii_key('V');
  case XK_W:
    return KeyboardButton::ascii_key('W');
  case XK_X:
    return KeyboardButton::ascii_key('X');
  case XK_Y:
    return KeyboardButton::ascii_key('Y');
  case XK_Z:
    return KeyboardButton::ascii_key('Z');
  case XK_bracketleft:
    return KeyboardButton::ascii_key('[');
  case XK_backslash:
    return KeyboardButton::ascii_key('\\');
  case XK_bracketright:
    return KeyboardButton::ascii_key(']');
  case XK_asciicircum:
    return KeyboardButton::ascii_key('^');
  case XK_underscore:
    return KeyboardButton::ascii_key('_');
  case XK_grave: // == XK_quoteleft
  case XK_dead_grave: // on int'l keyboards
    return KeyboardButton::ascii_key('`');
  case XK_a:
    return KeyboardButton::ascii_key('a');
  case XK_b:
    return KeyboardButton::ascii_key('b');
  case XK_c:
    return KeyboardButton::ascii_key('c');
  case XK_d:
    return KeyboardButton::ascii_key('d');
  case XK_e:
    return KeyboardButton::ascii_key('e');
  case XK_f:
    return KeyboardButton::ascii_key('f');
  case XK_g:
    return KeyboardButton::ascii_key('g');
  case XK_h:
    return KeyboardButton::ascii_key('h');
  case XK_i:
    return KeyboardButton::ascii_key('i');
  case XK_j:
    return KeyboardButton::ascii_key('j');
  case XK_k:
    return KeyboardButton::ascii_key('k');
  case XK_l:
    return KeyboardButton::ascii_key('l');
  case XK_m:
    return KeyboardButton::ascii_key('m');
  case XK_n:
    return KeyboardButton::ascii_key('n');
  case XK_o:
    return KeyboardButton::ascii_key('o');
  case XK_p:
    return KeyboardButton::ascii_key('p');
  case XK_q:
    return KeyboardButton::ascii_key('q');
  case XK_r:
    return KeyboardButton::ascii_key('r');
  case XK_s:
    return KeyboardButton::ascii_key('s');
  case XK_t:
    return KeyboardButton::ascii_key('t');
  case XK_u:
    return KeyboardButton::ascii_key('u');
  case XK_v:
    return KeyboardButton::ascii_key('v');
  case XK_w:
    return KeyboardButton::ascii_key('w');
  case XK_x:
    return KeyboardButton::ascii_key('x');
  case XK_y:
    return KeyboardButton::ascii_key('y');
  case XK_z:
    return KeyboardButton::ascii_key('z');
  case XK_braceleft:
    return KeyboardButton::ascii_key('{');
  case XK_bar:
    return KeyboardButton::ascii_key('|');
  case XK_braceright:
    return KeyboardButton::ascii_key('}');
  case XK_asciitilde:
    return KeyboardButton::ascii_key('~');
  case XK_F1:
  case XK_KP_F1:
    return KeyboardButton::f1();
  case XK_F2:
  case XK_KP_F2:
    return KeyboardButton::f2();
  case XK_F3:
  case XK_KP_F3:
    return KeyboardButton::f3();
  case XK_F4:
  case XK_KP_F4:
    return KeyboardButton::f4();
  case XK_F5:
    return KeyboardButton::f5();
  case XK_F6:
    return KeyboardButton::f6();
  case XK_F7:
    return KeyboardButton::f7();
  case XK_F8:
    return KeyboardButton::f8();
  case XK_F9:
    return KeyboardButton::f9();
  case XK_F10:
    return KeyboardButton::f10();
  case XK_F11:
    return KeyboardButton::f11();
  case XK_F12:
    return KeyboardButton::f12();
  case XK_KP_Left:
  case XK_Left:
    return KeyboardButton::left();
  case XK_KP_Up:
  case XK_Up:
    return KeyboardButton::up();
  case XK_KP_Right:
  case XK_Right:
    return KeyboardButton::right();
  case XK_KP_Down:
  case XK_Down:
    return KeyboardButton::down();
  case XK_KP_Prior:
  case XK_Prior:
    return KeyboardButton::page_up();
  case XK_KP_Next:
  case XK_Next:
    return KeyboardButton::page_down();
  case XK_KP_Home:
  case XK_Home:
    return KeyboardButton::home();
  case XK_KP_End:
  case XK_End:
    return KeyboardButton::end();
  case XK_KP_Insert:
  case XK_Insert:
    return KeyboardButton::insert();
  case XK_KP_Delete:
  case XK_Delete:
    return KeyboardButton::del();
  case XK_Num_Lock:
    return KeyboardButton::num_lock();
  case XK_Scroll_Lock:
    return KeyboardButton::scroll_lock();
  case XK_Print:
    return KeyboardButton::print_screen();
  case XK_Pause:
    return KeyboardButton::pause();
  case XK_Menu:
    return KeyboardButton::menu();
  case XK_Shift_L:
    return KeyboardButton::lshift();
  case XK_Shift_R:
    return KeyboardButton::rshift();
  case XK_Control_L:
    return KeyboardButton::lcontrol();
  case XK_Control_R:
    return KeyboardButton::rcontrol();
  case XK_Alt_L:
    return KeyboardButton::lalt();
  case XK_Alt_R:
    return KeyboardButton::ralt();
  case XK_Meta_L:
  case XK_Super_L:
    return KeyboardButton::lmeta();
  case XK_Meta_R:
  case XK_Super_R:
    return KeyboardButton::rmeta();
  case XK_Caps_Lock:
    return KeyboardButton::caps_lock();
  case XK_Shift_Lock:
    return KeyboardButton::shift_lock();
  }
  if (x11display_cat.is_debug()) {
    x11display_cat.debug()
      << "Unrecognized keysym 0x" << std::hex << key << std::dec << "\n";
  }
  return ButtonHandle::none();
}

/**
 * Maps from a single X keycode to Panda's ButtonHandle.
 */
ButtonHandle x11GraphicsWindow::
map_raw_button(KeyCode key) const {
#ifdef PHAVE_LINUX_INPUT_H
  // Most X11 servers are configured to use the evdev driver, which
  // adds 8 to the underlying evdev keycodes (not sure why).
  // In any case, this means we can use the same mapping as our raw
  // input code, which uses evdev directly.
  int index = key - 8;
  if (index >= 0) {
    return EvdevInputDevice::map_button(index);
  }
#endif
  return ButtonHandle::none();
}

/**
 * Returns the Panda ButtonHandle corresponding to the mouse button indicated
 * by the given button event.
 */
ButtonHandle x11GraphicsWindow::
get_mouse_button(XButtonEvent &button_event) {
  int index = button_event.button;
  if (index == x_wheel_up_button) {
    return MouseButton::wheel_up();
  } else if (index == x_wheel_down_button) {
    return MouseButton::wheel_down();
  } else if (index == x_wheel_left_button) {
    return MouseButton::wheel_left();
  } else if (index == x_wheel_right_button) {
    return MouseButton::wheel_right();
  } else {
    return MouseButton::button(index - 1);
  }
}

/**
 * Returns a ButtonMap containing the association between raw buttons and
 * virtual buttons.
 */
ButtonMap *x11GraphicsWindow::
get_keyboard_map() const {
  // NB.  This could be improved by using the Xkb API. XkbDescPtr desc =
  // XkbGetMap(_display, XkbAllMapComponentsMask, XkbUseCoreKbd);
  ButtonMap *map = new ButtonMap;

  LightReMutexHolder holder(x11GraphicsPipe::_x_mutex);

  for (int k = 9; k <= 135; ++k) {
    ButtonHandle raw_button = map_raw_button(k);
    if (raw_button == ButtonHandle::none()) {
      continue;
    }

    KeySym sym = XkbKeycodeToKeysym(_display, k, 0, 0);
    ButtonHandle button = map_button(sym);
    if (button == ButtonHandle::none()) {
      continue;
    }

    map->map_button(raw_button, button);
  }

  return map;
}

/**
 * This function is used as a predicate to XCheckIfEvent() to determine if the
 * indicated queued X event is relevant and should be returned to this window.
 */
Bool x11GraphicsWindow::
check_event(X11_Display *display, XEvent *event, char *arg) {
  const x11GraphicsWindow *self = (x11GraphicsWindow *)arg;

  // We accept any event that is sent to our window.
  return (event->xany.window == self->_xwindow);
}

/**
 * Loads and returns a Cursor corresponding to the indicated filename.  If the
 * file cannot be loaded, returns None.
 */
X11_Cursor x11GraphicsWindow::
get_cursor(const Filename &filename) {
  x11GraphicsPipe *x11_pipe;
  DCAST_INTO_R(x11_pipe, _pipe, None);

  if (x11_pipe->_xcursor_size == -1) {
    x11display_cat.info()
      << "libXcursor.so.1 not available; cannot change mouse cursor.\n";
    return None;
  }

  // First, look for the unresolved filename in our index.
  pmap<Filename, X11_Cursor>::iterator fi = _cursor_filenames.find(filename);
  if (fi != _cursor_filenames.end()) {
    return fi->second;
  }

  // If it wasn't found, resolve the filename and search for that.
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  Filename resolved (filename);
  if (!vfs->resolve_filename(resolved, get_model_path())) {
    // The filename doesn't exist.
    x11display_cat.warning()
      << "Could not find cursor filename " << filename << "\n";
    return None;
  }
  fi = _cursor_filenames.find(resolved);
  if (fi != _cursor_filenames.end()) {
    return fi->second;
  }

  // Open the file through the virtual file system.
  istream *str = vfs->open_read_file(resolved, true);
  if (str == nullptr) {
    x11display_cat.warning()
      << "Could not open cursor file " << filename << "\n";
    return None;
  }

  // Check the first four bytes to see what kind of file it is.
  char magic[4];
  str->read(magic, 4);
  if (!str->good()) {
    x11display_cat.warning()
      << "Could not read from cursor file " << filename << "\n";
    return None;
  }

  // Put back the read bytes. Do not use seekg, because this will
  // corrupt the stream if it points to encrypted/compressed file
  str->putback(magic[3]);
  str->putback(magic[2]);
  str->putback(magic[1]);
  str->putback(magic[0]);

  X11_Cursor h = None;
  if (memcmp(magic, "Xcur", 4) == 0) {
    // X11 cursor.
    x11display_cat.debug()
      << "Loading X11 cursor " << filename << "\n";
    XcursorFile xcfile;
    xcfile.closure = str;
    xcfile.read = &xcursor_read;
    xcfile.write = &xcursor_write;
    xcfile.seek = &xcursor_seek;

    XcursorImages *images = x11_pipe->_XcursorXcFileLoadImages(&xcfile, x11_pipe->_xcursor_size);
    if (images != nullptr) {
      h = x11_pipe->_XcursorImagesLoadCursor(_display, images);
      x11_pipe->_XcursorImagesDestroy(images);
    }

  } else if (memcmp(magic, "\0\0\1\0", 4) == 0
          || memcmp(magic, "\0\0\2\0", 4) == 0) {
    // Windows .ico or .cur file.
    x11display_cat.debug()
      << "Loading Windows cursor " << filename << "\n";
    h = read_ico(*str);
  }

  // Delete the istream.
  vfs->close_read_file(str);

  if (h == None) {
    x11display_cat.warning()
      << "X11 cursor filename '" << resolved << "' could not be loaded!\n";
  }

  _cursor_filenames[resolved] = h;
  return h;
}

/**
 * Reads a Windows .ico or .cur file from the indicated stream and returns it
 * as an X11 Cursor.  If the file cannot be loaded, returns None.
 */
X11_Cursor x11GraphicsWindow::
read_ico(istream &ico) {
  x11GraphicsPipe *x11_pipe;
  DCAST_INTO_R(x11_pipe, _pipe, None);

  // Local structs, this is just POD, make input easier
  typedef struct {
    uint16_t reserved, type, count;
  } IcoHeader;

  typedef struct {
    uint8_t width, height, colorCount, reserved;
    uint16_t xhot, yhot;
    uint32_t bitmapSize, offset;
  } IcoEntry;

  typedef struct {
    uint32_t headerSize, width, height;
    uint16_t planes, bitsPerPixel;
    uint32_t compression, imageSize, xPixelsPerM, yPixelsPerM, colorsUsed, colorsImportant;
  } IcoInfoHeader;

  typedef struct {
    uint8_t blue, green, red, reserved;
  } IcoColor;

  int i, entry = 0;
  unsigned int j, k, mask, shift;
  size_t colorCount, bitsPerPixel;
  IcoHeader header;
  IcoInfoHeader infoHeader;
  IcoEntry *entries = nullptr;
  IcoColor color, *palette = nullptr;

  size_t xorBmpSize, andBmpSize;
  char *curXor, *curAnd;
  char *xorBmp = nullptr, *andBmp = nullptr;
  XcursorImage *image = nullptr;
  X11_Cursor ret = None;

  int def_size = x11_pipe->_xcursor_size;

  // Get our header, note that ICO = type 1 and CUR = type 2.
  ico.read(reinterpret_cast<char *>(&header), sizeof(IcoHeader));
  if (!ico.good()) goto cleanup;
  if (header.type != 1 && header.type != 2) goto cleanup;
  if (header.count < 1) goto cleanup;

  // Read the entry table into memory, select the largest entry.
  entries = new IcoEntry[header.count];
  ico.read(reinterpret_cast<char *>(entries), header.count * sizeof(IcoEntry));
  if (!ico.good()) goto cleanup;
  for (i = 1; i < header.count; i++) {
    if (entries[i].width == def_size && entries[i].height == def_size) {
      // Wait, this is the default cursor size.  This is perfect.
      entry = i;
      break;
    }
    if (entries[i].width > entries[entry].width ||
        entries[i].height > entries[entry].height)
        entry = i;
  }

  // Seek to the image in the ICO.
  ico.seekg(entries[entry].offset);
  if (!ico.good()) goto cleanup;

  if (ico.peek() == 0x89) {
    // Hang on, this is actually a PNG header.
    PNMImage img;
    PNMFileTypeRegistry *reg = PNMFileTypeRegistry::get_global_ptr();
    if (!img.read(ico, "", reg->get_type_from_extension("png"))) {
      goto cleanup;
    }
    img.set_maxval(255);

    image = x11_pipe->_XcursorImageCreate(img.get_x_size(), img.get_y_size());

    xel *ptr = img.get_array();
    xelval *alpha = img.get_alpha_array();
    size_t num_pixels = (size_t)img.get_x_size() * (size_t)img.get_y_size();
    unsigned int *dest = image->pixels;

    if (alpha != nullptr) {
      for (size_t p = 0; p < num_pixels; ++p) {
        *dest++ = (*alpha << 24U) | (ptr->r << 16U) | (ptr->g << 8U) | (ptr->b);
        ++ptr;
        ++alpha;
      }
    } else {
      for (size_t p = 0; p < num_pixels; ++p) {
        *dest++ = 0xff000000U | (ptr->r << 16U) | (ptr->g << 8U) | (ptr->b);
        ++ptr;
      }
    }

  } else {
    ico.read(reinterpret_cast<char *>(&infoHeader), sizeof(IcoInfoHeader));
    if (!ico.good()) goto cleanup;
    bitsPerPixel = infoHeader.bitsPerPixel;

    if (infoHeader.compression != 0) goto cleanup;

    // Load the color palette, if one exists.
    if (bitsPerPixel != 24 && bitsPerPixel != 32) {
      colorCount = 1 << bitsPerPixel;
      palette = new IcoColor[colorCount];
      ico.read(reinterpret_cast<char *>(palette), colorCount * sizeof(IcoColor));
      if (!ico.good()) goto cleanup;
    }

    int and_stride = ((infoHeader.width >> 3) + 3) & ~0x03;

    // Read in the pixel data.
    xorBmpSize = (infoHeader.width * (infoHeader.height / 2) * bitsPerPixel) / 8;
    andBmpSize = and_stride * (infoHeader.height / 2);
    curXor = xorBmp = new char[xorBmpSize];
    curAnd = andBmp = new char[andBmpSize];
    ico.read(xorBmp, xorBmpSize);
    if (!ico.good()) goto cleanup;
    ico.read(andBmp, andBmpSize);
    if (!ico.good()) goto cleanup;

    image = x11_pipe->_XcursorImageCreate(infoHeader.width, infoHeader.height / 2);

    // Support all the formats that GIMP supports.
    switch (bitsPerPixel) {
    case 1:
    case 4:
    case 8:
      // For colors less that a byte wide, shift and mask the palette indices
      // off each element of the xorBmp and append them to the image.
      mask = ((1 << bitsPerPixel) - 1);
      for (i = image->height - 1; i >= 0; i--) {
        for (j = 0; j < image->width; j += 8 / bitsPerPixel) {
          for (k = 0; k < 8 / bitsPerPixel; k++) {
            shift = 8 - ((k + 1) * bitsPerPixel);
            color = palette[(*curXor & (mask << shift)) >> shift];
            image->pixels[(i * image->width) + j + k] = (color.red << 16) +
                                                        (color.green << 8) +
                                                        (color.blue);
          }

          curXor++;
        }

        // Set the alpha byte properly according to the andBmp.
        for (j = 0; j < image->width; j += 8) {
          for (k = 0; k < 8; k++) {
            shift = 7 - k;
            image->pixels[(i * image->width) + j + k] |=
              ((*curAnd & (1 << shift)) >> shift) ? 0x0 : (0xff << 24);
          }

          curAnd++;
        }
      }
      break;

    case 24:
      // Pack each of the three bytes into a single color, BGR -> 0RGB
      for (i = image->height - 1; i >= 0; i--) {
        for (j = 0; j < image->width; j++) {
          shift = 7 - (j & 0x7);
          uint32_t alpha = (curAnd[j >> 3] & (1 << shift)) ? 0 : 0xff000000U;
          image->pixels[(i * image->width) + j] = (uint8_t)curXor[0]
                                                | ((uint8_t)curXor[1] << 8u)
                                                | ((uint8_t)curXor[2] << 16u)
                                                | alpha;
          curXor += 3;
        }
        curAnd += and_stride;
      }
      break;

    case 32:
      // Pack each of the four bytes into a single color, BGRA -> ARGB
      for (i = image->height - 1; i >= 0; i--) {
        for (j = 0; j < image->width; j++) {
          image->pixels[(i * image->width) + j] = (*(curXor + 3) << 24) +
                                                  (*(curXor + 2) << 16) +
                                                  (*(curXor + 1) << 8) +
                                                  (*curXor);
          curXor += 4;
        }
      }
      break;

    default:
      goto cleanup;
    }
  }

  // If this is an actual CUR not an ICO set up the hotspot properly.
  if (header.type == 2) {
    image->xhot = entries[entry].xhot;
    image->yhot = entries[entry].yhot;
  } else {
    image->xhot = 0;
    image->yhot = 0;
  }

  ret = x11_pipe->_XcursorImageLoadCursor(_display, image);

cleanup:
  x11_pipe->_XcursorImageDestroy(image);
  delete[] entries;
  delete[] palette;
  delete[] xorBmp;
  delete[] andBmp;

  return ret;
}
