// Filename: osMesaGraphicsPipe.h
// Created by:  drose (09Feb04)
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
  virtual PT(GraphicsOutput) make_output(const string &name,
                                         const FrameBufferProperties &fb_prop,
                                         const WindowProperties &win_prop,
                                         int flags,
                                         GraphicsEngine *engine,
                                         GraphicsStateGuardian *gsg,
                                         GraphicsOutput *host,
                                         int retry,
                                         bool &precertify);

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
