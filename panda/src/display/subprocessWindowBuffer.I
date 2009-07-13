// Filename: subprocessWindowBuffer.I
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


////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindowBuffer::get_x_size
//       Access: Public
//  Description: Returns the width of the framebuffer in pixels.
////////////////////////////////////////////////////////////////////
inline int SubprocessWindowBuffer::
get_x_size() const {
  return _x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindowBuffer::get_y_size
//       Access: Public
//  Description: Returns the height of the framebuffer in pixels.
////////////////////////////////////////////////////////////////////
inline int SubprocessWindowBuffer::
get_y_size() const {
  return _y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindowBuffer::get_row_size
//       Access: Public
//  Description: Returns the length of a row of the framebuffer, in
//               bytes.
////////////////////////////////////////////////////////////////////
inline size_t SubprocessWindowBuffer::
get_row_size() const {
  return _row_size;
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindowBuffer::get_framebuffer_size
//       Access: Public
//  Description: Returns the total number of bytes in the framebuffer.
////////////////////////////////////////////////////////////////////
inline size_t SubprocessWindowBuffer::
get_framebuffer_size() const {
  return _framebuffer_size;
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindowBuffer::ready_for_read
//       Access: Public
//  Description: Returns true if the framebuffer data has been updated
//               since open_read_framebuffer() was last called.
////////////////////////////////////////////////////////////////////
inline bool SubprocessWindowBuffer::
ready_for_read() const {
  return (_last_written != _last_read);
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindowBuffer::ready_for_write
//       Access: Public
//  Description: Returns true if the framebuffer data has been read
//               since open_write_framebuffer() was last called.
////////////////////////////////////////////////////////////////////
inline bool SubprocessWindowBuffer::
ready_for_write() const {
  return (_last_written == _last_read);
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindowBuffer::open_read_framebuffer
//       Access: Public
//  Description: Returns a read-only pointer to the framebuffer.  It
//               is only valid to call this if ready_for_read() has
//               returned true.
//
//               You must call close_read_framebuffer() to indicate
//               you have finished reading.
////////////////////////////////////////////////////////////////////
inline const void *SubprocessWindowBuffer::
open_read_framebuffer() {
  assert(ready_for_read());
  return (void *)(this + 1);
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindowBuffer::close_read_framebuffer
//       Access: Public
//  Description: Releases the framebuffer after a previous call to
//               open_read_framebuffer().
////////////////////////////////////////////////////////////////////
inline void SubprocessWindowBuffer::
close_read_framebuffer() {
  _last_read = _last_written;
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindowBuffer::open_write_framebuffer
//       Access: Public
//  Description: Returns a writable pointer to the framebuffer.  It
//               is only valid to call this if ready_for_write() has
//               returned true.
//
//               You must call close_write_framebuffer() to indicate
//               you have finished writing.
////////////////////////////////////////////////////////////////////
inline void *SubprocessWindowBuffer::
open_write_framebuffer() {
  assert(ready_for_write());
  return (void *)(this + 1);
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindowBuffer::close_write_framebuffer
//       Access: Public
//  Description: Releases the framebuffer after a previous call to
//               open_write_framebuffer().
////////////////////////////////////////////////////////////////////
inline void SubprocessWindowBuffer::
close_write_framebuffer() {
  ++_last_written;
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindowBuffer::add_event
//       Access: Public
//  Description: Adds a new Event to the queue.  Returns false
//               if the queue was full.
////////////////////////////////////////////////////////////////////
inline bool SubprocessWindowBuffer::
add_event(const SubprocessWindowBuffer::Event &event) {
  if (((_event_in + 1) % max_events) == _event_out) {
    // The queue is full.
    return false;
  }
  _events[_event_in] = event;
  _event_in = (_event_in + 1) % max_events;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindowBuffer::has_event
//       Access: Public
//  Description: Returns true if the queue has at least one
//               Event to extract, false if it is empty.
////////////////////////////////////////////////////////////////////
inline bool SubprocessWindowBuffer::
has_event() const {
  return (_event_in != _event_out);
}

////////////////////////////////////////////////////////////////////
//     Function: SubprocessWindowBuffer::get_event
//       Access: Public
//  Description: If the queue is nonempty, fills event with the first
//               Event on the queue and returns true.  If the queue is
//               empty, returns false.
////////////////////////////////////////////////////////////////////
inline bool SubprocessWindowBuffer::
get_event(SubprocessWindowBuffer::Event &event) {
  if (_event_in == _event_out) {
    return false;
  }
  event = _events[_event_out];
  _event_out = (_event_out + 1) % max_events;
  return true;
}
