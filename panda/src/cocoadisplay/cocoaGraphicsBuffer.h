/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaGraphicsBuffer.h
 * @author rdb
 * @date 2017-12-19
 */

#ifndef COCOAGRAPHICSBUFFER_H
#define COCOAGRAPHICSBUFFER_H

#include "pandabase.h"
#include "glgsg.h"

/**
 * This is a light wrapper around GLGraphicsBuffer (ie. FBOs) to interface
 * with Cocoa contexts, so that it can be used without a host window.
 */
class EXPCL_PANDA_COCOADISPLAY CocoaGraphicsBuffer : public GLGraphicsBuffer {
public:
  CocoaGraphicsBuffer(GraphicsEngine *engine, GraphicsPipe *pipe,
                      const std::string &name,
                      const FrameBufferProperties &fb_prop,
                      const WindowProperties &win_prop,
                      int flags,
                      GraphicsStateGuardian *gsg,
                      GraphicsOutput *host);

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

protected:
  virtual void close_buffer();
  virtual bool open_buffer();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GLGraphicsBuffer::init_type();
    register_type(_type_handle, "CocoaGraphicsBuffer",
                  GLGraphicsBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cocoaGraphicsBuffer.I"

#endif
