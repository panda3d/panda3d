/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file EAGLGraphicsStateGuardian.h
 * @author D. Lawrence
 * @date 2019-01-03
 */

#ifndef EAGLGRAPHICSSTATEGUARDIAN_H
#define EAGLGRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"
#include "gles2gsg.h"
#include "eaglGraphicsPipe.h"


#import <OpenGLES/EAGL.h>

/**
 * A variation of GLES2GraphicsStateGuardian to support Apple's EGL
 * implementation.
 */
class EAGLGraphicsStateGuardian : public GLES2GraphicsStateGuardian {
public:
  EAGLGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe,
                            EAGLGraphicsStateGuardian *share_with);
  virtual ~EAGLGraphicsStateGuardian();
  
  INLINE const FrameBufferProperties &get_fb_properties() const;
  void get_properties(FrameBufferProperties &properties);
  void choose_pixel_format(const FrameBufferProperties &properties,
                           bool need_pbuffer);
  
  FrameBufferProperties _fbprops;
  EAGLContext *_context;
  
  Mutex _context_lock;
  
protected:
  virtual void *do_get_extension_func(const char *name);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GLES2GraphicsStateGuardian::init_type();
    register_type(_type_handle, "EAGLGraphicsStateGuardian",
                  GLES2GraphicsStateGuardian::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  
private:
  static TypeHandle _type_handle;
};

#include "EAGLGraphicsStateGuardian.I"

#endif
