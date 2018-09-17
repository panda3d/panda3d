/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tinySDLGraphicsWindow.cxx
 * @author drose
 * @date 2008-04-24
 */

#include "pandabase.h"

#ifdef HAVE_SDL

#include "tinySDLGraphicsWindow.h"
#include "tinyGraphicsStateGuardian.h"
#include "config_tinydisplay.h"
#include "tinySDLGraphicsPipe.h"
#include "mouseButton.h"
#include "keyboardButton.h"
#include "graphicsPipe.h"

TypeHandle TinySDLGraphicsWindow::_type_handle;

/**
 *
 */
TinySDLGraphicsWindow::
TinySDLGraphicsWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                      const std::string &name,
                      const FrameBufferProperties &fb_prop,
                      const WindowProperties &win_prop,
                      int flags,
                      GraphicsStateGuardian *gsg,
                      GraphicsOutput *host) :
  GraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  _screen = nullptr;
  _frame_buffer = nullptr;
  _pitch = 0;
  update_pixel_factor();

  add_input_device(GraphicsWindowInputDevice::pointer_and_keyboard(this, "keyboard_mouse"));
}

/**
 *
 */
TinySDLGraphicsWindow::
~TinySDLGraphicsWindow() {
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool TinySDLGraphicsWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  begin_frame_spam(mode);
  if (_gsg == nullptr) {
    return false;
  }

  TinyGraphicsStateGuardian *tinygsg;
  DCAST_INTO_R(tinygsg, _gsg, false);

  tinygsg->_current_frame_buffer = _frame_buffer;
  tinygsg->reset_if_new();

  _gsg->set_current_properties(&get_fb_properties());
  return _gsg->begin_frame(current_thread);
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void TinySDLGraphicsWindow::
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
  }
}

/**
 * This function will be called within the draw thread after begin_flip() has
 * been called on all windows, to finish the exchange of the front and back
 * buffers.
 *
 * This should cause the window to wait for the flip, if necessary.
 */
void TinySDLGraphicsWindow::
end_flip() {
  if (!_flip_ready) {
    GraphicsWindow::end_flip();
    return;
  }

  int fb_xsize = get_fb_x_size();
  int fb_ysize = get_fb_y_size();

  if (fb_xsize == _frame_buffer->xsize) {
    // No zooming is necessary--copy directly to the screen.
    if (SDL_MUSTLOCK(_screen)) {
      if (SDL_LockSurface(_screen) < 0) {
        tinydisplay_cat.error()
          << "Can't lock screen: " << SDL_GetError() << "\n";
      }
    }
    ZB_copyFrameBuffer(_frame_buffer, _screen->pixels, _pitch);

    if (SDL_MUSTLOCK(_screen)) {
      SDL_UnlockSurface(_screen);
    }

  } else {
    // Copy to another surface, then scale it onto the screen.
    SDL_Surface *temp =
      SDL_CreateRGBSurfaceFrom(_frame_buffer->pbuf, _frame_buffer->xsize, _frame_buffer->ysize,
                               32, _frame_buffer->linesize, 0xff0000, 0x00ff00, 0x0000ff, 0xff000000);
    SDL_SetAlpha(temp, SDL_RLEACCEL, 0);
    SDL_BlitSurface(temp, nullptr, _screen, nullptr);
    SDL_FreeSurface(temp);
  }

  SDL_Flip(_screen);
  GraphicsWindow::end_flip();
}

/**
 * Do whatever processing is necessary to ensure that the window responds to
 * user events.  Also, honor any requests recently made via
 * request_properties()
 *
 * This function is called only within the window thread.
 */
void TinySDLGraphicsWindow::
process_events() {
  GraphicsWindow::process_events();

  if (_screen == nullptr) {
    return;
  }

  WindowProperties properties;

  SDL_Event evt;
  ButtonHandle button;
  while (SDL_PollEvent(&evt)) {
    switch(evt.type) {
    case SDL_KEYDOWN:
      if (evt.key.keysym.unicode) {
        _input_devices[0]->keystroke(evt.key.keysym.unicode);
      }
      button = get_keyboard_button(evt.key.keysym.sym);
      if (button != ButtonHandle::none()) {
        _input_devices[0]->button_down(button);
      }
      break;

    case SDL_KEYUP:
      button = get_keyboard_button(evt.key.keysym.sym);
      if (button != ButtonHandle::none()) {
        _input_devices[0]->button_up(button);
      }
      break;

    case SDL_MOUSEBUTTONDOWN:
      button = get_mouse_button(evt.button.button);
      _input_devices[0]->set_pointer_in_window(evt.button.x, evt.button.y);
      _input_devices[0]->button_down(button);
      break;

    case SDL_MOUSEBUTTONUP:
      button = get_mouse_button(evt.button.button);
      _input_devices[0]->set_pointer_in_window(evt.button.x, evt.button.y);
      _input_devices[0]->button_up(button);
      break;

    case SDL_MOUSEMOTION:
      _input_devices[0]->set_pointer_in_window(evt.motion.x, evt.motion.y);
      break;

    case SDL_VIDEORESIZE:
      properties.set_size(evt.resize.w, evt.resize.h);
      system_changed_properties(properties);
      _screen = SDL_SetVideoMode(_properties.get_x_size(), _properties.get_y_size(), 32, _flags);
      ZB_resize(_frame_buffer, nullptr, _properties.get_x_size(), _properties.get_y_size());
      _pitch = _screen->pitch * 32 / _screen->format->BitsPerPixel;
      break;

    case SDL_QUIT:
      // The window was closed by the user.
      close_window();
      properties.set_open(false);
      system_changed_properties(properties);
      break;
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
void TinySDLGraphicsWindow::
set_properties_now(WindowProperties &properties) {
  GraphicsWindow::set_properties_now(properties);
  if (!properties.is_any_specified()) {
    // The base class has already handled this case.
    return;
  }
}

/**
 * Returns true if a call to set_pixel_zoom() will be respected, false if it
 * will be ignored.  If this returns false, then get_pixel_factor() will
 * always return 1.0, regardless of what value you specify for
 * set_pixel_zoom().
 *
 * This may return false if the underlying renderer doesn't support pixel
 * zooming, or if you have called this on a DisplayRegion that doesn't have
 * both set_clear_color() and set_clear_depth() enabled.
 */
bool TinySDLGraphicsWindow::
supports_pixel_zoom() const {
  return true;
}

/**
 * Closes the window right now.  Called from the window thread.
 */
void TinySDLGraphicsWindow::
close_window() {
  GraphicsWindow::close_window();
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool TinySDLGraphicsWindow::
open_window() {

  // GSG CreationInitialization
  TinyGraphicsStateGuardian *tinygsg;
  if (_gsg == 0) {
    // There is no old gsg.  Create a new one.
    tinygsg = new TinyGraphicsStateGuardian(_engine, _pipe, nullptr);
    _gsg = tinygsg;

  } else {
    DCAST_INTO_R(tinygsg, _gsg, false);
  }

  _flags = SDL_SWSURFACE;
  if (_properties.get_fullscreen()) {
    _flags |= SDL_FULLSCREEN;
  }
  if (!_properties.get_fixed_size()) {
    _flags |= SDL_RESIZABLE;
  }
  if (_properties.get_undecorated()) {
    _flags |= SDL_NOFRAME;
  }
  _screen = SDL_SetVideoMode(_properties.get_x_size(), _properties.get_y_size(), 32, _flags);

  if (_screen == nullptr) {
    tinydisplay_cat.error()
      << "Video mode set failed.\n";
    return false;
  }

  create_frame_buffer();
  if (_frame_buffer == nullptr) {
    tinydisplay_cat.error()
      << "Could not create frame buffer.\n";
    return false;
  }

  tinygsg->_current_frame_buffer = _frame_buffer;

  // Now that we have made the context current to a window, we can reset the
  // GSG state if this is the first time it has been used.  (We can't just
  // call reset() when we construct the GSG, because reset() requires having a
  // current context.)
  tinygsg->reset_if_new();

  return true;
}

/**
 * Creates a suitable frame buffer for the current window size.
 */
void TinySDLGraphicsWindow::
create_frame_buffer() {
  if (_frame_buffer != nullptr) {
    ZB_close(_frame_buffer);
    _frame_buffer = nullptr;
  }

  int mode;
  switch (_screen->format->BitsPerPixel) {
  case  8:
    tinydisplay_cat.error()
      << "SDL Palettes are currently not supported.\n";
    return;

  case 16:
    mode = ZB_MODE_5R6G5B;
    break;
  case 24:
    mode = ZB_MODE_RGB24;
    break;
  case 32:
    mode = ZB_MODE_RGBA;
    break;

  default:
    return;
  }

  _frame_buffer = ZB_open(_properties.get_x_size(), _properties.get_y_size(), mode, 0, 0, 0, 0);

  _pitch = _screen->pitch * 32 / _screen->format->BitsPerPixel;
}

/**
 * Maps from an SDL keysym to the corresponding Panda ButtonHandle.
 */
ButtonHandle TinySDLGraphicsWindow::
get_keyboard_button(SDLKey sym) {
  switch (sym) {
  case SDLK_BACKSPACE: return KeyboardButton::backspace();
  case SDLK_TAB: return KeyboardButton::tab();
    // case SDLK_CLEAR: return KeyboardButton::clear();
  case SDLK_RETURN: return KeyboardButton::enter();
    // case SDLK_PAUSE: return KeyboardButton::pause();
  case SDLK_ESCAPE: return KeyboardButton::escape();
  case SDLK_SPACE: return KeyboardButton::space();
  case SDLK_EXCLAIM: return KeyboardButton::ascii_key('!');
  case SDLK_QUOTEDBL: return KeyboardButton::ascii_key('"');
  case SDLK_HASH: return KeyboardButton::ascii_key('#');
  case SDLK_DOLLAR: return KeyboardButton::ascii_key('$');
  case SDLK_AMPERSAND: return KeyboardButton::ascii_key('&');
  case SDLK_QUOTE: return KeyboardButton::ascii_key('\'');
  case SDLK_LEFTPAREN: return KeyboardButton::ascii_key('(');
  case SDLK_RIGHTPAREN: return KeyboardButton::ascii_key(')');
  case SDLK_ASTERISK: return KeyboardButton::ascii_key('*');
  case SDLK_PLUS: return KeyboardButton::ascii_key('+');
  case SDLK_COMMA: return KeyboardButton::ascii_key(',');
  case SDLK_MINUS: return KeyboardButton::ascii_key('-');
  case SDLK_PERIOD: return KeyboardButton::ascii_key('.');
  case SDLK_SLASH: return KeyboardButton::ascii_key('/');
  case SDLK_0: return KeyboardButton::ascii_key('0');
  case SDLK_1: return KeyboardButton::ascii_key('1');
  case SDLK_2: return KeyboardButton::ascii_key('2');
  case SDLK_3: return KeyboardButton::ascii_key('3');
  case SDLK_4: return KeyboardButton::ascii_key('4');
  case SDLK_5: return KeyboardButton::ascii_key('5');
  case SDLK_6: return KeyboardButton::ascii_key('6');
  case SDLK_7: return KeyboardButton::ascii_key('7');
  case SDLK_8: return KeyboardButton::ascii_key('8');
  case SDLK_9: return KeyboardButton::ascii_key('9');
  case SDLK_COLON: return KeyboardButton::ascii_key(':');
  case SDLK_SEMICOLON: return KeyboardButton::ascii_key(';');
  case SDLK_LESS: return KeyboardButton::ascii_key('<');
  case SDLK_EQUALS: return KeyboardButton::ascii_key('=');
  case SDLK_GREATER: return KeyboardButton::ascii_key('>');
  case SDLK_QUESTION: return KeyboardButton::ascii_key('?');
  case SDLK_AT: return KeyboardButton::ascii_key('@');
  case SDLK_LEFTBRACKET: return KeyboardButton::ascii_key('[');
  case SDLK_BACKSLASH: return KeyboardButton::ascii_key('\\');
  case SDLK_RIGHTBRACKET: return KeyboardButton::ascii_key(']');
  case SDLK_CARET: return KeyboardButton::ascii_key('^');
  case SDLK_UNDERSCORE: return KeyboardButton::ascii_key('_');
  case SDLK_BACKQUOTE: return KeyboardButton::ascii_key('`');
  case SDLK_a: return KeyboardButton::ascii_key('a');
  case SDLK_b: return KeyboardButton::ascii_key('b');
  case SDLK_c: return KeyboardButton::ascii_key('c');
  case SDLK_d: return KeyboardButton::ascii_key('d');
  case SDLK_e: return KeyboardButton::ascii_key('e');
  case SDLK_f: return KeyboardButton::ascii_key('f');
  case SDLK_g: return KeyboardButton::ascii_key('g');
  case SDLK_h: return KeyboardButton::ascii_key('h');
  case SDLK_i: return KeyboardButton::ascii_key('i');
  case SDLK_j: return KeyboardButton::ascii_key('j');
  case SDLK_k: return KeyboardButton::ascii_key('k');
  case SDLK_l: return KeyboardButton::ascii_key('l');
  case SDLK_m: return KeyboardButton::ascii_key('m');
  case SDLK_n: return KeyboardButton::ascii_key('n');
  case SDLK_o: return KeyboardButton::ascii_key('o');
  case SDLK_p: return KeyboardButton::ascii_key('p');
  case SDLK_q: return KeyboardButton::ascii_key('q');
  case SDLK_r: return KeyboardButton::ascii_key('r');
  case SDLK_s: return KeyboardButton::ascii_key('s');
  case SDLK_t: return KeyboardButton::ascii_key('t');
  case SDLK_u: return KeyboardButton::ascii_key('u');
  case SDLK_v: return KeyboardButton::ascii_key('v');
  case SDLK_w: return KeyboardButton::ascii_key('w');
  case SDLK_x: return KeyboardButton::ascii_key('x');
  case SDLK_y: return KeyboardButton::ascii_key('y');
  case SDLK_z: return KeyboardButton::ascii_key('z');
  case SDLK_DELETE: return KeyboardButton::del();
  case SDLK_KP0: return KeyboardButton::ascii_key('0');
  case SDLK_KP1: return KeyboardButton::ascii_key('1');
  case SDLK_KP2: return KeyboardButton::ascii_key('2');
  case SDLK_KP3: return KeyboardButton::ascii_key('3');
  case SDLK_KP4: return KeyboardButton::ascii_key('4');
  case SDLK_KP5: return KeyboardButton::ascii_key('5');
  case SDLK_KP6: return KeyboardButton::ascii_key('6');
  case SDLK_KP7: return KeyboardButton::ascii_key('7');
  case SDLK_KP8: return KeyboardButton::ascii_key('8');
  case SDLK_KP9: return KeyboardButton::ascii_key('9');
  case SDLK_KP_PERIOD: return KeyboardButton::ascii_key('.');
  case SDLK_KP_DIVIDE: return KeyboardButton::ascii_key('/');
  case SDLK_KP_MULTIPLY: return KeyboardButton::ascii_key('*');
  case SDLK_KP_MINUS: return KeyboardButton::ascii_key('-');
  case SDLK_KP_PLUS: return KeyboardButton::ascii_key('+');
  case SDLK_KP_ENTER: return KeyboardButton::enter();
  case SDLK_KP_EQUALS: return KeyboardButton::ascii_key('=');
  case SDLK_UP: return KeyboardButton::up();
  case SDLK_DOWN: return KeyboardButton::down();
  case SDLK_RIGHT: return KeyboardButton::right();
  case SDLK_LEFT: return KeyboardButton::left();
  case SDLK_INSERT: return KeyboardButton::insert();
  case SDLK_HOME: return KeyboardButton::home();
  case SDLK_END: return KeyboardButton::end();
  case SDLK_PAGEUP: return KeyboardButton::page_up();
  case SDLK_PAGEDOWN: return KeyboardButton::page_down();
  case SDLK_F1: return KeyboardButton::f1();
  case SDLK_F2: return KeyboardButton::f2();
  case SDLK_F3: return KeyboardButton::f3();
  case SDLK_F4: return KeyboardButton::f4();
  case SDLK_F5: return KeyboardButton::f5();
  case SDLK_F6: return KeyboardButton::f6();
  case SDLK_F7: return KeyboardButton::f7();
  case SDLK_F8: return KeyboardButton::f8();
  case SDLK_F9: return KeyboardButton::f9();
  case SDLK_F10: return KeyboardButton::f10();
  case SDLK_F11: return KeyboardButton::f11();
  case SDLK_F12: return KeyboardButton::f12();
  case SDLK_F13: return KeyboardButton::f13();
  case SDLK_F14: return KeyboardButton::f14();
  case SDLK_F15: return KeyboardButton::f15();
    // case SDLK_NUMLOCK: return KeyboardButton::numlock(); case
    // SDLK_CAPSLOCK: return KeyboardButton::capslock(); case SDLK_SCROLLOCK:
    // return KeyboardButton::scrollock();
  case SDLK_RSHIFT: return KeyboardButton::rshift();
  case SDLK_LSHIFT: return KeyboardButton::lshift();
  case SDLK_RCTRL: return KeyboardButton::rcontrol();
  case SDLK_LCTRL: return KeyboardButton::lcontrol();
  case SDLK_RALT: return KeyboardButton::ralt();
  case SDLK_LALT: return KeyboardButton::lalt();
  case SDLK_RMETA: return KeyboardButton::ralt();
  case SDLK_LMETA: return KeyboardButton::lalt();
    // case SDLK_LSUPER: return KeyboardButton::left(); case SDLK_RSUPER:
    // return KeyboardButton::right(); case SDLK_MODE: return
    // KeyboardButton::mode();
  case SDLK_HELP: return KeyboardButton::help();
    // case SDLK_PRINT: return KeyboardButton::print-screen(); case
    // SDLK_SYSREQ: return KeyboardButton::SysRq(); case SDLK_BREAK: return
    // KeyboardButton::break(); case SDLK_MENU: return KeyboardButton::menu();
    // case SDLK_POWER: return KeyboardButton::power(); case SDLK_EURO: return
    // KeyboardButton::euro();
  }
  tinydisplay_cat.info()
    << "unhandled keyboard button " << sym << "\n";

  return ButtonHandle::none();
}

/**
 * Maps from an SDL mouse button index to the corresponding Panda
 * ButtonHandle.
 */
ButtonHandle TinySDLGraphicsWindow::
get_mouse_button(Uint8 button) {
  switch (button) {
  case SDL_BUTTON_LEFT:
    return MouseButton::one();

  case SDL_BUTTON_MIDDLE:
    return MouseButton::two();

  case SDL_BUTTON_RIGHT:
    return MouseButton::three();

  case SDL_BUTTON_WHEELUP:
    return MouseButton::wheel_up();

  case SDL_BUTTON_WHEELDOWN:
    return MouseButton::wheel_down();
  }
  tinydisplay_cat.info()
    << "unhandled mouse button " << button << "\n";

  return ButtonHandle::none();
}

#endif  // HAVE_SDL
