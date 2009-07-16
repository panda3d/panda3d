// Filename: subprocessWindow.cxx
// Created by:  drose (11Jul09)
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

#include "subprocessWindow.h"

#ifdef SUPPORT_SUBPROCESS_WINDOW

#include "graphicsEngine.h"
#include "config_display.h"

TypeHandle SubprocessWindow::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindow::Constructor
//       Access: Protected
//  Description: Normally, the SubprocessWindow constructor is not
//               called directly; these are created instead via the
//               GraphicsEngine::make_window() function.
////////////////////////////////////////////////////////////////////
SubprocessWindow::
SubprocessWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                 const string &name,
                 const FrameBufferProperties &fb_prop,
                 const WindowProperties &win_prop,
                 int flags,
                 GraphicsStateGuardian *gsg,
                 GraphicsOutput *host,
                 const string &filename) :
  GraphicsWindow(engine, pipe, name, fb_prop, win_prop, flags, gsg, host)
{
  GraphicsWindowInputDevice device =
    GraphicsWindowInputDevice::pointer_and_keyboard(this, "keyboard/mouse");
  _input_devices.push_back(device);

  // Create a buffer with the same properties as the window.
  flags = ((flags & ~GraphicsPipe::BF_require_window) | GraphicsPipe::BF_refuse_window);

  GraphicsOutput *buffer = 
    engine->make_output(pipe, name, 0, 
                        fb_prop, win_prop, flags, gsg, host);
  if (buffer != NULL) {
    _buffer = DCAST(GraphicsBuffer, buffer);
    // However, the buffer is not itself intended to be rendered.  We
    // only render it indirectly, via callbacks in here.
    _buffer->set_active(false);
  }

  // Now create a texture to receive the contents of the framebuffer
  // from the buffer.
  _texture = new Texture(name);

  _fd = -1;
  _mmap_size = 0;
  _filename = filename;
  _swbuffer = NULL;
  _last_event_flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindow::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
SubprocessWindow::
~SubprocessWindow() {
  if (_buffer != NULL) {
    _engine->remove_window(_buffer);
  }
  nassertv(_swbuffer == NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindow::process_events
//       Access: Public, Virtual
//  Description: Do whatever processing is necessary to ensure that
//               the window responds to user events.  Also, honor any
//               requests recently made via request_properties().
//
//               This function is called only within the window
//               thread.
////////////////////////////////////////////////////////////////////
void SubprocessWindow::
process_events() {
  GraphicsWindow::process_events();

  if (_swbuffer != NULL) {
    SubprocessWindowBuffer::Event swb_event;
    while (_swbuffer->get_event(swb_event)) {
      // Deal with this event.
      if (swb_event._flags & SubprocessWindowBuffer::EF_mouse_position) {
        _input_devices[0].set_pointer_in_window(swb_event._x, swb_event._y);
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

      ButtonHandle button;
      if (swb_event._source == SubprocessWindowBuffer::ES_mouse) {
        button = MouseButton::button(swb_event._code);

      } else {
        int keycode;
        button = translate_key(keycode, swb_event._code, swb_event._flags);
        if (keycode != 0 && swb_event._trans != SubprocessWindowBuffer::BT_up) {
          _input_devices[0].keystroke(keycode);
        }
      }

      if (swb_event._trans == SubprocessWindowBuffer::BT_up) {
        _input_devices[0].button_up(button);
      } else if (swb_event._trans == SubprocessWindowBuffer::BT_down) {
        _input_devices[0].button_down(button);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindow::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool SubprocessWindow::
begin_frame(FrameMode mode, Thread *current_thread) {
  if (_swbuffer == NULL || _buffer == NULL) {
    return false;
  }

  bool result = _buffer->begin_frame(mode, current_thread);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindow::end_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after rendering is completed for a given frame.  It
//               should do whatever finalization is required.
////////////////////////////////////////////////////////////////////
void SubprocessWindow::
end_frame(FrameMode mode, Thread *current_thread) {
  _buffer->end_frame(mode, current_thread);

  if (mode == FM_render) {
    _flip_ready = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindow::begin_flip
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
void SubprocessWindow::
begin_flip() {
  nassertv(_buffer != (GraphicsBuffer *)NULL);
  nassertv(_swbuffer != NULL);

  RenderBuffer buffer(_gsg, DrawableRegion::get_renderbuffer_type(RTP_color));
  buffer = _gsg->get_render_buffer(_buffer->get_draw_buffer_type(),
                                   _buffer->get_fb_properties());

  bool copied = 
    _gsg->framebuffer_copy_to_ram(_texture, -1,
                                  _default_display_region, buffer);

  if (copied) {
    CPTA_uchar image = _texture->get_ram_image();
    size_t framebuffer_size = _swbuffer->get_framebuffer_size();
    nassertv(image.size() == framebuffer_size);

    if (!_swbuffer->ready_for_write()) {
      // We have to wait for the other end to remove the last frame we
      // rendered.  We only wait so long before we give up, so we
      // don't completely starve the Python process just because the
      // render window is offscreen or something.
      
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

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindow::close_window
//       Access: Protected, Virtual
//  Description: Closes the window right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void SubprocessWindow::
close_window() {
  if (_swbuffer != NULL) {
    SubprocessWindowBuffer::close_buffer(_fd, _mmap_size, _filename, _swbuffer);
    _fd = -1;
    _filename = string();

    _swbuffer = NULL;
  }

  if (_buffer != NULL) {
    _buffer->request_close();
    _buffer->process_events();
  }

  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindow::open_window
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool SubprocessWindow::
open_window() {
  nout << "open_window\n";
  
  if (_buffer != NULL) {
    _buffer->request_open();
    _buffer->process_events();

    _is_valid = _buffer->is_valid();
  }

  if (!_is_valid) {
    return false;
  }

  _gsg = _buffer->get_gsg();

  _swbuffer = SubprocessWindowBuffer::open_buffer(_fd, _mmap_size, _filename);

  if (_swbuffer == NULL) {
    close(_fd);
    _fd = -1;
    _filename = string();
    _is_valid = false;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindow::translate_key
//       Access: Private
//  Description: Converts the os-specific keycode into the appropriate
//               ButtonHandle object.  Also stores the corresponding
//               Unicode keycode in keycode, if any; or 0 otherwise.
////////////////////////////////////////////////////////////////////
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

    // case  36: nk = KeyboardButton::ret(); break; // no return in panda ???
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
    // If we assigned an ASCII button, then get the original ASCII
    // code from the event (it will include shift et al).

    // TODO: is it possible to get any international characters via
    // this old EventRecord interface?
    keycode = os_code & 0xff;
  }
  
#endif __APPLE__

  return nk;
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindow::transition_button
//       Access: Private
//  Description: Sends the appropriate up/down transition for the
//               indicated modifier key, as determined implicitly from
//               the flags.
////////////////////////////////////////////////////////////////////
void SubprocessWindow::
transition_button(unsigned int flags, ButtonHandle button) {
  if (flags) {
    _input_devices[0].button_down(button);
  } else {  
    _input_devices[0].button_up(button);
  }
}


#endif  // SUPPORT_SUBPROCESS_WINDOW
