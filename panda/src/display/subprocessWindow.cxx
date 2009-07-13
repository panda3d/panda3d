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
    SubprocessWindowBuffer::Event event;
    while (_swbuffer->get_event(event)) {
      // Deal with this event.  For now, we only have mouse down/up.
      _input_devices[0].set_pointer_in_window(event._x, event._y);
      if (event._up) {
        _input_devices[0].button_up(MouseButton::one());
      } else {
        _input_devices[0].button_down(MouseButton::one());
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

  if (!_swbuffer->ready_for_write()) {
    // The other end hasn't removed a frame lately; don't bother to
    // render.
    Thread::force_yield();
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

    // Now copy the image to our shared framebuffer.
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
//     Function: SubprocessWindow::set_properties_now
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
//               The properties that have been applied are cleared
//               from the structure by this function; so on return,
//               whatever remains in the properties structure are
//               those that were unchanged for some reason (probably
//               because the underlying interface does not support
//               changing that property on an open window).
////////////////////////////////////////////////////////////////////
void SubprocessWindow::
set_properties_now(WindowProperties &properties) {
  GraphicsWindow::set_properties_now(properties);
}

#endif  // SUPPORT_SUBPROCESS_WINDOW
