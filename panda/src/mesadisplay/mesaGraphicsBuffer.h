// Filename: mesaGraphicsBuffer.h
// Created by:  drose (09Feb04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef MESAGRAPHICSBUFFER_H
#define MESAGRAPHICSBUFFER_H

#include "pandabase.h"

#include "mesaGraphicsPipe.h"
#include "graphicsBuffer.h"

////////////////////////////////////////////////////////////////////
//       Class : MesaGraphicsBuffer
// Description : An offscreen buffer using direct calls to Mesa.  This
//               is the only kind of graphics output supported by
//               osmesa.h.
////////////////////////////////////////////////////////////////////
class MesaGraphicsBuffer : public GraphicsBuffer {
public:
  MesaGraphicsBuffer(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                    int x_size, int y_size, bool want_texture);

  virtual ~MesaGraphicsBuffer();

  virtual void make_current();

  virtual void begin_flip();

protected:
  virtual void close_buffer();
  virtual bool open_buffer();

private:
  GLenum _type;
  PTA_uchar _image;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsBuffer::init_type();
    register_type(_type_handle, "MesaGraphicsBuffer",
                  GraphicsBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "mesaGraphicsBuffer.I"

#endif
