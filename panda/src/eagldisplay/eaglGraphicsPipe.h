/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file EAGLGraphicsPipe.h
 * @author D. Lawrence
 * @date 2019-01-03
 */

#ifndef EAGLGRAPHICSPIPE_H
#define EAGLGRAPHICSPIPE_H

#include "pandabase.h"
#include "graphicsWindow.h"
#include "graphicsPipe.h"
#include "lightMutex.h"
#include "lightReMutex.h"

class FrameBufferProperties;

/**
 * This graphics pipe will create GLES 2.0 contexts for systems that utilize
 * EAGLContext (iOS, tvOS).
 */
class EAGLGraphicsPipe : public GraphicsPipe {
public:
  EAGLGraphicsPipe();
  virtual ~EAGLGraphicsPipe();
  
  virtual PreferredWindowThread get_preferred_window_thread() const;
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
//  virtual PT(GraphicsStateGuardian) make_callback_gsg(GraphicsEngine *engine);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsPipe::init_type();
    register_type(_type_handle, "EAGLGraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  
private:
  static TypeHandle _type_handle;
};

#include "EAGLGraphicsPipe.I"

#endif

