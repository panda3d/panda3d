/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file subprocessWindow.h
 * @author drose
 * @date 2009-07-11
 */

#ifndef SUBPROCESSWINDOW_H
#define SUBPROCESSWINDOW_H

#include "pandabase.h"

// For now, a simple trigger whether to enable the subprocess window support.
// We only build it on OSX, because this is (presently) the only case where
// it's useful.
#ifdef IS_OSX
#define SUPPORT_SUBPROCESS_WINDOW 1
#else
#undef SUPPORT_SUBPROCESS_WINDOW
#endif

#ifdef SUPPORT_SUBPROCESS_WINDOW

#include "graphicsWindow.h"
#include "graphicsBuffer.h"
#include "texture.h"
#include "subprocessWindowBuffer.h"
#include "filename.h"

/**
 * This is a special "window" that actually renders to an offscreen buffer,
 * copies the pixels to RAM, and then ships them to a parent process via
 * shared memory for rendering to the window.
 *
 * This whole nonsense is necessary because OSX doesn't allow child processes
 * to draw to, or attach windows to, windows created in the parent process.
 * There's a rumor that 10.6 fixes this nonsense; this will remain to be seen.
 */
class SubprocessWindow : public GraphicsWindow {
public:
  SubprocessWindow(GraphicsEngine *engine, GraphicsPipe *pipe,
                   const std::string &name,
                   const FrameBufferProperties &fb_prop,
                   const WindowProperties &win_prop,
                   int flags,
                   GraphicsStateGuardian *gsg,
                   GraphicsOutput *host);
  virtual ~SubprocessWindow();

  virtual void process_events();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);
  virtual void begin_flip();

  virtual void set_properties_now(WindowProperties &properties);

protected:
  virtual void close_window();
  virtual bool open_window();

private:
  void internal_close_window();
  bool internal_open_window();

  ButtonHandle translate_key(int &keycode, int os_code, unsigned int flags) const;
  void transition_button(unsigned int flag, ButtonHandle button);

private:
  PT(GraphicsBuffer) _buffer;
  PT(Texture) _texture;
  PT(GraphicsWindowInputDevice) _input;

  int _fd;
  size_t _mmap_size;
  Filename _filename;
  SubprocessWindowBuffer *_swbuffer;

  unsigned int _last_event_flags;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsWindow::init_type();
    register_type(_type_handle, "SubprocessWindow",
                  GraphicsWindow::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "subprocessWindow.I"

#endif  // SUPPORT_SUBPROCESS_WINDOW

#endif
