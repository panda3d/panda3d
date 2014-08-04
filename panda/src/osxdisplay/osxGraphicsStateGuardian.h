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

#ifndef OSXGRAPHICSSTATEGUARDIAN_H
#define OSXGRAPHICSSTATEGUARDIAN_H
#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>

#define __glext_h_
#include <OpenGL/gl.h>
#include <AGL/agl.h>

#include "pandabase.h"
#include "glgsg.h"

#include "osxGraphicsWindow.h"

class osxGraphicsWindow;

////////////////////////////////////////////////////////////////////
//       Class : wglGraphicsStateGuardian
// Description : A tiny specialization on GLGraphicsStateGuardian to
//               add some wgl-specific information.
////////////////////////////////////////////////////////////////////
class osxGraphicsStateGuardian : public GLGraphicsStateGuardian {
public:
  osxGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe,
                           osxGraphicsStateGuardian *share_with);
  virtual ~osxGraphicsStateGuardian();
  virtual void reset();

  void draw_resize_box();

  bool get_gamma_table();
  bool static_set_gamma(bool restore, PN_stdfloat gamma);
  bool set_gamma(PN_stdfloat gamma);
  void atexit_function();
  void restore_gamma();

protected:
  virtual void *do_get_extension_func(const char *name);

public:
  OSStatus build_gl(bool full_screen, bool pbuffer, FrameBufferProperties &fb_props);
  AGLContext get_context() { return _aglcontext; };

  const AGLPixelFormat get_agl_pixel_format() const { return _aglPixFmt; };

private:
  void describe_pixel_format(FrameBufferProperties &fb_props);

  // We have to save a pointer to the GSG we intend to share texture
  // context with, since we don't create our own context in the
  // constructor.
  PT(osxGraphicsStateGuardian) _share_with;
  AGLPixelFormat _aglPixFmt;
  AGLContext _aglcontext;
  CGGammaValue _gOriginalRedTable[ 256 ];
  CGGammaValue _gOriginalGreenTable[ 256 ];
  CGGammaValue _gOriginalBlueTable[ 256 ];
  CGTableCount _sampleCount;
  CGDisplayErr _cgErr;

public:
  GLint _shared_buffer;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GLGraphicsStateGuardian::init_type();
    register_type(_type_handle, "osxGraphicsStateGuardian",
                  GLGraphicsStateGuardian::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class osxGraphicsBuffer;
};


#endif
