// Filename: osMesaGraphicsBuffer.h
// Created by:  drose (09Feb04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef OSMESAGRAPHICSBUFFER_H
#define OSMESAGRAPHICSBUFFER_H

#include "pandabase.h"

#include "osMesaGraphicsPipe.h"
#include "graphicsBuffer.h"

////////////////////////////////////////////////////////////////////
//       Class : OsMesaGraphicsBuffer
// Description : An offscreen buffer using direct calls to Mesa.  This
//               is the only kind of graphics output supported by
//               osmesa.h.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAMESA OsMesaGraphicsBuffer : public GraphicsBuffer {
public:
  OsMesaGraphicsBuffer(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                       const string &name,
                       int x_size, int y_size, bool want_texture);

  virtual ~OsMesaGraphicsBuffer();

  virtual void make_current();

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
    register_type(_type_handle, "OsMesaGraphicsBuffer",
                  GraphicsBuffer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "osMesaGraphicsBuffer.I"

#endif
