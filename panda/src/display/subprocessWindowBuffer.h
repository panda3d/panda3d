/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file subprocessWindowBuffer.h
 * @author drose
 * @date 2009-07-11
 */

#ifndef SUBPROCESSWINDOWBUFFER_H
#define SUBPROCESSWINDOWBUFFER_H

#include <stdio.h>  // perror
#include <assert.h>
#include <string>

/**
 * This is a special class that is designed to faciliate SubprocessWindow.
 * It's intended to be allocated within a shared memory buffer, and it
 * contains space for a framebuffer image to be stored for transferring
 * between processes, as well as appropriate synchronization primitives.
 *
 * It's designed to be compiled outside of Panda, so that code that doesn't
 * link with Panda may still link with this and use it.
 *
 * At the moment, and maybe indefinitely, it is only compiled on OSX, and only
 * when we are building support for the plugin; because it is only needed
 * then.
 */
class SubprocessWindowBuffer {
private:
  void *operator new(size_t, void *addr);
  SubprocessWindowBuffer(int x_size, int y_size);
  SubprocessWindowBuffer(const SubprocessWindowBuffer &copy);
  ~SubprocessWindowBuffer();

public:
  static SubprocessWindowBuffer *new_buffer(int &fd, size_t &mmap_size,
                                            std::string &filename,
                                            int x_size, int y_size);
  static void destroy_buffer(int fd, size_t mmap_size,
                             const std::string &filename,
                             SubprocessWindowBuffer *buffer);

  static SubprocessWindowBuffer *open_buffer(int &fd, size_t &mmap_size,
                                             const std::string &filename);
  static void close_buffer(int fd, size_t mmap_size,
                           const std::string &filename,
                           SubprocessWindowBuffer *buffer);

  bool verify_magic_number() const;

  inline int get_x_size() const;
  inline int get_y_size() const;
  inline size_t get_row_size() const;
  inline size_t get_framebuffer_size() const;

  inline bool ready_for_read() const;
  inline bool ready_for_write() const;

  inline const void *open_read_framebuffer();
  inline void close_read_framebuffer();
  inline void *open_write_framebuffer();
  inline void close_write_framebuffer();

  enum EventSource {
    ES_none,
    ES_mouse,
    ES_keyboard
  };

  enum EventType {
    ET_none,
    ET_button_down,
    ET_button_up,
    ET_button_again,  // if supported
  };

  enum EventFlags {
    EF_has_mouse      = 0x0001,
    EF_mouse_position = 0x0002,
    EF_shift_held     = 0x0004,
    EF_control_held   = 0x0008,
    EF_alt_held       = 0x0010,
    EF_meta_held      = 0x0020,
    EF_caps_lock      = 0x0040,
  };

  class Event {
  public:
    EventSource _source;
    int _code;  // mouse button, or os-specific keycode.
    EventType _type;
    int _x, _y;  // position of mouse at the time of the event, if EF_mouse_position is set
    unsigned int _flags;
  };

  inline bool add_event(const Event &event);
  inline bool has_event() const;
  inline bool get_event(Event &event);

private:
  // The first thing we store in the buffer is a magic number, so we don't
  // accidentally memory-map the wrong file and attempt to treat it as a
  // window buffer.
  enum { magic_number_length = 8 };
  static const char _magic_number[magic_number_length];
  char _this_magic[magic_number_length];

  // Then we have the required size of the entire structure, including its
  // data blocks.
  size_t _mmap_size;

  // Then some other important parameters.
  int _x_size, _y_size;
  size_t _row_size;
  size_t _framebuffer_size;

  // A circular queue of events.
  enum { max_events = 64 };
  int _event_in;  // next slot to write an event to
  int _event_out; // next slot to read an event from
  Event _events[max_events];
  // The queue is empty when _event_in == _event_out.  It is full when
  // _event_in == _event_out - 1, circularly.

  // These sequence numbers are incremented as frames are written and read.
  int _last_written;
  int _last_read;

  // The framebuffer data begins immediately at the end of this class.
};

#include "subprocessWindowBuffer.I"

#endif
