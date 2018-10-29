/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file subprocessWindow.cxx
 * @author drose
 * @date 2009-07-11
 */

#include "subprocessWindow.h"

#ifdef SUPPORT_SUBPROCESS_WINDOW

#include "graphicsEngine.h"
#include "config_display.h"
#include "nativeWindowHandle.h"
#include "mouseButton.h"
#include "throw_event.h"

using std::string;

TypeHandle SubprocessWindow::_type_handle;

/**
 * Normally, the SubprocessWindow constructor is not called directly; these
 * are created instead via the GraphicsEngine::make_window() function.
 */
SubprocessWindow::
SubprocessWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                 const string &name,
                 const FrameBufferProperties &fb_prop,
                 const WindowProperties &win_prop,
                 int flags,
                 GraphicsStateGuardian *gsg,
                 GraphicsOutput *host) :
  GraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  _input = GraphicsWindowInputDevice::pointer_and_keyboard(this, "keyboard/mouse");
  _input_devices.push_back(_input.p());

  // This will be an offscreen buffer that we use to render the actual
  // contents.
  _buffer = nullptr;

  // Create a texture to receive the contents of the framebuffer from the
  // offscreen buffer.
  _texture = new Texture(name);

  _fd = -1;
  _mmap_size = 0;
  _filename = string();
  _swbuffer = nullptr;
  _last_event_flags = 0;
}

/**
 *
 */
SubprocessWindow::
~SubprocessWindow() {
  nassertv(_buffer == nullptr);
  nassertv(_swbuffer == nullptr);
}

/**
 * Do whatever processing is necessary to ensure that the window responds to
 * user events.  Also, honor any requests recently made via
 * request_properties().
 *
 * This function is called only within the window thread.
 */
void SubprocessWindow::
process_events() {
  GraphicsWindow::process_events();

  if (_swbuffer != nullptr) {
    SubprocessWindowBuffer::Event swb_event;
    while (_swbuffer->get_event(swb_event)) {
      // Deal with this event.
      if (swb_event._flags & SubprocessWindowBuffer::EF_mouse_position) {
        _input->set_pointer_in_window(swb_event._x, swb_event._y);
      } else if ((swb_event._flags & SubprocessWindowBuffer::EF_has_mouse) == 0) {
        _input->set_pointer_out_of_window();
      }

      unsigned int diff = swb_event._flags ^ _last_event_flags;
      _last_event_flags = swb_event._flags;

      if (diff & SubprocessWindowBuffer::EF_shift_held) {
        transition_button(swb_event._flags & SubprocessWindowBuffer::EF_shift_held, KeyboardButton::shift());
      }
      if (diff & SubprocessWindowBuffer::EF_control_held) {
        transition_button(swb_event._flags & SubprocessWindowBuffer::EF_control_held, KeyboardButton::control());
      }
      if (diff & SubprocessWindowBuffer::EF_alt_held) {
        transition_button(swb_event._flags & SubprocessWindowBuffer::EF_alt_held, KeyboardButton::alt());
      }
      if (diff & SubprocessWindowBuffer::EF_meta_held) {
        transition_button(swb_event._flags & SubprocessWindowBuffer::EF_meta_held, KeyboardButton::meta());
      }

      ButtonHandle button = ButtonHandle::none();
      if (swb_event._source == SubprocessWindowBuffer::ES_mouse) {
        button = MouseButton::button(swb_event._code);

      } else if (swb_event._source == SubprocessWindowBuffer::ES_keyboard) {
        int keycode;
        button = translate_key(keycode, swb_event._code, swb_event._flags);
        if (keycode != 0 && swb_event._type != SubprocessWindowBuffer::ET_button_up) {
          _input->keystroke(keycode);
        }
      }

      if (swb_event._type == SubprocessWindowBuffer::ET_button_up) {
        _input->button_up(button);
      } else if (swb_event._type == SubprocessWindowBuffer::ET_button_down) {
        _input->button_down(button);
      }
    }
  }
}

/**
 * This function will be called within the draw thread before beginning
 * rendering for a given frame.  It should do whatever setup is required, and
 * return true if the frame should be rendered, or false if it should be
 * skipped.
 */
bool SubprocessWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  if (_swbuffer == nullptr || _buffer == nullptr) {
    return false;
  }

  bool result = _buffer->begin_frame(mode, current_thread);
  return result;
}

/**
 * This function will be called within the draw thread after rendering is
 * completed for a given frame.  It should do whatever finalization is
 * required.
 */
void SubprocessWindow::
end_frame(FrameMode mode, Thread *current_thread) {
  _buffer->end_frame(mode, current_thread);

  if (mode == FM_render) {
    _flip_ready = true;
  }
}

/**
 * This function will be called within the draw thread after end_frame() has
 * been called on all windows, to initiate the exchange of the front and back
 * buffers.
 *
 * This should instruct the window to prepare for the flip at the next video
 * sync, but it should not wait.
 *
 * We have the two separate functions, begin_flip() and end_flip(), to make it
 * easier to flip all of the windows at the same time.
 */
void SubprocessWindow::
begin_flip() {
  nassertv(_buffer != nullptr);
  if (_swbuffer == nullptr) {
    return;
  }

  RenderBuffer buffer(_gsg, DrawableRegion::get_renderbuffer_type(RTP_color));
  buffer = _gsg->get_render_buffer(_buffer->get_draw_buffer_type(),
                                   _buffer->get_fb_properties());

  bool copied =
    _gsg->framebuffer_copy_to_ram(_texture, 0, -1,
                                  _overlay_display_region, buffer);

  if (copied) {
    CPTA_uchar image = _texture->get_ram_image();
    size_t framebuffer_size = _swbuffer->get_framebuffer_size();
    nassertv(image.size() == framebuffer_size);

    if (!_swbuffer->ready_for_write()) {
      // We have to wait for the other end to remove the last frame we
      // rendered.  We only wait so long before we give up, so we don't
      // completely starve the Python process just because the render window
      // is offscreen or something.

      ClockObject *clock = ClockObject::get_global_clock();
      double start = clock->get_real_time();
      while (!_swbuffer->ready_for_write()) {
        Thread::force_yield();
        double now = clock->get_real_time();
        if (now - start > subprocess_window_max_wait) {
          // Never mind.
          return;
        }
      }
    }

    // We're ready to go.  Copy the image to our shared framebuffer.
    void *target = _swbuffer->open_write_framebuffer();
    memcpy(target, image.p(), framebuffer_size);
    _swbuffer->close_write_framebuffer();
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
 * The properties that have been applied are cleared from the structure by
 * this function; so on return, whatever remains in the properties structure
 * are those that were unchanged for some reason (probably because the
 * underlying interface does not support changing that property on an open
 * window).
 */
void SubprocessWindow::
set_properties_now(WindowProperties &properties) {
  Filename filename;
  WindowHandle *window_handle = properties.get_parent_window();
  if (window_handle != nullptr) {
    WindowHandle::OSHandle *os_handle = window_handle->get_os_handle();
    if (os_handle != nullptr) {
      if (os_handle->is_of_type(NativeWindowHandle::SubprocessHandle::get_class_type())) {
        NativeWindowHandle::SubprocessHandle *subprocess_handle = DCAST(NativeWindowHandle::SubprocessHandle, os_handle);
        filename = subprocess_handle->get_filename();
      }
    }
  }

  if (!filename.empty() && filename != _filename) {
    // We're changing the subprocess buffer filename; that means we might as
    // well completely close and re-open the window.
    display_cat.info() << "Re-opening SubprocessWindow\n";
    internal_close_window();

    _properties.add_properties(properties);
    properties.clear();

    internal_open_window();
    set_size_and_recalc(_properties.get_x_size(), _properties.get_y_size());
    throw_event(get_window_event(), this);
    return;
  }

  GraphicsWindow::set_properties_now(properties);
  if (!properties.is_any_specified()) {
    // The base class has already handled this case.
    return;
  }

  if (properties.has_parent_window()) {
    // Redundant parent-window specification.
    properties.clear_parent_window();
  }
}

/**
 * Closes the window right now.  Called from the window thread.
 */
void SubprocessWindow::
close_window() {
  internal_close_window();

  WindowProperties properties;
  properties.set_open(false);
  properties.set_foreground(false);
  system_changed_properties(properties);
}

/**
 * Opens the window right now.  Called from the window thread.  Returns true
 * if the window is successfully opened, or false if there was a problem.
 */
bool SubprocessWindow::
open_window() {
  if (!internal_open_window()) {
    return false;
  }

  WindowProperties properties;
  properties.set_open(true);
  properties.set_foreground(true);
  system_changed_properties(properties);

  return true;
}

/**
 * Closes the "window" and resets the buffer, without changing the
 * WindowProperties.
 */
void SubprocessWindow::
internal_close_window() {
  if (_swbuffer != nullptr) {
    SubprocessWindowBuffer::close_buffer
      (_fd, _mmap_size, _filename.to_os_specific(), _swbuffer);
    _fd = -1;
    _filename = string();

    _swbuffer = nullptr;
  }

  if (_buffer != nullptr) {
    _buffer->request_close();
    _buffer->process_events();
    _engine->remove_window(_buffer);
    _buffer = nullptr;
  }

  // Tell our parent window (if any) that we're no longer its child.
  if (_window_handle != nullptr &&
      _parent_window_handle != nullptr) {
    _parent_window_handle->detach_child(_window_handle);
  }

  _window_handle = nullptr;
  _parent_window_handle = nullptr;
  _is_valid = false;
}

/**
 * Opens the "window" and the associated offscreen buffer, without changing
 * the WindowProperties.
 */
bool SubprocessWindow::
internal_open_window() {
  nassertr(_buffer == nullptr, false);

  // Create a buffer with the same properties as the window.
  int flags = _creation_flags;
  flags = ((flags & ~GraphicsPipe::BF_require_window) | GraphicsPipe::BF_refuse_window);
  WindowProperties win_props = WindowProperties::size(_properties.get_x_size(), _properties.get_y_size());

  GraphicsOutput *buffer =
    _engine->make_output(_pipe, _name, 0, _fb_properties, win_props,
                         flags, _gsg, _host);
  if (buffer != nullptr) {
    _buffer = DCAST(GraphicsBuffer, buffer);
    // However, the buffer is not itself intended to be rendered.  We only
    // render it indirectly, via callbacks in here.
    _buffer->set_active(false);

    _buffer->request_open();
    _buffer->process_events();

    _is_valid = _buffer->is_valid();
  }

  if (!_is_valid) {
    display_cat.error()
      << "Failed to open SubprocessWindowBuffer's internal offscreen buffer.\n";
    return false;
  }

  _gsg = _buffer->get_gsg();

  WindowHandle *window_handle = _properties.get_parent_window();
  if (window_handle != nullptr) {
    WindowHandle::OSHandle *os_handle = window_handle->get_os_handle();
    if (os_handle != nullptr) {
      if (os_handle->is_of_type(NativeWindowHandle::SubprocessHandle::get_class_type())) {
        NativeWindowHandle::SubprocessHandle *subprocess_handle = DCAST(NativeWindowHandle::SubprocessHandle, os_handle);
        _filename = subprocess_handle->get_filename();
      }
    }
  }
  _parent_window_handle = window_handle;

  if (_filename.empty()) {
    _is_valid = false;
    display_cat.error()
      << "No filename given to SubprocessWindow.\n";
    return false;
  }

  _swbuffer = SubprocessWindowBuffer::open_buffer
    (_fd, _mmap_size, _filename.to_os_specific());

  if (_swbuffer == nullptr) {
    close(_fd);
    _fd = -1;
    _filename = string();
    _is_valid = false;
    display_cat.error()
      << "Failed to open SubprocessWindowBuffer's shared-memory buffer "
      << _filename << "\n";
    return false;
  }

  if (display_cat.is_debug()) {
    display_cat.debug()
      << "SubprocessWindow reading " << _filename << "\n";
  }

  // Create a WindowHandle for ourselves
  _window_handle = NativeWindowHandle::make_subprocess(_filename);

  // And tell our parent window that we're now its child.
  if (_parent_window_handle != nullptr) {
    _parent_window_handle->attach_child(_window_handle);
  }

  return true;
}

/**
 * Converts the os-specific keycode into the appropriate ButtonHandle object.
 * Also stores the corresponding Unicode keycode in keycode, if any; or 0
 * otherwise.
 */
ButtonHandle SubprocessWindow::
translate_key(int &keycode, int os_code, unsigned int flags) const {
  keycode = 0;
  ButtonHandle nk = ButtonHandle::none();

#ifdef __APPLE__
  switch ((os_code >> 8) & 0xff) {
  case   0: nk = KeyboardButton::ascii_key('a'); break;
  case  11: nk = KeyboardButton::ascii_key('b'); break;
  case   8: nk = KeyboardButton::ascii_key('c'); break;
  case   2: nk = KeyboardButton::ascii_key('d'); break;
  case  14: nk = KeyboardButton::ascii_key('e'); break;
  case   3: nk = KeyboardButton::ascii_key('f'); break;
  case   5: nk = KeyboardButton::ascii_key('g'); break;
  case   4: nk = KeyboardButton::ascii_key('h'); break;
  case  34: nk = KeyboardButton::ascii_key('i'); break;
  case  38: nk = KeyboardButton::ascii_key('j'); break;
  case  40: nk = KeyboardButton::ascii_key('k'); break;
  case  37: nk = KeyboardButton::ascii_key('l'); break;
  case  46: nk = KeyboardButton::ascii_key('m'); break;
  case  45: nk = KeyboardButton::ascii_key('n'); break;
  case  31: nk = KeyboardButton::ascii_key('o'); break;
  case  35: nk = KeyboardButton::ascii_key('p'); break;
  case  12: nk = KeyboardButton::ascii_key('q'); break;
  case  15: nk = KeyboardButton::ascii_key('r'); break;
  case   1: nk = KeyboardButton::ascii_key('s'); break;
  case  17: nk = KeyboardButton::ascii_key('t'); break;
  case  32: nk = KeyboardButton::ascii_key('u'); break;
  case   9: nk = KeyboardButton::ascii_key('v'); break;
  case  13: nk = KeyboardButton::ascii_key('w'); break;
  case   7: nk = KeyboardButton::ascii_key('x'); break;
  case  16: nk = KeyboardButton::ascii_key('y'); break;
  case   6: nk = KeyboardButton::ascii_key('z'); break;

    // top row numbers
  case  29: nk = KeyboardButton::ascii_key('0'); break;
  case  18: nk = KeyboardButton::ascii_key('1'); break;
  case  19: nk = KeyboardButton::ascii_key('2'); break;
  case  20: nk = KeyboardButton::ascii_key('3'); break;
  case  21: nk = KeyboardButton::ascii_key('4'); break;
  case  23: nk = KeyboardButton::ascii_key('5'); break;
  case  22: nk = KeyboardButton::ascii_key('6'); break;
  case  26: nk = KeyboardButton::ascii_key('7'); break;
  case  28: nk = KeyboardButton::ascii_key('8'); break;
  case  25: nk = KeyboardButton::ascii_key('9'); break;

    // key pad ... do they really map to the top number in panda ?
  case  82: nk = KeyboardButton::ascii_key('0'); break;
  case  83: nk = KeyboardButton::ascii_key('1'); break;
  case  84: nk = KeyboardButton::ascii_key('2'); break;
  case  85: nk = KeyboardButton::ascii_key('3'); break;
  case  86: nk = KeyboardButton::ascii_key('4'); break;
  case  87: nk = KeyboardButton::ascii_key('5'); break;
  case  88: nk = KeyboardButton::ascii_key('6'); break;
  case  89: nk = KeyboardButton::ascii_key('7'); break;
  case  91: nk = KeyboardButton::ascii_key('8'); break;
  case  92: nk = KeyboardButton::ascii_key('9'); break;

    // case  36: nk = KeyboardButton::ret(); break;  no return in panda ???
  case  49: nk = KeyboardButton::space(); break;
  case  51: nk = KeyboardButton::backspace(); break;
  case  48: nk = KeyboardButton::tab(); break;
  case  53: nk = KeyboardButton::escape(); break;
  case  76: nk = KeyboardButton::enter(); break;
  case  36: nk = KeyboardButton::enter(); break;

  case 123: nk = KeyboardButton::left(); break;
  case 124: nk = KeyboardButton::right(); break;
  case 125: nk = KeyboardButton::down(); break;
  case 126: nk = KeyboardButton::up(); break;
  case 116: nk = KeyboardButton::page_up(); break;
  case 121: nk = KeyboardButton::page_down(); break;
  case 115: nk = KeyboardButton::home(); break;
  case 119: nk = KeyboardButton::end(); break;
  case 114: nk = KeyboardButton::help(); break;
  case 117: nk = KeyboardButton::del(); break;

    // case  71: nk = KeyboardButton::num_lock() break;

  case 122: nk = KeyboardButton::f1(); break;
  case 120: nk = KeyboardButton::f2(); break;
  case  99: nk = KeyboardButton::f3(); break;
  case 118: nk = KeyboardButton::f4(); break;
  case  96: nk = KeyboardButton::f5(); break;
  case  97: nk = KeyboardButton::f6(); break;
  case  98: nk = KeyboardButton::f7(); break;
  case 100: nk = KeyboardButton::f8(); break;
  case 101: nk = KeyboardButton::f9(); break;
  case 109: nk = KeyboardButton::f10(); break;
  case 103: nk = KeyboardButton::f11(); break;
  case 111: nk = KeyboardButton::f12(); break;

  case 105: nk = KeyboardButton::f13(); break;
  case 107: nk = KeyboardButton::f14(); break;
  case 113: nk = KeyboardButton::f15(); break;
  case 106: nk = KeyboardButton::f16(); break;

    // shiftable chartablet
  case  50: nk = KeyboardButton::ascii_key('`'); break;
  case  27: nk = KeyboardButton::ascii_key('-'); break;
  case  24: nk = KeyboardButton::ascii_key('='); break;
  case  33: nk = KeyboardButton::ascii_key('['); break;
  case  30: nk = KeyboardButton::ascii_key(']'); break;
  case  42: nk = KeyboardButton::ascii_key('\\'); break;
  case  41: nk = KeyboardButton::ascii_key(';'); break;
  case  39: nk = KeyboardButton::ascii_key('\''); break;
  case  43: nk = KeyboardButton::ascii_key(','); break;
  case  47: nk = KeyboardButton::ascii_key('.'); break;
  case  44: nk = KeyboardButton::ascii_key('/'); break;

  default:
    // Punt.
    nk = KeyboardButton::ascii_key(os_code & 0xff);
  }

  if (nk.has_ascii_equivalent()) {
    // If we assigned an ASCII button, then get the original ASCII code from
    // the event (it will include shift et al).

    // TODO: is it possible to get any international characters via this old
    // EventRecord interface?
    keycode = os_code & 0xff;
  }

#endif  // __APPLE__

  return nk;
}

/**
 * Sends the appropriate up/down transition for the indicated modifier key, as
 * determined implicitly from the flags.
 */
void SubprocessWindow::
transition_button(unsigned int flags, ButtonHandle button) {
  if (flags) {
    _input->button_down(button);
  } else {
    _input->button_up(button);
  }
}


#endif  // SUPPORT_SUBPROCESS_WINDOW
