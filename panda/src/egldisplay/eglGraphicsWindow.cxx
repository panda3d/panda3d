// Filename: eglGraphicsWindow.cxx
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

#include <errno.h>
#include <sys/time.h>

#ifdef HAVE_LINUX_INPUT_H
#include <linux/input.h>
#endif

TypeHandle eglGraphicsWindow::_type_handle;

#define test_bit(bit, array) ((array)[(bit)/8] & (1<<((bit)&7)))

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
eglGraphicsWindow::
eglGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                  const string &name,
                  const FrameBufferProperties &fb_prop,
                  const WindowProperties &win_prop,
                  int flags,
                  GraphicsStateGuardian *gsg,
                  GraphicsOutput *host) :
  GraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  eglGraphicsPipe *egl_pipe;
  DCAST_INTO_V(egl_pipe, _pipe);
  _display = egl_pipe->get_display();
  _screen = egl_pipe->get_screen();
  _xwindow = (X11_Window)NULL;
  _ic = (XIC)NULL;
  _egl_display = egl_pipe->_egl_display;
  _egl_surface = 0;
  _awaiting_configure = false;
  _wm_delete_window = egl_pipe->_wm_delete_window;
  _net_wm_window_type = egl_pipe->_net_wm_window_type;
  _net_wm_window_type_splash = egl_pipe->_net_wm_window_type_splash;
  _net_wm_window_type_fullscreen = egl_pipe->_net_wm_window_type_fullscreen;
  _net_wm_state = egl_pipe->_net_wm_state;
  _net_wm_state_fullscreen = egl_pipe->_net_wm_state_fullscreen;
  _net_wm_state_above = egl_pipe->_net_wm_state_above;
  _net_wm_state_below = egl_pipe->_net_wm_state_below;
  _net_wm_state_add = egl_pipe->_net_wm_state_add;
  _net_wm_state_remove = egl_pipe->_net_wm_state_remove;

  GraphicsWindowInputDevice device =
    GraphicsWindowInputDevice::pointer_and_keyboard(this, "keyboard_mouse");
  add_input_device(device);
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
eglGraphicsWindow::
~eglGraphicsWindow() {
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::move_pointer
//       Access: Published, Virtual
//  Description: Forces the pointer to the indicated position within
//               the window, if possible.
//
//               Returns true if successful, false on failure.  This
//               may fail if the mouse is not currently within the
//               window, or if the API doesn't support this operation.
////////////////////////////////////////////////////////////////////
bool eglGraphicsWindow::
move_pointer(int device, int x, int y) {
  // Note: this is not thread-safe; it should be called only from App.
  // Probably not an issue.
  if (device == 0) {
    // Move the system mouse pointer.
    if (!_properties.get_foreground() ||
        !_input_devices[0].get_pointer().get_in_window()) {
      // If the window doesn't have input focus, or the mouse isn't
      // currently within the window, forget it.
      return false;
    }

    const MouseData &md = _input_devices[0].get_pointer();
    if (!md.get_in_window() || md.get_x() != x || md.get_y() != y) {
      XWarpPointer(_display, None, _xwindow, 0, 0, 0, 0, x, y);
      _input_devices[0].set_pointer_in_window(x, y);
    }
    return true;
  } else {
    // Move a raw mouse.
    if ((device < 1)||(device >= _input_devices.size())) {
      return false;
    }
    _input_devices[device].set_pointer_in_window(x, y);
    return true;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool eglGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  PStatTimer timer(_make_current_pcollector, current_thread);

  begin_frame_spam(mode);
  if (_gsg == (GraphicsStateGuardian *)NULL) {
    return false;
  }
  if (_awaiting_configure) {
    // Don't attempt to draw while we have just reconfigured the
    // window and we haven't got the notification back yet.
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
      // No need to make the context current again.  Short-circuit
      // this possibly-expensive call.
    } else {
      // Need to set the context.
      if (!eglMakeCurrent(_egl_display, _egl_surface, _egl_surface, eglgsg->_context)) {
        egldisplay_cat.error() << "Failed to call eglMakeCurrent: "
          << get_egl_error_string(eglGetError()) << "\n";
      }
    }
  }

  // Now that we have made the context current to a window, we can
  // reset the GSG state if this is the first time it has been used.
  // (We can't just call reset() when we construct the GSG, because
  // reset() requires having a current context.)
  eglgsg->reset_if_new();

  if (mode == FM_render) {
    // begin_render_texture();
    clear_cube_map_selection();
  }

  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void eglGraphicsWindow::
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
//     Function: eglGraphicsWindow::end_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after begin_flip() has been called on all windows, to
//               finish the exchange of the front and back buffers.
//
//               This should cause the window to wait for the flip, if
//               necessary.
////////////////////////////////////////////////////////////////////
void eglGraphicsWindow::
end_flip() {
  if (_gsg != (GraphicsStateGuardian *)NULL && _flip_ready) {

    // It doesn't appear to be necessary to ensure the graphics
    // context is current before flipping the windows, and insisting
    // on doing so can be a significant performance hit.

    //make_current();

    LightReMutexHolder holder(eglGraphicsPipe::_x_mutex);
    eglSwapBuffers(_egl_display, _egl_surface);
  }
  GraphicsWindow::end_flip();
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::process_events
//       Access: Public, Virtual
//  Description: Do whatever processing is necessary to ensure that
//               the window responds to user events.  Also, honor any
//               requests recently made via request_properties()
//
//               This function is called only within the window
//               thread.
////////////////////////////////////////////////////////////////////
void eglGraphicsWindow::
process_events() {
  LightReMutexHolder holder(eglGraphicsPipe::_x_mutex);

  GraphicsWindow::process_events();

  if (_xwindow == (X11_Window)0) {
    return;
  }

  poll_raw_mice();

  XEvent event;
  XKeyEvent keyrelease_event;
  bool got_keyrelease_event = false;

  while (XCheckIfEvent(_display, &event, check_event, (char *)this)) {
    if (XFilterEvent(&event, None)) {
      continue;
    }

    if (got_keyrelease_event) {
      // If a keyrelease event is immediately followed by a matching
      // keypress event, that's just key repeat and we should treat
      // the two events accordingly.  It would be nice if X provided a
      // way to differentiate between keyrepeat and explicit
      // keypresses more generally.
      got_keyrelease_event = false;

      if (event.type == KeyPress &&
          event.xkey.keycode == keyrelease_event.keycode &&
          (event.xkey.time - keyrelease_event.time <= 1)) {
        // In particular, we only generate down messages for the
        // repeated keys, not down-and-up messages.
        handle_keystroke(event.xkey);

        // We thought about not generating the keypress event, but we
        // need that repeat for backspace.  Rethink later.
        handle_keypress(event.xkey);
        continue;

      } else {
        // This keyrelease event is not immediately followed by a
        // matching keypress event, so it's a genuine release.
        handle_keyrelease(keyrelease_event);
      }
    }

    WindowProperties properties;
    ButtonHandle button;

    switch (event.type) {
    case ReparentNotify:
      break;

    case ConfigureNotify:
      _awaiting_configure = false;
      if (_properties.get_fixed_size()) {
        // If the window properties indicate a fixed size only, undo
        // any attempt by the user to change them.  In X, there
        // doesn't appear to be a way to universally disallow this
        // directly (although we do set the min_size and max_size to
        // the same value, which seems to work for most window
        // managers.)
        WindowProperties current_props = get_properties();
        if (event.xconfigure.width != current_props.get_x_size() ||
            event.xconfigure.height != current_props.get_y_size()) {
          XWindowChanges changes;
          changes.width = current_props.get_x_size();
          changes.height = current_props.get_y_size();
          int value_mask = (CWWidth | CWHeight);
          XConfigureWindow(_display, _xwindow, value_mask, &changes);
        }

      } else {
        // A normal window may be resized by the user at will.
        properties.set_size(event.xconfigure.width, event.xconfigure.height);
        system_changed_properties(properties);
      }
      break;

    case ButtonPress:
      // This refers to the mouse buttons.
      button = get_mouse_button(event.xbutton);
      _input_devices[0].set_pointer_in_window(event.xbutton.x, event.xbutton.y);
      _input_devices[0].button_down(button);
      break;

    case ButtonRelease:
      button = get_mouse_button(event.xbutton);
      _input_devices[0].set_pointer_in_window(event.xbutton.x, event.xbutton.y);
      _input_devices[0].button_up(button);
      break;

    case MotionNotify:
      _input_devices[0].set_pointer_in_window(event.xmotion.x, event.xmotion.y);
      break;

    case KeyPress:
      handle_keystroke(event.xkey);
      handle_keypress(event.xkey);
      break;

    case KeyRelease:
      // The KeyRelease can't be processed immediately, because we
      // have to check first if it's immediately followed by a
      // matching KeyPress event.
      keyrelease_event = event.xkey;
      got_keyrelease_event = true;
      break;

    case EnterNotify:
      _input_devices[0].set_pointer_in_window(event.xcrossing.x, event.xcrossing.y);
      break;

    case LeaveNotify:
      _input_devices[0].set_pointer_out_of_window();
      break;

    case FocusIn:
      properties.set_foreground(true);
      system_changed_properties(properties);
      break;

    case FocusOut:
      properties.set_foreground(false);
      system_changed_properties(properties);
      break;

    case UnmapNotify:
      properties.set_minimized(true);
      system_changed_properties(properties);
      break;

    case MapNotify:
      properties.set_minimized(false);
      system_changed_properties(properties);

      // Auto-focus the window when it is mapped.
      XSetInputFocus(_display, _xwindow, RevertToPointerRoot, CurrentTime);
      break;

    case ClientMessage:
      if ((Atom)(event.xclient.data.l[0]) == _wm_delete_window) {
        // This is a message from the window manager indicating that
        // the user has requested to close the window.
        string close_request_event = get_close_request_event();
        if (!close_request_event.empty()) {
          // In this case, the app has indicated a desire to intercept
          // the request and process it directly.
          throw_event(close_request_event);

        } else {
          // In this case, the default case, the app does not intend
          // to service the request, so we do by closing the window.

          // TODO: don't release the gsg in the window thread.
          close_window();
          properties.set_open(false);
          system_changed_properties(properties);
        }
      }
      break;

    case DestroyNotify:
      // Apparently, we never get a DestroyNotify on a toplevel
      // window.  Instead, we rely on hints from the window manager
      // (see above).
      egldisplay_cat.info()
        << "DestroyNotify\n";
      break;

    default:
      egldisplay_cat.error()
        << "unhandled X event type " << event.type << "\n";
    }
  }

  if (got_keyrelease_event) {
    // This keyrelease event is not immediately followed by a
    // matching keypress event, so it's a genuine release.
    handle_keyrelease(keyrelease_event);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::set_properties_now
//       Access: Public, Virtual
//  Description: Applies the requested set of properties to the
//               window, if possible, for instance to request a change
//               in size or minimization status.
//
//               The window properties are applied immediately, rather
//               than waiting until the next frame.  This implies that
//               this method may *only* be called from within the
//               window thread.
//
//               The return value is true if the properties are set,
//               false if they are ignored.  This is mainly useful for
//               derived classes to implement extensions to this
//               function.
////////////////////////////////////////////////////////////////////
void eglGraphicsWindow::
set_properties_now(WindowProperties &properties) {
  if (_pipe == (GraphicsPipe *)NULL) {
    // If the pipe is null, we're probably closing down.
    GraphicsWindow::set_properties_now(properties);
    return;
  }

  eglGraphicsPipe *egl_pipe;
  DCAST_INTO_V(egl_pipe, _pipe);

  // Fullscreen mode is implemented with a hint to the window manager.
  // However, we also implicitly set the origin to (0, 0) and the size
  // to the desktop size, and request undecorated mode, in case the
  // user has a less-capable window manager (or no window manager at
  // all).
  if (properties.get_fullscreen()) {
    properties.set_undecorated(true);
    properties.set_origin(0, 0);
    properties.set_size(egl_pipe->get_display_width(),
                        egl_pipe->get_display_height());
  }

  GraphicsWindow::set_properties_now(properties);
  if (!properties.is_any_specified()) {
    // The base class has already handled this case.
    return;
  }

  // The window is already open; we are limited to what we can change
  // on the fly.

  // We'll pass some property requests on as a window manager hint.
  WindowProperties wm_properties = _properties;
  wm_properties.add_properties(properties);

  // The window title may be changed by issuing another hint request.
  // Assume this will be honored.
  if (properties.has_title()) {
    _properties.set_title(properties.get_title());
    properties.clear_title();
  }

  // Ditto for fullscreen mode.
  if (properties.has_fullscreen()) {
    _properties.set_fullscreen(properties.get_fullscreen());
    properties.clear_fullscreen();
  }

  // The size and position of an already-open window are changed via
  // explicit X calls.  These may still get intercepted by the window
  // manager.  Rather than changing _properties immediately, we'll
  // wait for the ConfigureNotify message to come back.
  XWindowChanges changes;
  int value_mask = 0;

  if (properties.has_origin()) {
    changes.x = properties.get_x_origin();
    changes.y = properties.get_y_origin();
    value_mask |= (CWX | CWY);
    properties.clear_origin();
  }
  if (properties.has_size()) {
    changes.width = properties.get_x_size();
    changes.height = properties.get_y_size();
    value_mask |= (CWWidth | CWHeight);
    properties.clear_size();
  }
  if (properties.has_z_order()) {
    // We'll send the classic stacking request through the standard
    // interface, for users of primitive window managers; but we'll
    // also send it as a window manager hint, for users of modern
    // window managers.
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

  if (value_mask != 0) {
    XReconfigureWMWindow(_display, _xwindow, _screen, value_mask, &changes);

    // Don't draw anything until this is done reconfiguring.
    _awaiting_configure = true;
  }

  // We hide the cursor by setting it to an invisible pixmap.
  if (properties.has_cursor_hidden()) {
    _properties.set_cursor_hidden(properties.get_cursor_hidden());
    if (properties.get_cursor_hidden()) {
      XDefineCursor(_display, _xwindow, egl_pipe->get_hidden_cursor());
    } else {
      XDefineCursor(_display, _xwindow, None);
    }
    properties.clear_cursor_hidden();
  }

  if (properties.has_foreground()) {
    if (properties.get_foreground()) {
      XSetInputFocus(_display, _xwindow, RevertToPointerRoot, CurrentTime);
    } else {
      XSetInputFocus(_display, PointerRoot, RevertToPointerRoot, CurrentTime);
    }
    properties.clear_foreground();
  }

  set_wm_properties(wm_properties, true);
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::close_window
//       Access: Protected, Virtual
//  Description: Closes the window right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void eglGraphicsWindow::
close_window() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    if (!eglMakeCurrent(_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
      egldisplay_cat.error() << "Failed to call eglMakeCurrent: "
        << get_egl_error_string(eglGetError()) << "\n";
    }
    _gsg.clear();
  }

  if (_ic != (XIC)NULL) {
    XDestroyIC(_ic);
    _ic = (XIC)NULL;
  }

  if (_egl_surface != 0) {
    if (!eglDestroySurface(_egl_display, _egl_surface)) {
      egldisplay_cat.error() << "Failed to destroy surface: "
        << get_egl_error_string(eglGetError()) << "\n";
    }
  }

  if (_xwindow != (X11_Window)NULL) {
    XDestroyWindow(_display, _xwindow);
    _xwindow = (X11_Window)NULL;

    // This may be necessary if we just closed the last X window in an
    // application, so the server hears the close request.
    XFlush(_display);
  }
  GraphicsWindow::close_window();
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::open_window
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool eglGraphicsWindow::
open_window() {
  eglGraphicsPipe *egl_pipe;
  DCAST_INTO_R(egl_pipe, _pipe, false);

  // GSG Creation/Initialization
  eglGraphicsStateGuardian *eglgsg;
  if (_gsg == 0) {
    // There is no old gsg.  Create a new one.
    eglgsg = new eglGraphicsStateGuardian(_engine, _pipe, NULL);
    eglgsg->choose_pixel_format(_fb_properties, egl_pipe->get_display(), egl_pipe->get_screen(), false, false);
    _gsg = eglgsg;
  } else {
    // If the old gsg has the wrong pixel format, create a
    // new one that shares with the old gsg.
    DCAST_INTO_R(eglgsg, _gsg, false);
    if (!eglgsg->get_fb_properties().subsumes(_fb_properties)) {
      eglgsg = new eglGraphicsStateGuardian(_engine, _pipe, eglgsg);
      eglgsg->choose_pixel_format(_fb_properties, egl_pipe->get_display(), egl_pipe->get_screen(), false, false);
      _gsg = eglgsg;
    }
  }

  XVisualInfo *visual_info = eglgsg->_visual;
  if (visual_info == NULL) {
    // No X visual for this fbconfig; how can we open the window?
    egldisplay_cat.error()
      << "No X visual: cannot open window.\n";
    return false;
  }
  Visual *visual = visual_info->visual;
  int depth = visual_info->depth;

  if (!_properties.has_origin()) {
    _properties.set_origin(0, 0);
  }
  if (!_properties.has_size()) {
    _properties.set_size(100, 100);
  }
  
  X11_Window parent_window = egl_pipe->get_root();
  WindowHandle *window_handle = _properties.get_parent_window();
  if (window_handle != NULL) {
    egldisplay_cat.info()
      << "Got parent_window " << *window_handle << "\n";
    WindowHandle::OSHandle *os_handle = window_handle->get_os_handle();
    if (os_handle != NULL) {
      egldisplay_cat.info()
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

  setup_colormap(visual_info);

  _event_mask =
    ButtonPressMask | ButtonReleaseMask |
    KeyPressMask | KeyReleaseMask |
    EnterWindowMask | LeaveWindowMask |
    PointerMotionMask |
    FocusChangeMask |
    StructureNotifyMask;

  // Initialize window attributes
  XSetWindowAttributes wa;
  wa.background_pixel = XBlackPixel(_display, _screen);
  wa.border_pixel = 0;
  wa.colormap = _colormap;
  wa.event_mask = _event_mask;

  unsigned long attrib_mask =
    CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

  _xwindow = XCreateWindow
    (_display, parent_window,
     _properties.get_x_origin(), _properties.get_y_origin(),
     _properties.get_x_size(), _properties.get_y_size(),
     0, depth, InputOutput, visual, attrib_mask, &wa);

  if (_xwindow == (X11_Window)0) {
    egldisplay_cat.error()
      << "failed to create X window.\n";
    return false;
  }
  set_wm_properties(_properties, false);

  // We don't specify any fancy properties of the XIC.  It would be
  // nicer if we could support fancy IM's that want preedit callbacks,
  // etc., but that can wait until we have an X server that actually
  // supports these to test it on.
  XIM im = egl_pipe->get_im();
  _ic = NULL;
  if (im) {
    _ic = XCreateIC
      (im,
       XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
       (void*)NULL);
    if (_ic == (XIC)NULL) {
      egldisplay_cat.warning()
        << "Couldn't create input context.\n";
    }
  }

  if (_properties.get_cursor_hidden()) {
    XDefineCursor(_display, _xwindow, egl_pipe->get_hidden_cursor());
  }

  _egl_surface = eglCreateWindowSurface(_egl_display, eglgsg->_fbconfig, (NativeWindowType) _xwindow, NULL);
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

  XMapWindow(_display, _xwindow);

  if (_properties.get_raw_mice()) {
    open_raw_mice();
  } else {
    if (egldisplay_cat.is_debug()) {
      egldisplay_cat.debug()
        << "Raw mice not requested.\n";
    }
  }

  // Create a WindowHandle for ourselves
  _window_handle = NativeWindowHandle::make_x11(_xwindow);

  // And tell our parent window that we're now its child.
  if (_parent_window_handle != (WindowHandle *)NULL) {
    _parent_window_handle->attach_child(_window_handle);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::set_wm_properties
//       Access: Private
//  Description: Asks the window manager to set the appropriate
//               properties.  In X, these properties cannot be
//               specified directly by the application; they must be
//               requested via the window manager, which may or may
//               not choose to honor the request.
//
//               If already_mapped is true, the window has already
//               been mapped (manifested) on the display.  This means
//               we may need to use a different action in some cases.
////////////////////////////////////////////////////////////////////
void eglGraphicsWindow::
set_wm_properties(const WindowProperties &properties, bool already_mapped) {
  // Name the window if there is a name
  XTextProperty window_name;
  XTextProperty *window_name_p = (XTextProperty *)NULL;
  if (properties.has_title()) {
    char *name = (char *)properties.get_title().c_str();
    if (XStringListToTextProperty(&name, 1, &window_name) != 0) {
      window_name_p = &window_name;
    }
  }

  // The size hints request a window of a particular size and/or a
  // particular placement onscreen.
  XSizeHints *size_hints_p = NULL;
  if (properties.has_origin() || properties.has_size()) {
    size_hints_p = XAllocSizeHints();
    if (size_hints_p != (XSizeHints *)NULL) {
      if (properties.has_origin()) {
        size_hints_p->x = properties.get_x_origin();
        size_hints_p->y = properties.get_y_origin();
        size_hints_p->flags |= USPosition;
      }
      if (properties.has_size()) {
        size_hints_p->width = properties.get_x_size();
        size_hints_p->height = properties.get_y_size();
        size_hints_p->flags |= USSize;

        if (properties.get_fixed_size()) {
          size_hints_p->min_width = properties.get_x_size();
          size_hints_p->min_height = properties.get_y_size();
          size_hints_p->max_width = properties.get_x_size();
          size_hints_p->max_height = properties.get_y_size();
          size_hints_p->flags |= (PMinSize | PMaxSize);
        }
      }
    }
  }

  // The window manager hints include requests to the window manager
  // other than those specific to window geometry.
  XWMHints *wm_hints_p = NULL;
  wm_hints_p = XAllocWMHints();
  if (wm_hints_p != (XWMHints *)NULL) {
    if (properties.has_minimized() && properties.get_minimized()) {
      wm_hints_p->initial_state = IconicState;
    } else {
      wm_hints_p->initial_state = NormalState;
    }
    wm_hints_p->flags = StateHint;
  }

  // Two competing window manager interfaces have evolved.  One of
  // them allows to set certain properties as a "type"; the other one
  // as a "state".  We'll try to honor both.
  static const int max_type_data = 32;
  PN_int32 type_data[max_type_data];
  int next_type_data = 0;

  static const int max_state_data = 32;
  PN_int32 state_data[max_state_data];
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

  if (properties.get_fullscreen()) {
    // For a "fullscreen" request, we pass this through, hoping the
    // window manager will support EWMH.
    type_data[next_type_data++] = _net_wm_window_type_fullscreen;

    // We also request it as a state.
    state_data[next_state_data++] = _net_wm_state_fullscreen;
    set_data[next_set_data++] = SetAction(_net_wm_state_fullscreen, _net_wm_state_add);
  } else {
    set_data[next_set_data++] = SetAction(_net_wm_state_fullscreen, _net_wm_state_remove);
  }

  // If we asked for a window without a border, there's no excellent
  // way to arrange that.  For users whose window managers follow the
  // EWMH specification, we can ask for a "splash" screen, which is
  // usually undecorated.  It's not exactly right, but the spec
  // doesn't give us an exactly-right option.

  // For other users, we'll totally punt and just set the window's
  // Class to "Undecorated", and let the user configure his/her window
  // manager not to put a border around windows of this class.
  XClassHint *class_hints_p = NULL;
  if (properties.get_undecorated()) {
    class_hints_p = XAllocClassHint();
    class_hints_p->res_class = (char*) "Undecorated";

    if (!properties.get_fullscreen()) {
      type_data[next_type_data++] = _net_wm_window_type_splash;
    }
  }

  if (properties.has_z_order()) {
    switch (properties.get_z_order()) {
    case WindowProperties::Z_bottom:
      state_data[next_state_data++] = _net_wm_state_below;
      set_data[next_set_data++] = SetAction(_net_wm_state_below, _net_wm_state_add);
      set_data[next_set_data++] = SetAction(_net_wm_state_above, _net_wm_state_remove);
      break;

    case WindowProperties::Z_normal:
      set_data[next_set_data++] = SetAction(_net_wm_state_below, _net_wm_state_remove);
      set_data[next_set_data++] = SetAction(_net_wm_state_above, _net_wm_state_remove);
      break;

    case WindowProperties::Z_top:
      state_data[next_state_data++] = _net_wm_state_above;
      set_data[next_set_data++] = SetAction(_net_wm_state_below, _net_wm_state_remove);
      set_data[next_set_data++] = SetAction(_net_wm_state_above, _net_wm_state_add);
      break;
    }
  }

  nassertv(next_type_data < max_type_data);
  nassertv(next_state_data < max_state_data);
  nassertv(next_set_data < max_set_data);

  XChangeProperty(_display, _xwindow, _net_wm_window_type,
                  XA_ATOM, 32, PropModeReplace,
                  (unsigned char *)type_data, next_type_data);

  // Request the state properties all at once.
  XChangeProperty(_display, _xwindow, _net_wm_state,
                  XA_ATOM, 32, PropModeReplace,
                  (unsigned char *)state_data, next_state_data);

  if (already_mapped) {
    // We have to request state changes differently when the window
    // has been mapped.  To do this, we need to send a client message
    // to the root window for each change.

    eglGraphicsPipe *egl_pipe;
    DCAST_INTO_V(egl_pipe, _pipe);

    for (int i = 0; i < next_set_data; ++i) {
      XClientMessageEvent event;
      memset(&event, 0, sizeof(event));

      event.type = ClientMessage;
      event.send_event = True;
      event.display = _display;
      event.window = _xwindow;
      event.message_type = _net_wm_state;
      event.format = 32;
      event.data.l[0] = set_data[i]._action;
      event.data.l[1] = set_data[i]._state;
      event.data.l[2] = 0;
      event.data.l[3] = 1;

      XSendEvent(_display, egl_pipe->get_root(), True, 0, (XEvent *)&event);
    }
  }

  XSetWMProperties(_display, _xwindow, window_name_p, window_name_p,
                   NULL, 0, size_hints_p, wm_hints_p, class_hints_p);

  if (size_hints_p != (XSizeHints *)NULL) {
    XFree(size_hints_p);
  }
  if (wm_hints_p != (XWMHints *)NULL) {
    XFree(wm_hints_p);
  }
  if (class_hints_p != (XClassHint *)NULL) {
    XFree(class_hints_p);
  }

  // Also, indicate to the window manager that we'd like to get a
  // chance to close our windows cleanly, rather than being rudely
  // disconnected from the X server if the user requests a window
  // close.
  Atom protocols[] = {
    _wm_delete_window,
  };

  XSetWMProtocols(_display, _xwindow, protocols,
                  sizeof(protocols) / sizeof(Atom));
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::setup_colormap
//       Access: Private
//  Description: Allocates a colormap appropriate to the visual and
//               stores in in the _colormap method.
////////////////////////////////////////////////////////////////////
void eglGraphicsWindow::
setup_colormap(XVisualInfo *visual) {
  eglGraphicsPipe *egl_pipe;
  DCAST_INTO_V(egl_pipe, _pipe);
  X11_Window root_window = egl_pipe->get_root();

  int visual_class = visual->c_class;
  int rc, is_rgb;

  switch (visual_class) {
    case PseudoColor:
      _colormap = XCreateColormap(_display, root_window,
                                  visual->visual, AllocAll);
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
      egldisplay_cat.error()
        << "Could not allocate a colormap for visual class "
        << visual_class << ".\n";
      break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::open_raw_mice
//       Access: Private
//  Description: Adds raw mice to the _input_devices list.
////////////////////////////////////////////////////////////////////
void eglGraphicsWindow::
open_raw_mice()
{
#ifdef HAVE_LINUX_INPUT_H
  bool any_present = false;
  bool any_mice = false;

  for (int i=0; i<64; i++) {
    uint8_t evtypes[EV_MAX/8 + 1];
    ostringstream fnb;
    fnb << "/dev/input/event" << i;
    string fn = fnb.str();
    int fd = open(fn.c_str(), O_RDONLY | O_NONBLOCK, 0);
    if (fd >= 0) {
      any_present = true;
      char name[256];
      char phys[256];
      char uniq[256];
      if ((ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0)||
          (ioctl(fd, EVIOCGPHYS(sizeof(phys)), phys) < 0)||
          (ioctl(fd, EVIOCGPHYS(sizeof(uniq)), uniq) < 0)||
          (ioctl(fd, EVIOCGBIT(0, EV_MAX), &evtypes) < 0)) {
        close(fd);
        egldisplay_cat.error() <<
          "Opening raw mice: ioctl failed on " << fn << "\n";
      } else {
        if (test_bit(EV_REL, evtypes) || test_bit(EV_ABS, evtypes)) {
          for (char *p=name; *p; p++) {
            if (((*p<'a')||(*p>'z')) && ((*p<'A')||(*p>'Z')) && ((*p<'0')||(*p>'9'))) {
              *p = '_';
            }
          }
          for (char *p=uniq; *p; p++) {
            if (((*p<'a')||(*p>'z')) && ((*p<'A')||(*p>'Z')) && ((*p<'0')||(*p>'9'))) {
              *p = '_';
            }
          }
          string full_id = ((string)name) + "." + uniq;
          MouseDeviceInfo inf;
          inf._fd = fd;
          inf._input_device_index = _input_devices.size();
          inf._io_buffer = "";
          _mouse_device_info.push_back(inf);
          GraphicsWindowInputDevice device =
            GraphicsWindowInputDevice::pointer_only(this, full_id);
          add_input_device(device);
          egldisplay_cat.info() << "Raw mouse " <<
            inf._input_device_index << " detected: " << full_id << "\n";
          any_mice = true;
        } else {
          close(fd);
        }
      }
    } else {
      if ((errno == ENOENT)||(errno == ENOTDIR)) {
        break;
      } else {
        any_present = true;
        egldisplay_cat.error() <<
          "Opening raw mice: " << strerror(errno) << " " << fn << "\n";
      }
    }
  }

  if (!any_present) {
    egldisplay_cat.error() <<
      "Opening raw mice: files not found: /dev/input/event*\n";
  } else if (!any_mice) {
    egldisplay_cat.error() <<
      "Opening raw mice: no mouse devices detected in /dev/input/event*\n";
  }
#else
  egldisplay_cat.error() <<
    "Opening raw mice: panda not compiled with raw mouse support.\n";
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::poll_raw_mice
//       Access: Private
//  Description: Reads events from the raw mouse device files.
////////////////////////////////////////////////////////////////////
void eglGraphicsWindow::
poll_raw_mice()
{
#ifdef HAVE_LINUX_INPUT_H
  for (int dev=0; dev<_mouse_device_info.size(); dev++) {
    MouseDeviceInfo &inf = _mouse_device_info[dev];

    // Read all bytes into buffer.
    if (inf._fd >= 0) {
      while (1) {
        char tbuf[1024];
        int nread = read(inf._fd, tbuf, sizeof(tbuf));
        if (nread > 0) {
          inf._io_buffer += string(tbuf, nread);
        } else {
          if ((nread < 0)&&((errno == EWOULDBLOCK) || (errno==EAGAIN))) {
            break;
          }
          close(inf._fd);
          inf._fd = -1;
          break;
        }
      }
    }

    // Process events.
    int nevents = inf._io_buffer.size() / sizeof(struct input_event);
    if (nevents == 0) {
      continue;
    }
    const input_event *events = (const input_event *)(inf._io_buffer.c_str());
    GraphicsWindowInputDevice &dev = _input_devices[inf._input_device_index];
    int x = dev.get_raw_pointer().get_x();
    int y = dev.get_raw_pointer().get_y();
    for (int i=0; i<nevents; i++) {
      if (events[i].type == EV_REL) {
        if (events[i].code == REL_X) x += events[i].value;
        if (events[i].code == REL_Y) y += events[i].value;
      } else if (events[i].type == EV_ABS) {
        if (events[i].code == ABS_X) x = events[i].value;
        if (events[i].code == ABS_Y) y = events[i].value;
      } else if (events[i].type == EV_KEY) {
        if ((events[i].code >= BTN_MOUSE)&&(events[i].code < BTN_MOUSE+8)) {
          int btn = events[i].code - BTN_MOUSE;
          dev.set_pointer_in_window(x,y);
          if (events[i].value) {
            dev.button_down(MouseButton::button(btn));
          } else {
            dev.button_up(MouseButton::button(btn));
          }
        }
      }
    }
    inf._io_buffer.erase(0,nevents*sizeof(struct input_event));
    dev.set_pointer_in_window(x,y);
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::handle_keystroke
//       Access: Private
//  Description: Generates a keystroke corresponding to the indicated
//               X KeyPress event.
////////////////////////////////////////////////////////////////////
void eglGraphicsWindow::
handle_keystroke(XKeyEvent &event) {
  _input_devices[0].set_pointer_in_window(event.x, event.y);

  if (_ic) {
    // First, get the keystroke as a wide-character sequence.
    static const int buffer_size = 256;
    wchar_t buffer[buffer_size];
    Status status;
    int len = XwcLookupString(_ic, &event, buffer, buffer_size, NULL,
                              &status);
    if (status == XBufferOverflow) {
      egldisplay_cat.error()
        << "Overflowed input buffer.\n";
    }

    // Now each of the returned wide characters represents a
    // keystroke.
    for (int i = 0; i < len; i++) {
      _input_devices[0].keystroke(buffer[i]);
    }

  } else {
    // Without an input context, just get the ascii keypress.
    ButtonHandle button = get_button(event, true);
    if (button.has_ascii_equivalent()) {
      _input_devices[0].keystroke(button.get_ascii_equivalent());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::handle_keypress
//       Access: Private
//  Description: Generates a keypress corresponding to the indicated
//               X KeyPress event.
////////////////////////////////////////////////////////////////////
void eglGraphicsWindow::
handle_keypress(XKeyEvent &event) {
  _input_devices[0].set_pointer_in_window(event.x, event.y);

  // Now get the raw unshifted button.
  ButtonHandle button = get_button(event, false);
  if (button != ButtonHandle::none()) {
    if (button == KeyboardButton::lcontrol() || button == KeyboardButton::rcontrol()) {
      _input_devices[0].button_down(KeyboardButton::control());
    }
    if (button == KeyboardButton::lshift() || button == KeyboardButton::rshift()) {
      _input_devices[0].button_down(KeyboardButton::shift());
    }
    if (button == KeyboardButton::lalt() || button == KeyboardButton::ralt()) {
      _input_devices[0].button_down(KeyboardButton::alt());
    }
    if (button == KeyboardButton::lmeta() || button == KeyboardButton::rmeta()) {
      _input_devices[0].button_down(KeyboardButton::meta());
    }
    _input_devices[0].button_down(button);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::handle_keyrelease
//       Access: Private
//  Description: Generates a keyrelease corresponding to the indicated
//               X KeyRelease event.
////////////////////////////////////////////////////////////////////
void eglGraphicsWindow::
handle_keyrelease(XKeyEvent &event) {
  _input_devices[0].set_pointer_in_window(event.x, event.y);

  // Now get the raw unshifted button.
  ButtonHandle button = get_button(event, false);
  if (button != ButtonHandle::none()) {
    if (button == KeyboardButton::lcontrol() || button == KeyboardButton::rcontrol()) {
      _input_devices[0].button_up(KeyboardButton::control());
    }
    if (button == KeyboardButton::lshift() || button == KeyboardButton::rshift()) {
      _input_devices[0].button_up(KeyboardButton::shift());
    }
    if (button == KeyboardButton::lalt() || button == KeyboardButton::ralt()) {
      _input_devices[0].button_up(KeyboardButton::alt());
    }
    if (button == KeyboardButton::lmeta() || button == KeyboardButton::rmeta()) {
      _input_devices[0].button_up(KeyboardButton::meta());
    }
    _input_devices[0].button_up(button);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::get_button
//       Access: Private
//  Description: Returns the Panda ButtonHandle corresponding to the
//               keyboard button indicated by the given key event.
////////////////////////////////////////////////////////////////////
ButtonHandle eglGraphicsWindow::
get_button(XKeyEvent &key_event, bool allow_shift) {
  KeySym key = XLookupKeysym(&key_event, 0);

  if ((key_event.state & Mod2Mask) != 0) {
    // Mod2Mask corresponds to NumLock being in effect.  In this case,
    // we want to get the alternate keysym associated with any keypad
    // keys.  Weird system.
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
      // If that didn't produce a button we know, just fall through
      // and handle the normal, un-numlocked key.
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

    // If caps lock is down, shift lowercase letters to uppercase.  We
    // can do this in just the ASCII set, because we handle
    // international keyboards elsewhere (via an input context).
    if ((key_event.state & (ShiftMask | LockMask)) != 0) {
      if (key >= XK_a and key <= XK_z) {
        key += (XK_A - XK_a);
      }
    }
  }

  return map_button(key);
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::map_button
//       Access: Private
//  Description: Maps from a single X keysym to Panda's ButtonHandle.
//               Called by get_button(), above.
////////////////////////////////////////////////////////////////////
ButtonHandle eglGraphicsWindow::
map_button(KeySym key) {
  switch (key) {
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
    return KeyboardButton::lmeta();
  case XK_Meta_R:
    return KeyboardButton::rmeta();
  case XK_Caps_Lock:
    return KeyboardButton::caps_lock();
  case XK_Shift_Lock:
    return KeyboardButton::shift_lock();
  }

  return ButtonHandle::none();
}

////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::get_mouse_button
//       Access: Private
//  Description: Returns the Panda ButtonHandle corresponding to the
//               mouse button indicated by the given button event.
////////////////////////////////////////////////////////////////////
ButtonHandle eglGraphicsWindow::
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
////////////////////////////////////////////////////////////////////
//     Function: eglGraphicsWindow::check_event
//       Access: Private, Static
//  Description: This function is used as a predicate to
//               XCheckIfEvent() to determine if the indicated queued
//               X event is relevant and should be returned to this
//               window.
////////////////////////////////////////////////////////////////////
Bool eglGraphicsWindow::
check_event(X11_Display *display, XEvent *event, char *arg) {
  const eglGraphicsWindow *self = (eglGraphicsWindow *)arg;

  // We accept any event that is sent to our window.
  return (event->xany.window == self->_xwindow);
}
