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

#ifndef OSXGRAPHICSSTATEGUARDIAN_H
#define OSXGRAPHICSSTATEGUARDIAN_H
#include <Carbon/Carbon.h>

#define __glext_h_
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
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
  osxGraphicsStateGuardian(GraphicsPipe *pipe,
                           osxGraphicsStateGuardian *share_with);
  virtual ~osxGraphicsStateGuardian();
  virtual void reset();
	
protected:
  virtual void *get_extension_func(const char *prefix, const char *name);
  
public:
  OSStatus buildGL (osxGraphicsWindow  &window);
  AGLContext  get_context(void) { return _aglcontext; };

  // We have to save a pointer to the GSG we intend to share texture
  // context with, since we don't create our own context in the
  // constructor.
  PT(osxGraphicsStateGuardian) _share_with;
  AGLPixelFormat	_aglPixFmt;
  AGLContext		_aglcontext;
  
  const AGLPixelFormat  getAGlPixelFormat() const { return _aglPixFmt; };

public:
  GLint   SharedBuffer;

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
