// Filename: glxGraphicsWindow.cxx
// Created by:  mike (09Jan97)
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

#include <errno.h>
#include <sys/time.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

TypeHandle glxGraphicsWindow::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
glxGraphicsWindow::
glxGraphicsWindow(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                  const string &name) :
  GraphicsWindow(pipe, gsg, name) 
{
  glxGraphicsPipe *glx_pipe;
  DCAST_INTO_V(glx_pipe, _pipe);
  _display = glx_pipe->get_display();
  _screen = glx_pipe->get_screen();
  _xwindow = (Window)NULL;
  _ic = (XIC)NULL;
  _awaiting_configure = false;
  _wm_delete_window = glx_pipe->get_wm_delete_window();

  GraphicsWindowInputDevice device =
    GraphicsWindowInputDevice::pointer_and_keyboard("keyboard/mouse");
  _input_devices.push_back(device);
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
glxGraphicsWindow::
~glxGraphicsWindow() {
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::make_current
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               during begin_frame() to ensure the graphics context
//               is ready for drawing.
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::
make_current() {
  PStatTimer timer(_make_current_pcollector);

  glxGraphicsStateGuardian *glxgsg;
  DCAST_INTO_V(glxgsg, _gsg);
  glXMakeCurrent(_display, _xwindow, glxgsg->_context);

  // Now that we have made the context current to a window, we can
  // reset the GSG state if this is the first time it has been used.
  // (We can't just call reset() when we construct the GSG, because
  // reset() requires having a current context.)
  glxgsg->reset_if_new();
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::release_gsg
//       Access: Public
//  Description: Releases the current GSG pointer, if it is currently
//               held, and resets the GSG to NULL.  The window will be
//               permanently unable to render; this is normally called
//               only just before destroying the window.  This should
//               only be called from within the draw thread.
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::
release_gsg() {
  glXMakeCurrent(_display, None, NULL);
  GraphicsWindow::release_gsg();
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool glxGraphicsWindow::
begin_frame() {
  if (_awaiting_configure) {
    // Don't attempt to draw while we have just reconfigured the
    // window and we haven't got the notification back yet.
    return false;
  }

  return GraphicsWindow::begin_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::begin_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after end_frame() has been called on all windows, to
//               initiate the exchange of the front and back buffers.
//
//               This should instruct the window to prepare for the
//               flip at the next video sync, but it should not wait.
//
//               We have the two separate functions, begin_flip() and
//               end_flip(), to make it easier to flip all of the
//               windows at the same time.
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::
begin_flip() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    make_current();
    glXSwapBuffers(_display, _xwindow);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::process_events
//       Access: Public, Virtual
//  Description: Do whatever processing is necessary to ensure that
//               the window responds to user events.  Also, honor any
//               requests recently made via request_properties()
//
//               This function is called only within the window
//               thread.
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::
process_events() {
  GraphicsWindow::process_events();

  if (_xwindow == (Window)0) {
    return;
  }

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
      button = MouseButton::button(event.xbutton.button - 1);
      _input_devices[0].set_pointer_in_window(event.xbutton.x, event.xbutton.y);
      _input_devices[0].button_down(button);
      break;
      
    case ButtonRelease:
      button = MouseButton::button(event.xbutton.button - 1);
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
      break;

    case ClientMessage:
      if ((Atom)(event.xclient.data.l[0]) == _wm_delete_window) {
        // This is a message from the window manager indicating that
        // the user has requested to close the window.  Honor the
        // request.
        // TODO: don't call release_gsg() in the window thread.
        release_gsg();
        close_window();
        properties.set_open(false);
        system_changed_properties(properties);
      }
      break;

    case DestroyNotify:
      // Apparently, we never get a DestroyNotify on a toplevel
      // window.  Instead, we rely on hints from the window manager
      // (see above).
      glxdisplay_cat.info()
        << "DestroyNotify\n";
      break;

    default:
      glxdisplay_cat.error()
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
//     Function: glxGraphicsWindow::set_properties_now
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
void glxGraphicsWindow::
set_properties_now(WindowProperties &properties) {
  GraphicsWindow::set_properties_now(properties);
  if (!properties.is_any_specified()) {
    // The base class has already handled this case.
    return;
  }

  // The window is already open; we are limited to what we can change
  // on the fly.

  // The window title may be changed by issuing another hint request.
  // Assume this will be honored.
  if (properties.has_title()) {
    WindowProperties wm_properties;
    wm_properties.set_title(properties.get_title());
    _properties.set_title(properties.get_title());
    set_wm_properties(wm_properties);
    properties.clear_title();
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

  if (value_mask != 0) {
    XConfigureWindow(_display, _xwindow, value_mask, &changes);

    // Don't draw anything until this is done reconfiguring.
    _awaiting_configure = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::close_window
//       Access: Protected, Virtual
//  Description: Closes the window right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::
close_window() {
  if (_ic != (XIC)NULL) {
    XDestroyIC(_ic);
    _ic = (XIC)NULL;
  }

  if (_xwindow != (Window)NULL) {
    XDestroyWindow(_display, _xwindow);
    _xwindow = (Window)NULL;

    // This may be necessary if we just closed the last X window in an
    // application, so the server hears the close request.
    XFlush(_display);
  }
  GraphicsWindow::close_window();
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::open_window
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool glxGraphicsWindow::
open_window() {
  if (_properties.get_fullscreen()) {
    // We don't support fullscreen windows.
    return false;
  }

  glxGraphicsPipe *glx_pipe;
  DCAST_INTO_R(glx_pipe, _pipe, false);
  glxGraphicsStateGuardian *glxgsg;
  DCAST_INTO_R(glxgsg, _gsg, false);

  XVisualInfo *visual_info = 
    glXGetVisualFromFBConfig(_display, glxgsg->_fbconfig);

  if (visual_info == NULL) {
    // No X visual for this fbconfig; how can we open the window?
    glxdisplay_cat.error()
      << "Cannot open window without an X visual.\n";
    return false;
  }
  Visual *visual = visual_info->visual;
  int depth = visual_info->depth;
  XFree(visual_info);


  if (!_properties.has_origin()) {
    _properties.set_origin(0, 0);
  }
  if (!_properties.has_size()) {
    _properties.set_size(100, 100);
  }

  Window root_window = glx_pipe->get_root();

  setup_colormap(glxgsg->_fbconfig);

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
    (_display, root_window,
     _properties.get_x_origin(), _properties.get_y_origin(),
     _properties.get_x_size(), _properties.get_y_size(),
     0, depth, InputOutput, visual, attrib_mask, &wa);

  if (_xwindow == (Window)0) {
    glxdisplay_cat.error()
      << "failed to create X window.\n";
    return false;
  }
  set_wm_properties(_properties);

  // We don't specify any fancy properties of the XIC.  It would be
  // nicer if we could support fancy IM's that want preedit callbacks,
  // etc., but that can wait until we have an X server that actually
  // supports these to test it on.
  _ic = XCreateIC
    (glx_pipe->get_im(),
     XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
     NULL);
  if (_ic == (XIC)NULL) {
    glxdisplay_cat.warning()
      << "Couldn't create input context.\n";
  }

  XMapWindow(_display, _xwindow);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::set_wm_properties
//       Access: Private
//  Description: Asks the window manager to set the appropriate
//               properties.  In X, these properties cannot be
//               specified directly by the application; they must be
//               requested via the window manager, which may or may
//               not choose to honor the request.
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::
set_wm_properties(const WindowProperties &properties) {
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

        if (properties.has_fixed_size()) {
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

  // If we asked for a window without a border, there's no good way to
  // arrange that.  It completely depends on the user's window manager
  // of choice.  Instead, we'll totally punt and just set the window's
  // Class to "Undecorated", and let the user configure his/her window
  // manager not to put a border around windows of this class.
  XClassHint *class_hints_p = NULL;
  if (properties.get_undecorated()) {
    class_hints_p = XAllocClassHint();
    class_hints_p->res_class = "Undecorated";
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
//     Function: glxGraphicsWindow::setup_colormap
//       Access: Private
//  Description: Allocates a colormap appropriate to the fbconfig and
//               stores in in the _colormap method.
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::
setup_colormap(GLXFBConfig fbconfig) {
  XVisualInfo *visual_info = glXGetVisualFromFBConfig(_display, fbconfig);
  if (visual_info == NULL) {
    // No X visual; no need to set up a colormap.
    return;
  }
  int visual_class = visual_info->c_class;
  Visual *visual = visual_info->visual;
  XFree(visual_info);

  glxGraphicsPipe *glx_pipe;
  DCAST_INTO_V(glx_pipe, _pipe);
  Window root_window = glx_pipe->get_root();

  int rc, is_rgb;

  switch (visual_class) {
    case PseudoColor:
      rc = glXGetFBConfigAttrib(_display, fbconfig, GLX_RGBA, &is_rgb);
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

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::handle_keystroke
//       Access: Private
//  Description: Generates a keystroke corresponding to the indicated
//               X KeyPress event.
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::
handle_keystroke(XKeyEvent &event) {
  _input_devices[0].set_pointer_in_window(event.x, event.y);

  // First, get the keystroke as a wide-character sequence.
  static const int buffer_size = 256;
  wchar_t buffer[buffer_size];
  Status status;
  int len = XwcLookupString(_ic, &event, buffer, buffer_size, NULL,
                            &status);
  if (status == XBufferOverflow) {
    glxdisplay_cat.error()
      << "Overflowed input buffer.\n";
  }
  
  // Now each of the returned wide characters represents a
  // keystroke.
  for (int i = 0; i < len; i++) {
    _input_devices[0].keystroke(buffer[i]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::handle_keypress
//       Access: Private
//  Description: Generates a keypress corresponding to the indicated
//               X KeyPress event.
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::
handle_keypress(XKeyEvent &event) {
  _input_devices[0].set_pointer_in_window(event.x, event.y);

  // Now get the raw unshifted button.
  ButtonHandle button = get_button(&event);
  if (button != ButtonHandle::none()) {
    _input_devices[0].button_down(button);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::handle_keyrelease
//       Access: Private
//  Description: Generates a keyrelease corresponding to the indicated
//               X KeyRelease event.
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::
handle_keyrelease(XKeyEvent &event) {
  _input_devices[0].set_pointer_in_window(event.x, event.y);

  // Now get the raw unshifted button.
  ButtonHandle button = get_button(&event);
  if (button != ButtonHandle::none()) {
    _input_devices[0].button_up(button);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::get_button
//       Access: Private
//  Description: Returns the Panda ButtonHandle corresponding to the
//               keyboard button indicated by the given key event.
////////////////////////////////////////////////////////////////////
ButtonHandle glxGraphicsWindow::
get_button(XKeyEvent *key_event) {
  KeySym key = XLookupKeysym(key_event, 0);

  switch (key) {
  case XK_BackSpace:
    return KeyboardButton::backspace();
  case XK_Tab:
    return KeyboardButton::tab();
  case XK_Return:
    return KeyboardButton::enter();
  case XK_Escape:
    return KeyboardButton::escape();
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
    return KeyboardButton::ascii_key('*');
  case XK_plus:
    return KeyboardButton::ascii_key('+');
  case XK_comma:
    return KeyboardButton::ascii_key(',');
  case XK_minus:
    return KeyboardButton::ascii_key('-');
  case XK_period:
    return KeyboardButton::ascii_key('.');
  case XK_slash:
    return KeyboardButton::ascii_key('/');
  case XK_0:
    return KeyboardButton::ascii_key('0');
  case XK_1:
    return KeyboardButton::ascii_key('1');
  case XK_2:
    return KeyboardButton::ascii_key('2');
  case XK_3:
    return KeyboardButton::ascii_key('3');
  case XK_4:
    return KeyboardButton::ascii_key('4');
  case XK_5:
    return KeyboardButton::ascii_key('5');
  case XK_6:
    return KeyboardButton::ascii_key('6');
  case XK_7:
    return KeyboardButton::ascii_key('7');
  case XK_8:
    return KeyboardButton::ascii_key('8');
  case XK_9:
    return KeyboardButton::ascii_key('9');
  case XK_colon:
    return KeyboardButton::ascii_key(':');
  case XK_semicolon:
    return KeyboardButton::ascii_key(';');
  case XK_less:
    return KeyboardButton::ascii_key('<');
  case XK_equal:
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
    return KeyboardButton::f1();
  case XK_F2:
    return KeyboardButton::f2();
  case XK_F3:
    return KeyboardButton::f3();
  case XK_F4:
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
  case XK_Shift_L:
  case XK_Shift_R:
    return KeyboardButton::shift();
  case XK_Control_L:
  case XK_Control_R:
    return KeyboardButton::control();
  case XK_Alt_L:
  case XK_Alt_R:
    return KeyboardButton::alt();
  case XK_Meta_L:
  case XK_Meta_R:
    return KeyboardButton::meta();
  case XK_Caps_Lock:
    return KeyboardButton::caps_lock();
  case XK_Shift_Lock:
    return KeyboardButton::shift_lock();
  }

  return ButtonHandle::none();
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::check_event
//       Access: Private, Static
//  Description: This function is used as a predicate to
//               XCheckIfEvent() to determine if the indicated queued
//               X event is relevant and should be returned to this
//               window.
////////////////////////////////////////////////////////////////////
Bool glxGraphicsWindow::
check_event(Display *display, XEvent *event, char *arg) {
  const glxGraphicsWindow *self = (glxGraphicsWindow *)arg;

  // We accept any event that is sent to our window.
  return (event->xany.window == self->_xwindow);
}
