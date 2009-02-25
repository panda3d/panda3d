// Filename: tinyOffscreenGraphicsPipe.h
// Created by:  drose (09Feb09)
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

#ifndef TINYOFFSCREENGRAPHICSPIPE_H
#define TINYOFFSCREENGRAPHICSPIPE_H

#include "pandabase.h"

#include "graphicsWindow.h"
#include "graphicsPipe.h"
#include "tinyGraphicsStateGuardian.h"

class FrameBufferProperties;

////////////////////////////////////////////////////////////////////
//       Class : TinyOffscreenGraphicsPipe
// Description : This graphics pipe creates offscreen buffers only,
//               but is completely platform-independent.
////////////////////////////////////////////////////////////////////
class EXPCL_TINYDISPLAY TinyOffscreenGraphicsPipe : public GraphicsPipe {
public:
  TinyOffscreenGraphicsPipe();
  virtual ~TinyOffscreenGraphicsPipe();

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
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsPipe::init_type();
    register_type(_type_handle, "TinyOffscreenGraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinyOffscreenGraphicsPipe.I"

#endif
