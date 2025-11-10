/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file webGLGraphicsWindow.cxx
 * @author rdb
 * @date 2015-03-31
 */

#include "webGLGraphicsWindow.h"
#include "webGLGraphicsStateGuardian.h"
#include "config_webgldisplay.h"
#include "mouseButton.h"
#include "keyboardButton.h"
#include "throw_event.h"
#include "pointerData.h"
#include <emscripten.h>

#ifndef CPPPARSER
extern "C" void EMSCRIPTEN_KEEPALIVE
_canvas_resized(WebGLGraphicsWindow *window, double width, double height) {
  window->on_resize(width, height);
}
#endif

TypeHandle WebGLGraphicsWindow::_type_handle;

/**
 *
 */
WebGLGraphicsWindow::
WebGLGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                    const std::string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host) :
  GraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  PT(GraphicsWindowInputDevice) device =
    GraphicsWindowInputDevice::pointer_and_keyboard(this, "keyboard_mouse");
  add_input_device(device);
  _input = device.p();
}

/**
 *
 */
WebGLGraphicsWindow::
~WebGLGraphicsWindow() {
}

/**
 * Forces the pointer to the indicated position within the window, if
 * possible.
 *
 * Returns true if successful, false on failure.  This may fail if the mouse
 * is not currently within the window, or if the API doesn't support this
 * operation.
 */
bool WebGLGraphicsWindow::
move_pointer(int device, int x, int y) {
  if (device == 0 && _properties.get_mouse_mode() == WindowProperties::M_relative) {
    // The pointer position is meaningless in relative mode, so we silently
    // pretend that this worked.
    _input->set_pointer_in_window(x, y);
    return true;
  }
  return false;
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool WebGLGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  PStatTimer timer(_make_current_pcollector, current_thread);

  begin_frame_spam(mode);
  if (_gsg == nullptr) {
    return false;
  }

  WebGLGraphicsStateGuardian *webgl_gsg;
  DCAST_INTO_R(webgl_gsg, _gsg, false);

  if (emscripten_is_webgl_context_lost(webgl_gsg->_context)) {
    // The context was lost, and any GL calls we make will fail.
    return false;
  }

  if (emscripten_webgl_make_context_current(webgl_gsg->_context) != EMSCRIPTEN_RESULT_SUCCESS) {
    webgldisplay_cat.error()
      << "Failed to make context current.\n";
    return false;
  }

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
void WebGLGraphicsWindow::
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
void WebGLGraphicsWindow::
end_flip() {
  GraphicsWindow::end_flip();
}

/**
 * Do whatever processing is necessary to ensure that the window responds to
 * user events.  Also, honor any requests recently made via
 * request_properties()
 *
 * This function is called only within the window thread.
 */
void WebGLGraphicsWindow::
process_events() {
  GraphicsWindow::process_events();
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
void WebGLGraphicsWindow::
set_properties_now(WindowProperties &properties) {
  GraphicsWindow::set_properties_now(properties);

  const char *target = "#canvas";

  if (properties.has_size()) {
    _properties.set_size(properties.get_size());
    emscripten_set_canvas_element_size(target, properties.get_x_size(), properties.get_y_size());
    emscripten_set_element_css_size(target, properties.get_x_size(), properties.get_y_size());
    properties.clear_size();
    set_size_and_recalc(_properties.get_x_size(), _properties.get_y_size());
    throw_event(get_window_event(), this);
  }

  if (properties.has_fullscreen() &&
      properties.get_fullscreen() != _properties.get_fullscreen()) {

    if (properties.get_fullscreen()) {
      EMSCRIPTEN_RESULT result = emscripten_request_fullscreen(target, true);

      if (result == EMSCRIPTEN_RESULT_SUCCESS) {
        _properties.set_fullscreen(true);
        properties.clear_fullscreen();

      } else if (result == EMSCRIPTEN_RESULT_DEFERRED) {
        // We can't switch to fullscreen just yet - this action is deferred
        // until we're in an event handler.  We can't know for sure yet that
        // fullscreen will be supported, but we shouldn't report failure.
        properties.clear_fullscreen();
      }
    } else {
      if (emscripten_exit_fullscreen() == EMSCRIPTEN_RESULT_SUCCESS) {
        _properties.set_fullscreen(false);
        properties.clear_fullscreen();
      }
    }
  }

  if (properties.has_mouse_mode() &&
      properties.get_mouse_mode() != _properties.get_mouse_mode()) {

    if (properties.get_mouse_mode() == WindowProperties::M_relative) {
      EMSCRIPTEN_RESULT result = emscripten_request_pointerlock(target, true);

      if (result == EMSCRIPTEN_RESULT_SUCCESS) {
        // Great, we're in pointerlock mode.
        _properties.set_mouse_mode(WindowProperties::M_absolute);
        _properties.set_cursor_hidden(true);
        properties.clear_mouse_mode();

      } else if (result == EMSCRIPTEN_RESULT_DEFERRED) {
        // We can't switch to pointerlock just yet - this action is deferred
        // until we're in an event handler.  We can't know for sure yet that
        // pointerlock will be supported, but we shouldn't report failure.
        properties.clear_mouse_mode();
        if (properties.has_cursor_hidden() && properties.get_cursor_hidden()) {
          properties.clear_cursor_hidden();
        }
      }
    } else {
      if (emscripten_exit_pointerlock() == EMSCRIPTEN_RESULT_SUCCESS) {
        _properties.set_mouse_mode(WindowProperties::M_absolute);
        properties.clear_mouse_mode();
        properties.clear_cursor_hidden();
      }
    }
  }

  if (properties.has_cursor_hidden() &&
      properties.get_cursor_hidden() == (_properties.get_mouse_mode() == WindowProperties::M_relative)) {
    // A hidden cursor comes for free with pointerlock.  Without pointerlock,
    // though, we can't hide the cursor.
    properties.clear_cursor_hidden();
  }

  if (properties.get_foreground()) {
    EM_ASM_({
      var canvas = document.getElementById('canvas');
      if (canvas) {
        canvas.focus();
      }
    });
    properties.clear_foreground();
  }
}

/**
 * Closes the window right now.  Called from the window thread.
 */
void WebGLGraphicsWindow::
close_window() {
  if (_gsg != nullptr) {
    emscripten_webgl_make_context_current(0);
    _gsg.clear();
  }

  EM_ASM({
    var canvas = document.getElementById('canvas');
    if (canvas && canvas._p3d_resizeObserver) {
      canvas._p3d_resizeObserver.disconnect();
      delete canvas._p3d_resizeObserver;
    }
  });

  // Clear the assigned callbacks.
  const char *target = "#canvas";
  emscripten_set_fullscreenchange_callback(target, nullptr, false, nullptr);
  emscripten_set_pointerlockchange_callback(target, nullptr, false, nullptr);
  emscripten_set_visibilitychange_callback(nullptr, false, nullptr);

  emscripten_set_focus_callback(target, nullptr, false, nullptr);
  emscripten_set_blur_callback(target, nullptr, false, nullptr);

  emscripten_set_keypress_callback(target, nullptr, false, nullptr);
  emscripten_set_keydown_callback(target, nullptr, false, nullptr);
  emscripten_set_keyup_callback(target, nullptr, false, nullptr);

  //emscripten_set_click_callback(target, nullptr, false, nullptr);
  emscripten_set_mousedown_callback(target, nullptr, false, nullptr);
  emscripten_set_mouseup_callback(target, nullptr, false, nullptr);
  emscripten_set_mousemove_callback(target, nullptr, false, nullptr);
  emscripten_set_mouseenter_callback(target, nullptr, false, nullptr);
  emscripten_set_mouseleave_callback(target, nullptr, false, nullptr);

  emscripten_set_wheel_callback(target, nullptr, false, nullptr);

  GraphicsWindow::close_window();
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool WebGLGraphicsWindow::
open_window() {
  //WebGLGraphicsPipe *webgl_pipe;
  //DCAST_INTO_R(webgl_pipe, _pipe, false);

  const char *target = "#canvas";

  // GSG Creation/Initialization
  WebGLGraphicsStateGuardian *webgl_gsg;
  if (_gsg == nullptr) {
    // There is no old gsg.  Create a new one.
    webgl_gsg = new WebGLGraphicsStateGuardian(_engine, _pipe);
    webgl_gsg->choose_pixel_format(_fb_properties, target);
    _gsg = webgl_gsg;
  } else {
    // If the old gsg has the wrong pixel format, create a new one.
    DCAST_INTO_R(webgl_gsg, _gsg, false);
    //if (!webgl_gsg->_fb_properties.subsumes(_fb_properties)) {
    //  webgl_gsg = new WebGLGraphicsStateGuardian(_engine, _pipe);
    //  webgl_gsg->choose_pixel_format(_fb_properties, target);
    //  _gsg = webgl_gsg;
    //}
  }

  if (!webgl_gsg->_have_context) {
    return false;
  }

  // For now, always use the size specified in the CSS, except when fixed size
  // has been specified, or the CSS has no size
  double css_width, css_height;
  emscripten_get_element_css_size(target, &css_width, &css_height);

  if (_properties.has_size() && (_properties.get_fixed_size() || css_width == 0.0 || css_height == 0.0)) {
    emscripten_set_canvas_element_size(target, _properties.get_x_size(), _properties.get_y_size());
    emscripten_set_element_css_size(target, _properties.get_x_size(), _properties.get_y_size());
  } else {
    int width = (int)css_width;
    int height = (int)css_height;
    if (width == 0 || height == 0) {
      emscripten_get_canvas_element_size(target, &width, &height);
    } else {
      emscripten_set_canvas_element_size(target, width, height);
    }
    _properties.set_size(width, height);

    EmscriptenFullscreenChangeEvent event;
    emscripten_get_fullscreen_status(&event);
    _properties.set_fullscreen(event.isFullscreen);
  }

  _properties.set_undecorated(true);

  if (emscripten_webgl_make_context_current(webgl_gsg->_context) != EMSCRIPTEN_RESULT_SUCCESS) {
    webgldisplay_cat.error()
      << "Failed to make context current.\n";
    return false;
  }

  webgl_gsg->reset();

  _fb_properties.clear();
  _fb_properties.set_rgb_color(true);
  _fb_properties.set_force_hardware(true);
  _fb_properties.set_back_buffers(1);

  GLint red_bits, green_bits, blue_bits, alpha_bits, depth_bits, stencil_bits;

  glGetIntegerv(GL_RED_BITS, &red_bits);
  glGetIntegerv(GL_GREEN_BITS, &green_bits);
  glGetIntegerv(GL_BLUE_BITS, &blue_bits);
  glGetIntegerv(GL_ALPHA_BITS, &alpha_bits);
  glGetIntegerv(GL_DEPTH_BITS, &depth_bits);
  glGetIntegerv(GL_STENCIL_BITS, &stencil_bits);

  _fb_properties.set_rgba_bits(red_bits, green_bits, blue_bits, alpha_bits);
  _fb_properties.set_depth_bits(depth_bits);
  _fb_properties.set_stencil_bits(stencil_bits);

  // Set callbacks.
  emscripten_set_fullscreenchange_callback(target, (void *)this, false, &on_fullscreen_event);
  emscripten_set_pointerlockchange_callback(target, (void *)this, false, &on_pointerlock_event);
  emscripten_set_visibilitychange_callback((void *)this, false, &on_visibility_event);

  emscripten_set_focus_callback(target, (void *)this, false, &on_focus_event);
  emscripten_set_blur_callback(target, (void *)this, false, &on_focus_event);

  void *user_data = _input;
  emscripten_set_keypress_callback(target, user_data, false, &on_keyboard_event);
  emscripten_set_keydown_callback(target, user_data, false, &on_keyboard_event);
  emscripten_set_keyup_callback(target, user_data, false, &on_keyboard_event);

  //emscripten_set_click_callback(target, user_data, false, &on_mouse_event);
  emscripten_set_mousedown_callback(target, user_data, false, &on_mouse_event);
  emscripten_set_mouseup_callback(target, user_data, false, &on_mouse_event);
  emscripten_set_mousemove_callback(target, user_data, false, &on_mouse_event);
  emscripten_set_mouseenter_callback(target, user_data, false, &on_mouse_event);
  emscripten_set_mouseleave_callback(target, user_data, false, &on_mouse_event);

  emscripten_set_wheel_callback(target, user_data, false, &on_wheel_event);

  // Emscripten has no working resize handler for the canvas element, we'll
  // have to create our own
  EM_ASM({
    var canvas = document.getElementById('canvas');
    if (canvas) {
      var observer = new ResizeObserver(function(entries) {
        var entry = entries[0];
        if (entry) {
          var width = entry.contentRect.width;
          var height = entry.contentRect.height;
          if (width != 0 && height != 0) {
            __canvas_resized($0, entry.contentRect.width, entry.contentRect.height);
          }
        }
      });
      observer.observe(canvas);
      if (canvas._p3d_resizeObserver) {
        canvas._p3d_resizeObserver.disconnect();
      }
      canvas._p3d_resizeObserver = observer;
    }
  }, this);

  if (!_properties.has_foreground() || _properties.get_foreground()) {
    _properties.set_foreground(EM_ASM_INT({
      var canvas = document.getElementById('canvas');
      if (canvas) {
        canvas.focus();

        return document.activeElement === canvas;
      } else {
        return false;
      }
    }));
  }

  return true;
}

/**
 *
 */
void WebGLGraphicsWindow::
on_resize(double width, double height) {
  if (_properties.get_fixed_size()) {
    return;
  }

  const char *target = "#canvas";

  LVecBase2i size((int)width, (int)height);
  emscripten_set_canvas_element_size(target, size[0], size[1]);
  if (_properties.get_size() != size) {
    system_changed_properties(WindowProperties::size(size));
  }
}

/**
 *
 */
EM_BOOL WebGLGraphicsWindow::
on_fullscreen_event(int type, const EmscriptenFullscreenChangeEvent *event, void *user_data) {
  WebGLGraphicsWindow *window = (WebGLGraphicsWindow *)user_data;
  nassertr(window != nullptr, false);

  if (type == EMSCRIPTEN_EVENT_FULLSCREENCHANGE) {
    WindowProperties props;
    props.set_fullscreen(event->isFullscreen);
    window->system_changed_properties(props);
    return true;
  }

  return false;
}

/**
 *
 */
EM_BOOL WebGLGraphicsWindow::
on_pointerlock_event(int type, const EmscriptenPointerlockChangeEvent *event, void *user_data) {
  WebGLGraphicsWindow *window = (WebGLGraphicsWindow *)user_data;
  nassertr(window != nullptr, false);

  if (type == EMSCRIPTEN_EVENT_POINTERLOCKCHANGE) {
    WindowProperties props;
    if (event->isActive) {
      std::cout << "pointerlock engaged\n";
      props.set_mouse_mode(WindowProperties::M_relative);
      props.set_cursor_hidden(true);
    } else {
      std::cout << "pointerlock disabled\n";
      props.set_mouse_mode(WindowProperties::M_absolute);
      props.set_cursor_hidden(false);
    }
    window->system_changed_properties(props);
    return true;
  }

  return false;
}

/**
 *
 */
EM_BOOL WebGLGraphicsWindow::
on_visibility_event(int type, const EmscriptenVisibilityChangeEvent *event, void *user_data) {
  WebGLGraphicsWindow *window = (WebGLGraphicsWindow *)user_data;
  nassertr(window != nullptr, false);

  if (type == EMSCRIPTEN_EVENT_VISIBILITYCHANGE) {
    WindowProperties props;
    props.set_minimized(event->hidden != 0);
    window->system_changed_properties(props);
    return true;
  }

  return false;
}

/**
 *
 */
EM_BOOL WebGLGraphicsWindow::
on_focus_event(int type, const EmscriptenFocusEvent *event, void *user_data) {
  WebGLGraphicsWindow *window = (WebGLGraphicsWindow *)user_data;
  nassertr(window != nullptr, false);

  if (type == EMSCRIPTEN_EVENT_FOCUS) {
    WindowProperties props;
    props.set_foreground(true);
    window->system_changed_properties(props);
    return true;
  } else if (type == EMSCRIPTEN_EVENT_BLUR) {
    WindowProperties props;
    props.set_foreground(false);
    window->system_changed_properties(props);
    return true;
  }

  return false;
}

/**
 * Handles HTML5 keypress, keydown and keyup events.
 */
EM_BOOL WebGLGraphicsWindow::
on_keyboard_event(int type, const EmscriptenKeyboardEvent *event, void *user_data) {
  GraphicsWindowInputDevice *device;
  device = (GraphicsWindowInputDevice *)user_data;
  nassertr(device != nullptr, false);

  if (type == EMSCRIPTEN_EVENT_KEYPRESS) {
    // We have to use String.fromCharCode to turn this into a text character.
    // When called from a key event, it does some special magic to ensure that
    // it does the right thing.  We grab the first unicode code point.
    // Unfortunately, this doesn't seem to handle dead keys on Firefox.
    int keycode = 0;
    keycode = EM_ASM_INT({
      return String.fromCharCode($0).codePointAt(0);
    }, event->charCode);

    if (keycode != 0) {
      device->keystroke(keycode);
      return true;
    }

  } else if (type == EMSCRIPTEN_EVENT_KEYDOWN ||
             type == EMSCRIPTEN_EVENT_KEYUP) {

    ButtonHandle handle = map_key(event->which);

    // Send a raw event too, if the browser supports providing it.
    ButtonHandle raw_handle;
    if (event->code[0]) {
      raw_handle = map_raw_key(event->code);
    } else {
      // This browser doesn't send raw events.  Let's just pretend the user is
      // using QWERTY.  Better than nothing?
      raw_handle = handle;
    }

    if (raw_handle != ButtonHandle::none()) {
      if (type == EMSCRIPTEN_EVENT_KEYUP) {
        device->raw_button_up(raw_handle);
      } else if (!event->repeat) {
        device->raw_button_down(raw_handle);
      }
    }

    // Send a regular event for the 'virtual' key mapping.
    if (handle != ButtonHandle::none()) {
      if (type == EMSCRIPTEN_EVENT_KEYUP) {
        //webgldisplay_cat.info() << "button up " << handle << "\n";
        device->button_up(handle);
      } else if (event->repeat) {
        device->button_resume_down(handle);
      } else {
        //webgldisplay_cat.info() << "button down " << handle << "\n";
        device->button_down(handle);
      }

      // If we preventDefault a keydown event, its keypress event won't fire,
      // which would prevent text input from working.  However, we must still
      // prevent the default action from working in backspace and tab events.
      if (type == EMSCRIPTEN_EVENT_KEYDOWN &&
          event->keyCode != 8 && event->keyCode != 9) {
        return false;
      } else {
        return true;
      }
    } else if (event->which != 0) {
      webgldisplay_cat.info()
        << "unhandled event code " << event->which << "\n";
    }
  }

  return false;
}

/**
 * Handles mousedown, mouseup, mousemove, mouseenter and mouseleave events.
 */
EM_BOOL WebGLGraphicsWindow::
on_mouse_event(int type, const EmscriptenMouseEvent *event, void *user_data) {
  GraphicsWindowInputDevice *device;
  device = (GraphicsWindowInputDevice *)user_data;
  nassertr(device != nullptr, false);

  const char *target = "#canvas";

  switch (type) {
  case EMSCRIPTEN_EVENT_MOUSEDOWN:
    // Don't register out-of-bounds mouse downs.
    if (event->targetX >= 0 && event->targetY >= 0) {
      int w, h;
      emscripten_get_canvas_element_size(target, &w, &h);
      if (event->targetX < w && event->targetY < h) {
        device->button_down(MouseButton::button(event->button));
        return true;
      }
    }
    return false;

  case EMSCRIPTEN_EVENT_MOUSEUP:
    device->button_up(MouseButton::button(event->button));
    return true;

  case EMSCRIPTEN_EVENT_MOUSEMOVE:
    {
      EmscriptenPointerlockChangeEvent ev;
      emscripten_get_pointerlock_status(&ev);

      if (ev.isActive) {
        PointerData md = device->get_pointer();
        device->set_pointer_in_window(md.get_x() + event->movementX,
                                      md.get_y() + event->movementY);
      } else {
        device->set_pointer_in_window(event->targetX, event->targetY);
      }
    }
    return true;

  case EMSCRIPTEN_EVENT_MOUSEENTER:
    break;

  case EMSCRIPTEN_EVENT_MOUSELEAVE:
    device->set_pointer_out_of_window();
    return true;

  default:
    break;
  }

  return false;
}

/**
 *
 */
EM_BOOL WebGLGraphicsWindow::
on_wheel_event(int type, const EmscriptenWheelEvent *event, void *user_data) {
  GraphicsWindowInputDevice *device;
  device = (GraphicsWindowInputDevice *)user_data;
  nassertr(device != nullptr, false);

  if (type == EMSCRIPTEN_EVENT_WHEEL) {
    if (event->deltaY < 0) {
      device->button_down(MouseButton::wheel_up());
      device->button_up(MouseButton::wheel_up());
    }
    if (event->deltaY > 0) {
      device->button_down(MouseButton::wheel_down());
      device->button_up(MouseButton::wheel_down());
    }
    return true;
  }

  return false;
}


/**
 * Maps a JavaScript event keycode to a ButtonHandle.
 */
ButtonHandle WebGLGraphicsWindow::
map_key(int which) {
  switch (which) {
  case 8:
    return KeyboardButton::backspace();
  case 9:
    return KeyboardButton::tab();
  case 13:
    return KeyboardButton::enter();
  case 16:
    return KeyboardButton::shift();
  case 17:
    return KeyboardButton::control();
  case 18:
    return KeyboardButton::alt();
  case 19:
    return KeyboardButton::pause();
  case 20:
    return KeyboardButton::caps_lock();
  case 27:
    return KeyboardButton::escape();
  case 32:
    return KeyboardButton::space();
  case 33:
    return KeyboardButton::page_up();
  case 34:
    return KeyboardButton::page_down();
  case 35:
    return KeyboardButton::end();
  case 36:
    return KeyboardButton::home();
  case 37:
    return KeyboardButton::left();
  case 38:
    return KeyboardButton::up();
  case 39:
    return KeyboardButton::right();
  case 40:
    return KeyboardButton::down();
  case 42:
    return KeyboardButton::print_screen();
  case 45:
    return KeyboardButton::insert();
  case 46:
    return KeyboardButton::del();
  case 48:
  case 49:
  case 50:
  case 51:
  case 52:
  case 53:
  case 54:
  case 55:
  case 56:
  case 57:
    return KeyboardButton::ascii_key('0' + (which - 48));
  case 59:
    return KeyboardButton::ascii_key(';');
  case 61:
    return KeyboardButton::ascii_key('=');
  case 65:
  case 66:
  case 67:
  case 68:
  case 69:
  case 70:
  case 71:
  case 72:
  case 73:
  case 74:
  case 75:
  case 76:
  case 77:
  case 78:
  case 79:
  case 80:
  case 81:
  case 82:
  case 83:
  case 84:
  case 85:
  case 86:
  case 87:
  case 88:
  case 89:
  case 90:
    return KeyboardButton::ascii_key('a' + (which - 65));
  case 91:
    return KeyboardButton::lmeta();
  case 92:
    return KeyboardButton::rmeta();
  case 93:
    return KeyboardButton::menu();
  case 96:
  case 97:
  case 98:
  case 99:
  case 100:
  case 101:
  case 102:
  case 103:
  case 104:
  case 105:
    return KeyboardButton::ascii_key('0' + (which - 96));
  case 106:
    return KeyboardButton::ascii_key('*');
  case 107:
    return KeyboardButton::ascii_key('+');
  case 109:
    return KeyboardButton::ascii_key('-');
  case 110:
    return KeyboardButton::ascii_key('.');
  case 111:
    return KeyboardButton::ascii_key('/');
  case 112:
    return KeyboardButton::f1();
  case 113:
    return KeyboardButton::f2();
  case 114:
    return KeyboardButton::f3();
  case 115:
    return KeyboardButton::f4();
  case 116:
    return KeyboardButton::f5();
  case 117:
    return KeyboardButton::f6();
  case 118:
    return KeyboardButton::f7();
  case 119:
    return KeyboardButton::f8();
  case 120:
    return KeyboardButton::f9();
  case 121:
    return KeyboardButton::f10();
  case 122:
    return KeyboardButton::f11();
  case 123:
    return KeyboardButton::f12();
  case 144:
    return KeyboardButton::num_lock();
  case 145:
    return KeyboardButton::scroll_lock();
  case 173:
    return KeyboardButton::ascii_key('-');
  case 186:
    return KeyboardButton::ascii_key(';');
  case 187:
    return KeyboardButton::ascii_key('=');
  case 188:
    return KeyboardButton::ascii_key(',');
  case 189:
    return KeyboardButton::ascii_key('-');
  case 190:
    return KeyboardButton::ascii_key('.');
  case 191:
    return KeyboardButton::ascii_key('?');
  case 192:
    return KeyboardButton::ascii_key('`');
  case 219:
    return KeyboardButton::ascii_key('[');
  case 220:
    return KeyboardButton::ascii_key('\\');
  case 221:
    return KeyboardButton::ascii_key(']');
  case 222:
    return KeyboardButton::ascii_key('\'');
  default:
    return ButtonHandle::none();
  }
}

/**
 * Maps a HTML5 KeyboardEvent.code string to a ButtonHandle.
 */
ButtonHandle WebGLGraphicsWindow::
map_raw_key(const char *code) {
  static struct {
    const char *code;
    ButtonHandle handle;
  } mappings[] = {
    //{"Again", KeyboardButton::()},
    {"AltLeft", KeyboardButton::lalt()},
    {"AltRight", KeyboardButton::ralt()},
    {"ArrowDown", KeyboardButton::down()},
    {"ArrowLeft", KeyboardButton::left()},
    {"ArrowRight", KeyboardButton::right()},
    {"ArrowUp", KeyboardButton::up()},
    {"Backquote", KeyboardButton::ascii_key('`')},
    {"Backslash", KeyboardButton::ascii_key('\\')},
    {"Backspace", KeyboardButton::backspace()},
    {"BracketLeft", KeyboardButton::ascii_key('[')},
    {"BracketRight", KeyboardButton::ascii_key(']')},
    //{"BrowserBack", KeyboardButton::()},
    //{"BrowserFavorites", KeyboardButton::()},
    //{"BrowserForward", KeyboardButton::()},
    //{"BrowserRefresh", KeyboardButton::()},
    //{"BrowserSearch", KeyboardButton::()},
    //{"BrowserStop", KeyboardButton::()},
    {"CapsLock", KeyboardButton::caps_lock()},
    {"Comma", KeyboardButton::ascii_key(',')},
    {"ContextMenu", KeyboardButton::menu()},
    {"ControlLeft", KeyboardButton::lcontrol()},
    {"ControlRight", KeyboardButton::rcontrol()},
    //{"Convert", KeyboardButton::()},
    //{"Copy", KeyboardButton::()},
    //{"Cut", KeyboardButton::()},
    {"Delete", KeyboardButton::del()},
    {"Digit0", KeyboardButton::ascii_key('0')},
    {"Digit1", KeyboardButton::ascii_key('1')},
    {"Digit2", KeyboardButton::ascii_key('2')},
    {"Digit3", KeyboardButton::ascii_key('3')},
    {"Digit4", KeyboardButton::ascii_key('4')},
    {"Digit5", KeyboardButton::ascii_key('5')},
    {"Digit6", KeyboardButton::ascii_key('6')},
    {"Digit7", KeyboardButton::ascii_key('7')},
    {"Digit8", KeyboardButton::ascii_key('8')},
    {"Digit9", KeyboardButton::ascii_key('9')},
    //{"Eject", KeyboardButton::()},
    {"End", KeyboardButton::end()},
    {"Enter", KeyboardButton::enter()},
    {"Equal", KeyboardButton::ascii_key('=')},
    {"Escape", KeyboardButton::escape()},
    {"F1", KeyboardButton::f1()},
    {"F10", KeyboardButton::f10()},
    {"F11", KeyboardButton::f11()},
    {"F12", KeyboardButton::f12()},
    {"F13", KeyboardButton::f13()},
    {"F14", KeyboardButton::f14()},
    {"F15", KeyboardButton::f15()},
    {"F16", KeyboardButton::f16()},
    //{"F17", KeyboardButton::f17()},
    //{"F18", KeyboardButton::f18()},
    //{"F19", KeyboardButton::f19()},
    {"F2", KeyboardButton::f2()},
    //{"F20", KeyboardButton::f20()},
    //{"F21", KeyboardButton::f21()},
    //{"F22", KeyboardButton::f22()},
    //{"F23", KeyboardButton::f23()},
    //{"F24", KeyboardButton::f24()},
    {"F3", KeyboardButton::f3()},
    {"F4", KeyboardButton::f4()},
    {"F5", KeyboardButton::f5()},
    {"F6", KeyboardButton::f6()},
    {"F7", KeyboardButton::f7()},
    {"F8", KeyboardButton::f8()},
    {"F9", KeyboardButton::f9()},
    //{"Find", KeyboardButton::()},
    //{"Fn", KeyboardButton::()},
    {"Help", KeyboardButton::help()},
    {"Home", KeyboardButton::home()},
    {"Insert", KeyboardButton::insert()},
    //{"IntlBackslash", KeyboardButton::()},
    //{"IntlRo", KeyboardButton::()},
    //{"IntlYen", KeyboardButton::()},
    //{"KanaMode", KeyboardButton::()},
    {"KeyA", KeyboardButton::ascii_key('a')},
    {"KeyB", KeyboardButton::ascii_key('b')},
    {"KeyC", KeyboardButton::ascii_key('c')},
    {"KeyD", KeyboardButton::ascii_key('d')},
    {"KeyE", KeyboardButton::ascii_key('e')},
    {"KeyF", KeyboardButton::ascii_key('f')},
    {"KeyG", KeyboardButton::ascii_key('g')},
    {"KeyH", KeyboardButton::ascii_key('h')},
    {"KeyI", KeyboardButton::ascii_key('i')},
    {"KeyJ", KeyboardButton::ascii_key('j')},
    {"KeyK", KeyboardButton::ascii_key('k')},
    {"KeyL", KeyboardButton::ascii_key('l')},
    {"KeyM", KeyboardButton::ascii_key('m')},
    {"KeyN", KeyboardButton::ascii_key('n')},
    {"KeyO", KeyboardButton::ascii_key('o')},
    {"KeyP", KeyboardButton::ascii_key('p')},
    {"KeyQ", KeyboardButton::ascii_key('q')},
    {"KeyR", KeyboardButton::ascii_key('r')},
    {"KeyS", KeyboardButton::ascii_key('s')},
    {"KeyT", KeyboardButton::ascii_key('t')},
    {"KeyU", KeyboardButton::ascii_key('u')},
    {"KeyV", KeyboardButton::ascii_key('v')},
    {"KeyW", KeyboardButton::ascii_key('w')},
    {"KeyX", KeyboardButton::ascii_key('x')},
    {"KeyY", KeyboardButton::ascii_key('y')},
    {"KeyZ", KeyboardButton::ascii_key('z')},
    //{"Lang1", KeyboardButton::()},
    //{"Lang2", KeyboardButton::()},
    //{"LaunchApp1", KeyboardButton::()},
    //{"MediaPlayPause", KeyboardButton::()},
    //{"MediaStop", KeyboardButton::()},
    //{"MediaTrackNext", KeyboardButton::()},
    //{"MediaTrackPrevious", KeyboardButton::()},
    {"Minus", KeyboardButton::ascii_key('-')},
    //{"NonConvert", KeyboardButton::()},
    {"NumLock", KeyboardButton::num_lock()},
    {"Numpad0", KeyboardButton::ascii_key('0')},
    {"Numpad1", KeyboardButton::ascii_key('1')},
    {"Numpad2", KeyboardButton::ascii_key('2')},
    {"Numpad3", KeyboardButton::ascii_key('3')},
    {"Numpad4", KeyboardButton::ascii_key('4')},
    {"Numpad5", KeyboardButton::ascii_key('5')},
    {"Numpad6", KeyboardButton::ascii_key('6')},
    {"Numpad7", KeyboardButton::ascii_key('7')},
    {"Numpad8", KeyboardButton::ascii_key('8')},
    {"Numpad9", KeyboardButton::ascii_key('9')},
    {"NumpadAdd", KeyboardButton::ascii_key('+')},
    {"NumpadComma", KeyboardButton::ascii_key(',')},
    {"NumpadDecimal", KeyboardButton::ascii_key('.')},
    {"NumpadDivide", KeyboardButton::ascii_key('/')},
    {"NumpadEnter", KeyboardButton::enter()},
    {"NumpadEqual", KeyboardButton::ascii_key('=')},
    {"NumpadMultiply", KeyboardButton::ascii_key('*')},
    {"NumpadSubtract", KeyboardButton::ascii_key('-')},
    //{"Open", KeyboardButton::()},
    {"OSLeft", KeyboardButton::lmeta()},
    {"OSRight", KeyboardButton::rmeta()},
    {"PageDown", KeyboardButton::page_down()},
    {"PageUp", KeyboardButton::page_up()},
    //{"Paste", KeyboardButton::()},
    {"Pause", KeyboardButton::pause()},
    {"Period", KeyboardButton::ascii_key('.')},
    //{"Power", KeyboardButton::()},
    {"PrintScreen", KeyboardButton::print_screen()},
    //{"Props", KeyboardButton::()},
    {"Quote", KeyboardButton::ascii_key('\'')},
    {"ScrollLock", KeyboardButton::scroll_lock()},
    //{"Select", KeyboardButton::()},
    {"Semicolon", KeyboardButton::ascii_key(';')},
    {"ShiftLeft", KeyboardButton::lshift()},
    {"ShiftRight", KeyboardButton::rshift()},
    {"Slash", KeyboardButton::ascii_key('/')},
    //{"Sleep", KeyboardButton::()},
    {"Space", KeyboardButton::ascii_key(' ')},
    {"Tab", KeyboardButton::tab()},
    //{"Undo", KeyboardButton::()},
    //{"VolumeDown", KeyboardButton::()},
    //{"VolumeMute", KeyboardButton::()},
    //{"VolumeUp", KeyboardButton::()},
    //{"WakeUp", KeyboardButton::()},
    {nullptr, ButtonHandle::none()}
  };

  for (int i = 0; mappings[i].code; ++i) {
    int cmp = strcmp(mappings[i].code, code);
    if (cmp == 0) {
      return mappings[i].handle;
    } else if (cmp > 0) {
      // They're in alphabetical order, and we've passed it by, so bail early.
      return ButtonHandle::none();
    }
  }

  return ButtonHandle::none();
}
