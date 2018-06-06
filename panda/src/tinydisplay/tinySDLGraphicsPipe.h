/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file tinySDLGraphicsPipe.h
 * @author drose
 * @date 2008-04-24
 */

#ifndef TINYSDLGRAPHICSPIPE_H
#define TINYSDLGRAPHICSPIPE_H

#include "pandabase.h"

#ifdef HAVE_SDL

#include "graphicsWindow.h"
#include "graphicsPipe.h"

class FrameBufferProperties;

/**
 * This graphics pipe manages SDL windows for rendering TinyPanda software
 * buffers.
 */
class EXPCL_TINYDISPLAY TinySDLGraphicsPipe : public GraphicsPipe {
public:
  TinySDLGraphicsPipe();
  virtual ~TinySDLGraphicsPipe();

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
    GraphicsPipe::init_type();
    register_type(_type_handle, "TinySDLGraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "tinySDLGraphicsPipe.I"

#endif  // HAVE_SDL

#endif
