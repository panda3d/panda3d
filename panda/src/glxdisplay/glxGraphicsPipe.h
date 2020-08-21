/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glxGraphicsPipe.h
 * @author mike
 * @date 1997-01-09
 */

#ifndef GLXGRAPHICSPIPE_H
#define GLXGRAPHICSPIPE_H

#include "pandabase.h"
#include "graphicsWindow.h"
#include "graphicsPipe.h"
#include "glgsg.h"
#include "lightMutex.h"
#include "lightReMutex.h"
#include "x11GraphicsPipe.h"

class FrameBufferProperties;

#ifndef CPPPARSER

// Don't pick up the system glxext.h; use our own, which is better.
#define __glxext_h_

#include "pre_x11_include.h"
#include <GL/glx.h>
#include "post_x11_include.h"

// This must be included after we have included glgsg.h (which includes gl.h),
// and after we have checked GLX_VERSION_1_3.  But we must also include it
// before we redefine the GLXFBConfig types, below.
#include "panda_glxext.h"

// drose: the version of GLglx.h that ships with Fedora Core 2 seems to define
// GLX_VERSION_1_4, but for some reason does not define GLX_SAMPLE_BUFFERS or
// GLX_SAMPLES.  We work around that here.

#ifndef GLX_SAMPLE_BUFFERS
#define GLX_SAMPLE_BUFFERS                 100000
#endif
#ifndef GLX_SAMPLES
#define GLX_SAMPLES                        100001
#endif


/*
#if !defined(HAVE_GLXFBCONFIG) && defined(GLX_SGIX_fbconfig) && defined(GLX_SGIX_pbuffer)
  // If the system glx version isn't 1.3, but these were defined as
  // extensions, we can work with that.
  #define GLXFBConfig GLXFBConfigSGIX
  #define GLXPbuffer GLXPbufferSGIX
  #define glXChooseFBConfig glXChooseFBConfigSGIX
  #define glXCreateNewContext glXCreateContextWithConfigSGIX
  #define glXGetVisualFromFBConfig glXGetVisualFromFBConfigSGIX
  #define glXGetFBConfigAttrib glXGetFBConfigAttribSGIX
  #define glXDestroyPbuffer glXDestroyGLXPbufferSGIX

  #define HAVE_GLXFBCONFIG
  #define HAVE_SGI_GLXFBCONFIG
#endif
*/

#endif  // CPPPARSER

/**
 * This graphics pipe represents the interface for creating OpenGL graphics
 * windows on an X-based (e.g.  Unix) client.
 */
class glxGraphicsPipe : public x11GraphicsPipe {
public:
  glxGraphicsPipe(const std::string &display = std::string());
  virtual ~glxGraphicsPipe() {};

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
  virtual PT(GraphicsStateGuardian) make_callback_gsg(GraphicsEngine *engine);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    x11GraphicsPipe::init_type();
    register_type(_type_handle, "glxGraphicsPipe",
                  x11GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glxGraphicsPipe.I"

#endif
