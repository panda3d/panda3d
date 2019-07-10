/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eaglGraphicsBuffer.h
 * @author D. Lawrence
 * @date 2019-07-09
 */

#ifndef EAGLGRAPHICSBUFFER_H
#define EAGLGRAPHICSBUFFER_H

#include "pandabase.h"
#include "gles2gsg.h"



/**
 * This is a light wrapper around GLGraphicsBuffer (ie. FBOs) to interface
 * with Cocoa contexts, so that it can be used without a host window.
 */
class EAGLGraphicsBuffer : public GLES2GraphicsBuffer {
public:
  EAGLGraphicsBuffer(GraphicsEngine *engine, GraphicsPipe *pipe,
                      const std::string &name,
                      const FrameBufferProperties &fb_prop,
                      const WindowProperties &win_prop,
                      int flags,
                      GraphicsStateGuardian *gsg,
                      GraphicsOutput *host);

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GLES2GraphicsBuffer::init_type();
    register_type(_type_handle, "EAGLGraphicsBuffer",
                  GLES2GraphicsBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
