// Filename: osMesaGraphicsPipe.h
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

#ifndef OSMESAGRAPHICSPIPE_H
#define OSMESAGRAPHICSPIPE_H

#include "pandabase.h"
#include "graphicsWindow.h"
#include "graphicsPipe.h"
#include "mesagsg.h"

class FrameBufferProperties;

////////////////////////////////////////////////////////////////////
//       Class : OsMesaGraphicsPipe
// Description : This graphics pipe represents the interface for
//               rendering with direct calls to the Mesa open-source
//               software-only implementation of OpenGL.
//
//               Raw Mesa supports only offscreen buffers, but it's
//               possible to create and render into these offscreen
//               buffers without having any X server or other
//               operating system infrastructure in place.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAMESA OsMesaGraphicsPipe : public GraphicsPipe {
public:
  OsMesaGraphicsPipe();
  virtual ~OsMesaGraphicsPipe();

  virtual string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();

protected:
  virtual PT(GraphicsStateGuardian) make_gsg(const FrameBufferProperties &properties,
                                             GraphicsStateGuardian *share_with);
  virtual PT(GraphicsBuffer) make_buffer(GraphicsStateGuardian *gsg, 
                                         const string &name,
                                         int x_size, int y_size, bool want_texture);

private:

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsPipe::init_type();
    register_type(_type_handle, "OsMesaGraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "osMesaGraphicsPipe.I"

#endif
