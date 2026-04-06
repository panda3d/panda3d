/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file graphicsBuffer.h
 * @author drose
 * @date 2004-02-06
 */

#ifndef GRAPHICSBUFFER_H
#define GRAPHICSBUFFER_H

#include "pandabase.h"

#include "graphicsOutput.h"
#include "texture.h"
#include "pointerTo.h"

/**
 * An offscreen buffer for rendering into.  This is similar in function to a
 * GraphicsWindow, except that the output is not visible to the user.
 */
class EXPCL_PANDA_DISPLAY GraphicsBuffer : public GraphicsOutput {
protected:
  GraphicsBuffer(GraphicsEngine *engine,
                 GraphicsPipe *pipe,
                 const std::string &name,
                 const FrameBufferProperties &fb_prop,
                 const WindowProperties &win_prop,
                 int flags,
                 GraphicsStateGuardian *gsg,
                 GraphicsOutput *host);

PUBLISHED:
  virtual ~GraphicsBuffer();
  virtual void set_size(int x, int y);

public:
  virtual void request_open();
  virtual void request_close();

  // It is an error to call any of the following methods from any thread other
  // than the window thread.  These methods are normally called by the
  // GraphicsEngine.
  virtual void set_close_now();
  virtual void process_events();

protected:
  virtual void close_buffer();
  virtual bool open_buffer();

protected:
  enum OpenRequest {
    OR_none,
    OR_open,
    OR_close,
  };
  OpenRequest _open_request;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsOutput::init_type();
    register_type(_type_handle, "GraphicsBuffer",
                  GraphicsOutput::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "graphicsBuffer.I"

#endif
