/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file androidGraphicsWindow.cxx
 * @author rdb
 * @date 2013-01-11
 */

#include "androidGraphicsWindow.h"
#include "androidGraphicsStateGuardian.h"
#include "config_androiddisplay.h"
#include "androidGraphicsPipe.h"

#include "graphicsPipe.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "clockObject.h"
#include "pStatTimer.h"
#include "textEncoder.h"
#include "throw_event.h"
#include "nativeWindowHandle.h"

#include "android_native_app_glue.h"
#include <android/window.h>
#include <android/log.h>

extern IMPORT_CLASS struct android_app* panda_android_app;

TypeHandle AndroidGraphicsWindow::_type_handle;

/**
 *
 */
AndroidGraphicsWindow::
AndroidGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                      const std::string &name,
                      const FrameBufferProperties &fb_prop,
                      const WindowProperties &win_prop,
                      int flags,
                      GraphicsStateGuardian *gsg,
                      GraphicsOutput *host) :
  GraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host),
  _mouse_button_state(0)
{
  AndroidGraphicsPipe *android_pipe;
  DCAST_INTO_V(android_pipe, _pipe);

  _egl_display = android_pipe->_egl_display;
  _egl_surface = 0;

  _app = panda_android_app;

  PT(GraphicsWindowInputDevice) device = GraphicsWindowInputDevice::pointer_and_keyboard(this, "keyboard_mouse");
  add_input_device(device);
  _input = device;
}

/**
 *
 */
AndroidGraphicsWindow::
~AndroidGraphicsWindow() {
  destroy_surface();
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool AndroidGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  PStatTimer timer(_make_current_pcollector, current_thread);

  begin_frame_spam(mode);
  if (_gsg == nullptr) {
    return false;
  }

  // XXX not open yet.
  if (_egl_surface == EGL_NO_SURFACE) {
    return false;
  }

  AndroidGraphicsStateGuardian *androidgsg;
  DCAST_INTO_R(androidgsg, _gsg, false);
  {
    if (eglGetCurrentDisplay() == _egl_display &&
        eglGetCurrentSurface(EGL_READ) == _egl_surface &&
        eglGetCurrentSurface(EGL_DRAW) == _egl_surface &&
        eglGetCurrentContext() == androidgsg->_context) {
      // No need to make the context current again.  Short-circuit this
      // possibly-expensive call.
    } else {
      // Need to set the context.
      if (!eglMakeCurrent(_egl_display, _egl_surface, _egl_surface, androidgsg->_context)) {
        androiddisplay_cat.error() << "Failed to call eglMakeCurrent: "
          << get_egl_error_string(eglGetError()) << "\n";
      }
    }
  }

  // Now that we have made the context current to a window, we can reset the
  // GSG state if this is the first time it has been used.  (We can't just
  // call reset() when we construct the GSG, because reset() requires having a
  // current context.)
  androidgsg->reset_if_new();

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
void AndroidGraphicsWindow::
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
void AndroidGraphicsWindow::
end_flip() {
  if (_gsg != nullptr && _flip_ready) {

    // It doesn't appear to be necessary to ensure the graphics context is
    // current before flipping the windows, and insisting on doing so can be a
    // significant performance hit.

    // make_current();

    if (_egl_surface != EGL_NO_SURFACE) {
      eglSwapBuffers(_egl_display, _egl_surface);
    }
  }
  GraphicsWindow::end_flip();
}

/**
 * Do whatever processing is necessary to ensure that the window responds to
 * user events.  Also, honor any requests recently made via
 * request_properties()
 *
 * This function is called only within the window thread.
 */
void AndroidGraphicsWindow::
process_events() {
  GraphicsWindow::process_events();

  // Read all pending events.
  int looper_id;
  int events;
  struct android_poll_source* source;

  // Loop until all events are read.
  while ((looper_id = ALooper_pollAll(0, nullptr, &events, (void**)&source)) >= 0) {
    // Process this event.
    if (source != nullptr) {
      source->process(_app, source);
    }
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
void AndroidGraphicsWindow::
set_properties_now(WindowProperties &properties) {
  if (_pipe == nullptr) {
    // If the pipe is null, we're probably closing down.
    GraphicsWindow::set_properties_now(properties);
    return;
  }

  GraphicsWindow::set_properties_now(properties);
  if (!properties.is_any_specified()) {
    // The base class has already handled this case.
    return;
  }

  // There's not really much we can change on Android.
  if (properties.has_fullscreen()) {
    uint32_t add_flags = 0;
    uint32_t del_flags = 0;
    if (_properties.get_fullscreen()) {
      add_flags |= AWINDOW_FLAG_FULLSCREEN;
    } else {
      del_flags |= AWINDOW_FLAG_FULLSCREEN;
    }
    ANativeActivity_setWindowFlags(_app->activity, add_flags, del_flags);

    _properties.set_fullscreen(properties.get_fullscreen());
    properties.clear_fullscreen();
  }
}

/**
 * Closes the window right now.  Called from the window thread.
 */
void AndroidGraphicsWindow::
close_window() {
  destroy_surface();

  if (_gsg != nullptr) {
    _gsg.clear();
  }

  GraphicsWindow::close_window();

  nassertv(_app != nullptr);
  if (_app->userData == this) {
    _app->userData = nullptr;
    _app->onAppCmd = nullptr;
    _app->onInputEvent = nullptr;
  }
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool AndroidGraphicsWindow::
open_window() {
  // GSG CreationInitialization
  AndroidGraphicsStateGuardian *androidgsg;
  if (_gsg == 0) {
    // There is no old gsg.  Create a new one.
    androidgsg = new AndroidGraphicsStateGuardian(_engine, _pipe, nullptr);
    androidgsg->choose_pixel_format(_fb_properties, false, false);
    _gsg = androidgsg;
  } else {
    // If the old gsg has the wrong pixel format, create a new one that shares
    // with the old gsg.
    DCAST_INTO_R(androidgsg, _gsg, false);
    if (!androidgsg->get_fb_properties().subsumes(_fb_properties)) {
      androidgsg = new AndroidGraphicsStateGuardian(_engine, _pipe, androidgsg);
      androidgsg->choose_pixel_format(_fb_properties, false, false);
      _gsg = androidgsg;
    }
  }

  // Register the callbacks
  assert(_app != nullptr);
  _app->userData = this;
  _app->onAppCmd = handle_command;
  _app->onInputEvent = handle_input_event;

  // Wait until Android has opened the window.
  while (_app->window == nullptr) {
    process_events();
  }

  // create_surface should have been called by now.
  if (_egl_surface == EGL_NO_SURFACE) {
    return false;
  }

  // Set some other properties.
  _properties.set_origin(0, 0);
  _properties.set_cursor_hidden(true);
  _properties.set_undecorated(true);

  if (!androidgsg->get_fb_properties().verify_hardware_software
      (_fb_properties, androidgsg->get_gl_renderer())) {
    close_window();
    return false;
  }

  _fb_properties = androidgsg->get_fb_properties();

  return true;
}

/**
 * Terminates the EGL surface.
 */
void AndroidGraphicsWindow::
destroy_surface() {
  if (_egl_surface != EGL_NO_SURFACE) {
    if (!eglDestroySurface(_egl_display, _egl_surface)) {
      androiddisplay_cat.error() << "Failed to destroy surface: "
        << get_egl_error_string(eglGetError()) << "\n";
    }
    _egl_surface = EGL_NO_SURFACE;
  }

  // Destroy the current context.
  if (_gsg != nullptr) {
    AndroidGraphicsStateGuardian *androidgsg;
    DCAST_INTO_V(androidgsg, _gsg);
    androidgsg->destroy_context();
  }
}

/**
 * Creates the EGL surface.
 */
bool AndroidGraphicsWindow::
create_surface() {
  AndroidGraphicsStateGuardian *androidgsg;
  DCAST_INTO_R(androidgsg, _gsg, false);

  // Reconfigure the window buffers to match that of our framebuffer config.
  ANativeWindow_setBuffersGeometry(_app->window, 0, 0, androidgsg->_format);

  // Set any window flags
  uint32_t add_flags = 0;
  uint32_t del_flags = 0;
  if (_properties.get_fullscreen()) {
    add_flags |= AWINDOW_FLAG_FULLSCREEN;
  } else {
    del_flags |= AWINDOW_FLAG_FULLSCREEN;
  }
  ANativeActivity_setWindowFlags(_app->activity, add_flags, del_flags);

  // Create the EGL surface.
  _egl_surface = eglCreateWindowSurface(_egl_display, androidgsg->_fbconfig, _app->window, nullptr);
  if (eglGetError() != EGL_SUCCESS) {
    androiddisplay_cat.error()
      << "Failed to create window surface.\n";
    return false;
  }

  // Create a context.
  if (androidgsg->_context == EGL_NO_CONTEXT) {
    if (!androidgsg->create_context()) {
      return false;
    }
  }

  // Switch to our newly created context.
  if (!eglMakeCurrent(_egl_display, _egl_surface, _egl_surface, androidgsg->_context)) {
    androiddisplay_cat.error() << "Failed to call eglMakeCurrent: "
      << get_egl_error_string(eglGetError()) << "\n";
  }

  // Query the size of the surface.  EGLint width, height;
  // eglQuerySurface(_egl_display, _egl_surface, EGL_WIDTH, &width);
  // eglQuerySurface(_egl_display, _egl_surface, EGL_HEIGHT, &height);

  androidgsg->reset_if_new();
  if (!androidgsg->is_valid()) {
    close_window();
    return false;
  }

  return true;
}

/**
 * Android app sends a command from the main thread.
 */
void AndroidGraphicsWindow::
handle_command(struct android_app *app, int32_t command) {
  AndroidGraphicsWindow *window = (AndroidGraphicsWindow *)app->userData;
  if (window != nullptr) {
    window->ns_handle_command(command);
  }
}

/**
 * Android app sends a command from the main thread.
 */
void AndroidGraphicsWindow::
ns_handle_command(int32_t command) {
  WindowProperties properties;

  switch (command) {
    case APP_CMD_SAVE_STATE:
      // The system has asked us to save our current state.  Do so.
      // engine->app->savedState = malloc(sizeof(struct saved_state));
      // *((struct saved_state*)engine->app->savedState) = engine->state;
      // engine->app->savedStateSize = sizeof(struct saved_state);
      break;
    case APP_CMD_INIT_WINDOW:
      // The window is being shown, get it ready.
      if (_app->window != nullptr) {
        create_surface();
        properties.set_size(ANativeWindow_getWidth(_app->window),
                            ANativeWindow_getHeight(_app->window));
        properties.set_minimized(false);
        system_changed_properties(properties);
      }
      break;
    case APP_CMD_CONFIG_CHANGED:
      properties.set_size(ANativeWindow_getWidth(_app->window),
                          ANativeWindow_getHeight(_app->window));
      system_changed_properties(properties);
      break;
    case APP_CMD_TERM_WINDOW:
      destroy_surface();
      properties.set_minimized(true);
      system_changed_properties(properties);
      break;
    case APP_CMD_WINDOW_RESIZED:
      properties.set_size(ANativeWindow_getWidth(_app->window),
                          ANativeWindow_getHeight(_app->window));
      break;
    case APP_CMD_WINDOW_REDRAW_NEEDED:
      break;
    case APP_CMD_CONTENT_RECT_CHANGED:
      properties.set_origin(_app->contentRect.left, _app->contentRect.top);
      properties.set_size(_app->contentRect.right - _app->contentRect.left,
                          _app->contentRect.bottom - _app->contentRect.top);
      system_changed_properties(properties);
      break;
    case APP_CMD_GAINED_FOCUS:
      properties.set_foreground(true);
      system_changed_properties(properties);
      break;
    case APP_CMD_LOST_FOCUS:
      properties.set_foreground(false);
      system_changed_properties(properties);
      break;
    case APP_CMD_DESTROY:
      close_window();
      properties.set_open(false);
      system_changed_properties(properties);
      break;
  }
}

/**
 * Processes an input event.  Returns 1 if the event was handled, 0 otherwise.
 */
int32_t AndroidGraphicsWindow::
handle_input_event(struct android_app* app, AInputEvent *event) {
  AndroidGraphicsWindow* window = (AndroidGraphicsWindow*) app->userData;

  int32_t event_type = AInputEvent_getType(event);
  switch (event_type) {
  case AINPUT_EVENT_TYPE_KEY:
    return window->handle_key_event(event);
  case AINPUT_EVENT_TYPE_MOTION:
    return window->handle_motion_event(event);
  }
  return 0;
}

/**
 * Processes a key event.
 */
int32_t AndroidGraphicsWindow::
handle_key_event(const AInputEvent *event) {
  /*
  int32_t meta = AKeyEvent_getMetaState(event);
  if (meta | AMETA_ALT_ON) {
    _input->button_down(KeyboardButton.alt());
  }
  if (meta | AMETA_ALT_LEFT_ON) {
    _input->button_down(KeyboardButton.lalt());
  }
  if (meta | AMETA_ALT_RIGHT_ON) {
    _input->button_down(KeyboardButton.ralt());
  }
  if (meta | AMETA_SHIFT_ON) {
    _input->button_down(KeyboardButton.shift());
  }
  if (meta | AMETA_SHIFT_LEFT_ON) {
    _input->button_down(KeyboardButton.lshift());
  }
  if (meta | AMETA_SHIFT_RIGHT_ON) {
    _input->button_down(KeyboardButton.rshift());
  }*/

  int32_t keycode = AKeyEvent_getKeyCode(event);
  ButtonHandle button = map_button(keycode);

  if (button == ButtonHandle::none()) {
    androiddisplay_cat.warning()
      << "Unknown keycode: " << keycode << "\n";
    return 0;
  }

  // Is it an up or down event?
  int32_t action = AKeyEvent_getAction(event);
  if (action == AKEY_EVENT_ACTION_DOWN) {
    if (AKeyEvent_getRepeatCount(event) > 0) {
      _input->button_resume_down(button);
    } else {
      _input->button_down(button);
    }
  } else if (action == AKEY_EVENT_ACTION_UP) {
    _input->button_up(button);
  }
  // TODO AKEY_EVENT_ACTION_MULTIPLE

  return 1;
}

/**
 * Processes a motion event.
 */
int32_t AndroidGraphicsWindow::
handle_motion_event(const AInputEvent *event) {
  int32_t action = AMotionEvent_getAction(event);
  action &= AMOTION_EVENT_ACTION_MASK;

  if (action == AMOTION_EVENT_ACTION_DOWN ||
      action == AMOTION_EVENT_ACTION_UP) {
    // The up event doesn't let us know which button is up, so we need to
    // keep track of the button state ourselves.
    int32_t button_state = AMotionEvent_getButtonState(event);
    if (button_state == 0 && action == AMOTION_EVENT_ACTION_DOWN) {
      button_state = AMOTION_EVENT_BUTTON_PRIMARY;
    }
    int32_t changed = _mouse_button_state ^ button_state;
    if (changed != 0) {
      if (changed & AMOTION_EVENT_BUTTON_PRIMARY) {
        if (button_state & AMOTION_EVENT_BUTTON_PRIMARY) {
          _input->button_down(MouseButton::one());
        } else {
          _input->button_up(MouseButton::one());
        }
      }
      if (changed & AMOTION_EVENT_BUTTON_SECONDARY) {
        if (button_state & AMOTION_EVENT_BUTTON_SECONDARY) {
          _input->button_down(MouseButton::three());
        } else {
          _input->button_up(MouseButton::three());
        }
      }
      _mouse_button_state = button_state;
    }
  }

  float x = AMotionEvent_getX(event, 0) - _app->contentRect.left;
  float y = AMotionEvent_getY(event, 0) - _app->contentRect.top;

  _input->set_pointer_in_window(x, y);

  return 1;
}

/**
 * Given an Android keycode, returns an appropriate ButtonHandle object, or
 * ButtonHandle::none() if a matching ButtonHandle does not exist.
 */
ButtonHandle AndroidGraphicsWindow::
map_button(int32_t keycode) {
  switch (keycode) {
    case AKEYCODE_SOFT_LEFT:
    case AKEYCODE_SOFT_RIGHT:
    case AKEYCODE_HOME:
    case AKEYCODE_BACK:
    case AKEYCODE_CALL:
    case AKEYCODE_ENDCALL:
      break;
    case AKEYCODE_0:
      return KeyboardButton::ascii_key('0');
    case AKEYCODE_1:
      return KeyboardButton::ascii_key('1');
    case AKEYCODE_2:
      return KeyboardButton::ascii_key('2');
    case AKEYCODE_3:
      return KeyboardButton::ascii_key('3');
    case AKEYCODE_4:
      return KeyboardButton::ascii_key('4');
    case AKEYCODE_5:
      return KeyboardButton::ascii_key('5');
    case AKEYCODE_6:
      return KeyboardButton::ascii_key('6');
    case AKEYCODE_7:
      return KeyboardButton::ascii_key('7');
    case AKEYCODE_8:
      return KeyboardButton::ascii_key('8');
    case AKEYCODE_9:
      return KeyboardButton::ascii_key('9');
    case AKEYCODE_STAR:
      return KeyboardButton::ascii_key('*');
    case AKEYCODE_POUND:
      return KeyboardButton::ascii_key('#');
    case AKEYCODE_DPAD_UP:
      return KeyboardButton::up();
    case AKEYCODE_DPAD_DOWN:
      return KeyboardButton::down();
    case AKEYCODE_DPAD_LEFT:
      return KeyboardButton::left();
    case AKEYCODE_DPAD_RIGHT:
      return KeyboardButton::right();
    case AKEYCODE_DPAD_CENTER:
    case AKEYCODE_VOLUME_UP:
    case AKEYCODE_VOLUME_DOWN:
    case AKEYCODE_POWER:
    case AKEYCODE_CAMERA:
    case AKEYCODE_CLEAR:
      break;
    case AKEYCODE_A:
      return KeyboardButton::ascii_key('a');
    case AKEYCODE_B:
      return KeyboardButton::ascii_key('b');
    case AKEYCODE_C:
      return KeyboardButton::ascii_key('c');
    case AKEYCODE_D:
      return KeyboardButton::ascii_key('d');
    case AKEYCODE_E:
      return KeyboardButton::ascii_key('e');
    case AKEYCODE_F:
      return KeyboardButton::ascii_key('f');
    case AKEYCODE_G:
      return KeyboardButton::ascii_key('g');
    case AKEYCODE_H:
      return KeyboardButton::ascii_key('h');
    case AKEYCODE_I:
      return KeyboardButton::ascii_key('i');
    case AKEYCODE_J:
      return KeyboardButton::ascii_key('j');
    case AKEYCODE_K:
      return KeyboardButton::ascii_key('k');
    case AKEYCODE_L:
      return KeyboardButton::ascii_key('l');
    case AKEYCODE_M:
      return KeyboardButton::ascii_key('m');
    case AKEYCODE_N:
      return KeyboardButton::ascii_key('n');
    case AKEYCODE_O:
      return KeyboardButton::ascii_key('o');
    case AKEYCODE_P:
      return KeyboardButton::ascii_key('p');
    case AKEYCODE_Q:
      return KeyboardButton::ascii_key('q');
    case AKEYCODE_R:
      return KeyboardButton::ascii_key('r');
    case AKEYCODE_S:
      return KeyboardButton::ascii_key('s');
    case AKEYCODE_T:
      return KeyboardButton::ascii_key('t');
    case AKEYCODE_U:
      return KeyboardButton::ascii_key('u');
    case AKEYCODE_V:
      return KeyboardButton::ascii_key('v');
    case AKEYCODE_W:
      return KeyboardButton::ascii_key('w');
    case AKEYCODE_X:
      return KeyboardButton::ascii_key('x');
    case AKEYCODE_Y:
      return KeyboardButton::ascii_key('y');
    case AKEYCODE_Z:
      return KeyboardButton::ascii_key('z');
    case AKEYCODE_COMMA:
      return KeyboardButton::ascii_key(',');
    case AKEYCODE_PERIOD:
      return KeyboardButton::ascii_key('.');
    case AKEYCODE_ALT_LEFT:
      return KeyboardButton::lalt();
    case AKEYCODE_ALT_RIGHT:
      return KeyboardButton::ralt();
    case AKEYCODE_SHIFT_LEFT:
      return KeyboardButton::lshift();
    case AKEYCODE_SHIFT_RIGHT:
      return KeyboardButton::rshift();
    case AKEYCODE_TAB:
      return KeyboardButton::tab();
    case AKEYCODE_SPACE:
      return KeyboardButton::space();
    case AKEYCODE_SYM:
    case AKEYCODE_EXPLORER:
    case AKEYCODE_ENVELOPE:
      break;
    case AKEYCODE_ENTER:
      return KeyboardButton::enter();
    case AKEYCODE_DEL:
      return KeyboardButton::backspace();
    case AKEYCODE_GRAVE:
      return KeyboardButton::ascii_key('`');
    case AKEYCODE_MINUS:
      return KeyboardButton::ascii_key('-');
    case AKEYCODE_EQUALS:
      return KeyboardButton::ascii_key('=');
    case AKEYCODE_LEFT_BRACKET:
      return KeyboardButton::ascii_key('[');
    case AKEYCODE_RIGHT_BRACKET:
      return KeyboardButton::ascii_key(']');
    case AKEYCODE_BACKSLASH:
      return KeyboardButton::ascii_key('\\');
    case AKEYCODE_SEMICOLON:
      return KeyboardButton::ascii_key(';');
    case AKEYCODE_APOSTROPHE:
      return KeyboardButton::ascii_key('\'');
    case AKEYCODE_SLASH:
      return KeyboardButton::ascii_key('/');
    case AKEYCODE_AT:
      return KeyboardButton::ascii_key('@');
    case AKEYCODE_NUM:
    case AKEYCODE_HEADSETHOOK:
    case AKEYCODE_FOCUS:
      break;
    case AKEYCODE_PLUS:
      return KeyboardButton::ascii_key('+');
    case AKEYCODE_MENU:
      return KeyboardButton::menu();
    case AKEYCODE_NOTIFICATION:
    case AKEYCODE_SEARCH:
    case AKEYCODE_MEDIA_PLAY_PAUSE:
    case AKEYCODE_MEDIA_STOP:
    case AKEYCODE_MEDIA_NEXT:
    case AKEYCODE_MEDIA_PREVIOUS:
    case AKEYCODE_MEDIA_REWIND:
    case AKEYCODE_MEDIA_FAST_FORWARD:
    case AKEYCODE_MUTE:
      break;
    case AKEYCODE_PAGE_UP:
      return KeyboardButton::page_up();
    case AKEYCODE_PAGE_DOWN:
      return KeyboardButton::page_down();
    case AKEYCODE_PICTSYMBOLS:
    case AKEYCODE_SWITCH_CHARSET:
    case AKEYCODE_BUTTON_A:
    case AKEYCODE_BUTTON_B:
    case AKEYCODE_BUTTON_C:
    case AKEYCODE_BUTTON_X:
    case AKEYCODE_BUTTON_Y:
    case AKEYCODE_BUTTON_Z:
    case AKEYCODE_BUTTON_L1:
    case AKEYCODE_BUTTON_R1:
    case AKEYCODE_BUTTON_L2:
    case AKEYCODE_BUTTON_R2:
    case AKEYCODE_BUTTON_THUMBL:
    case AKEYCODE_BUTTON_THUMBR:
    case AKEYCODE_BUTTON_START:
    case AKEYCODE_BUTTON_SELECT:
    case AKEYCODE_BUTTON_MODE:
      break;
    case AKEYCODE_ESCAPE:
      return KeyboardButton::escape();
    case AKEYCODE_FORWARD_DEL:
      return KeyboardButton::del();
    case AKEYCODE_CTRL_LEFT:
      return KeyboardButton::lcontrol();
    case AKEYCODE_CTRL_RIGHT:
      return KeyboardButton::rcontrol();
    case AKEYCODE_CAPS_LOCK:
      return KeyboardButton::caps_lock();
    case AKEYCODE_SCROLL_LOCK:
      return KeyboardButton::scroll_lock();
    case AKEYCODE_META_LEFT:
      return KeyboardButton::lmeta();
    case AKEYCODE_META_RIGHT:
      return KeyboardButton::rmeta();
    case AKEYCODE_FUNCTION:
      break;
    case AKEYCODE_SYSRQ:
      return KeyboardButton::print_screen();
    case AKEYCODE_BREAK:
      return KeyboardButton::pause();
    case AKEYCODE_MOVE_HOME:
      return KeyboardButton::home();
    case AKEYCODE_MOVE_END:
      return KeyboardButton::end();
    case AKEYCODE_INSERT:
      return KeyboardButton::insert();
    case AKEYCODE_F1:
      return KeyboardButton::f1();
    case AKEYCODE_F2:
      return KeyboardButton::f2();
    case AKEYCODE_F3:
      return KeyboardButton::f3();
    case AKEYCODE_F4:
      return KeyboardButton::f4();
    case AKEYCODE_F5:
      return KeyboardButton::f5();
    case AKEYCODE_F6:
      return KeyboardButton::f6();
    case AKEYCODE_F7:
      return KeyboardButton::f7();
    case AKEYCODE_F8:
      return KeyboardButton::f8();
    case AKEYCODE_F9:
      return KeyboardButton::f9();
    case AKEYCODE_F10:
      return KeyboardButton::f10();
    case AKEYCODE_F11:
      return KeyboardButton::f11();
    case AKEYCODE_F12:
      return KeyboardButton::f12();
    case AKEYCODE_NUM_LOCK:
      return KeyboardButton::num_lock();
    default:
      break;
  }
  return ButtonHandle::none();
}
