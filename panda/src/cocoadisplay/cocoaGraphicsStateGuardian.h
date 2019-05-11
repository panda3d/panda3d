/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cocoaGraphicsStateGuardian.h
 * @author rdb
 * @date 2012-05-14
 */

#ifndef COCOAGRAPHICSSTATEGUARDIAN_H
#define COCOAGRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"
#include "cocoaGraphicsPipe.h"
#include "glgsg.h"

#import <AppKit/NSOpenGL.h>
#import <OpenGL/OpenGL.h>
#import <CoreVideo/CoreVideo.h>

/**
 * A tiny specialization on GLGraphicsStateGuardian to add some Cocoa-specific
 * information.
 */
class EXPCL_PANDA_COCOADISPLAY CocoaGraphicsStateGuardian : public GLGraphicsStateGuardian {
public:
  INLINE const FrameBufferProperties &get_fb_properties() const;
  void get_properties(FrameBufferProperties &properties,
                      NSOpenGLPixelFormat *pixel_format, int virtual_screen);
  void choose_pixel_format(const FrameBufferProperties &properties,
                           CGDirectDisplayID display,
                           bool need_pbuffer);

  CocoaGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe,
                             CocoaGraphicsStateGuardian *share_with);

  virtual ~CocoaGraphicsStateGuardian();
  bool setup_vsync();

  INLINE void lock_context();
  INLINE void unlock_context();

  NSOpenGLContext *_share_context;
  NSOpenGLContext *_context;
  NSOpenGLPixelFormat *_format = nullptr;
  FrameBufferProperties _fbprops;

  CVDisplayLinkRef _display_link = nullptr;
  TrueMutexImpl _swap_lock;
  TrueConditionVarImpl _swap_condition;
  AtomicAdjust::Integer _last_wait_frame = 0;

protected:
  virtual void query_gl_version();
  virtual void *do_get_extension_func(const char *name);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GLGraphicsStateGuardian::init_type();
    register_type(_type_handle, "CocoaGraphicsStateGuardian",
                  GLGraphicsStateGuardian::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cocoaGraphicsStateGuardian.I"

#endif
