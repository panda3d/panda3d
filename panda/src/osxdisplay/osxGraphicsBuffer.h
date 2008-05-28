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

#ifndef OSXGRAPHICSBUFFER_H
#define OSXGRAPHICSBUFFER_H
#include <Carbon/Carbon.h>

#define __glext_h_
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <AGL/agl.h>

#include "pandabase.h"
#include "graphicsBuffer.h"
#include "glgsg.h"

////////////////////////////////////////////////////////////////////
//       Class : OSXGraphicsBuffer
//  rhh mar-2006
//  Sorry ... this is not functional at all... I have no need for it yet ?
// 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAGL osxGraphicsBuffer : public GraphicsBuffer {
public:
  osxGraphicsBuffer(GraphicsPipe *pipe, 
                    const string &name,
                    const FrameBufferProperties &fb_prop,
                    const WindowProperties &win_prop,
                    int flags,
                    GraphicsStateGuardian *gsg,
                    GraphicsOutput *host);
  virtual ~osxGraphicsBuffer();


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
    GraphicsBuffer::init_type();
    register_type(_type_handle, "owsGraphicsBuffer",
                  GraphicsBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class osxGraphicsStateGuardian;
};


#endif
