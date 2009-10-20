// Filename: tinyGraphicsBuffer.h
// Created by:  drose (08Aug08)
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

#ifndef TINYGRAPHICSBUFFER_H
#define TINYGRAPHICSBUFFER_H

#include "pandabase.h"
#include "graphicsBuffer.h"
#include "tinyGraphicsStateGuardian.h"

////////////////////////////////////////////////////////////////////
//       Class : TinyGraphicsBuffer
// Description : An offscreen graphics buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_TINYDISPLAY TinyGraphicsBuffer : public GraphicsBuffer {
public:
  TinyGraphicsBuffer(GraphicsEngine *engine, GraphicsPipe *pipe, 
                     const string &name,
                     const FrameBufferProperties &fb_prop,
                     const WindowProperties &win_prop,
                     int flags,
                     GraphicsStateGuardian *gsg,
                     GraphicsOutput *host);
  virtual ~TinyGraphicsBuffer();

  virtual bool begin_frame(FrameMode mode, Thread *current_thread);
  virtual void end_frame(FrameMode mode, Thread *current_thread);

  INLINE ZBuffer *get_frame_buffer();

protected:
  virtual void close_buffer();
  virtual bool open_buffer();

private:
  void create_frame_buffer();

private:
  ZBuffer *_frame_buffer;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsBuffer::init_type();
    register_type(_type_handle, "TinyGraphicsBuffer",
                  GraphicsBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinyGraphicsBuffer.I"

#endif

