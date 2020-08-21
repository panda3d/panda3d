/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tinyXGraphicsPipe.h
 * @author drose
 * @date 2008-05-03
 */

#ifndef TINYXGRAPHICSPIPE_H
#define TINYXGRAPHICSPIPE_H

#include "pandabase.h"

#ifdef HAVE_X11

#include "x11GraphicsWindow.h"
#include "x11GraphicsPipe.h"
#include "tinyGraphicsStateGuardian.h"
#include "lightMutex.h"
#include "lightReMutex.h"

/**
 * This graphics pipe represents the interface for creating TinyPanda graphics
 * windows on an X11-based (e.g.  Unix) client.
 */
class EXPCL_TINYDISPLAY TinyXGraphicsPipe : public x11GraphicsPipe {
public:
  TinyXGraphicsPipe(const std::string &display = std::string());
  virtual ~TinyXGraphicsPipe();

  virtual std::string get_interface_name() const;
  static PT(GraphicsPipe) pipe_constructor();

protected:
  virtual PT(GraphicsOutput) make_output(const std::string &name,
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
    x11GraphicsPipe::init_type();
    register_type(_type_handle, "TinyXGraphicsPipe",
                  x11GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinyXGraphicsPipe.I"

#endif  // HAVE_X11

#endif
