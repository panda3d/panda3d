// Filename: glxGraphicsStateGuardian.h
// Created by:  drose (27Jan03)
//
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

#ifndef GLXGRAPHICSSTATEGUARDIAN_H
#define GLXGRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"

#include "glgsg.h"
#include "glxGraphicsPipe.h"

#include <GL/glx.h>

// This must be included after we have included glgsg.h (which
// includes gl.h).
#include "glxext.h"

// drose: the version of GL/glx.h that ships with Fedora Core 2 seems
// to define GLX_VERSION_1_4, but for some reason does not define
// GLX_SAMPLE_BUFFERS or GLX_SAMPLES.  We work around that here.

#ifndef GLX_SAMPLE_BUFFERS
#define GLX_SAMPLE_BUFFERS                 100000
#endif
#ifndef GLX_SAMPLES
#define GLX_SAMPLES                        100001
#endif

// These typedefs are declared in glxext.h, but we must repeat them
// here, mainly because they will not be included from glxext.h if the
// system GLX version matches or exceeds the GLX version in which
// these functions are defined, and the system glx.h sometimes doesn't
// declare these typedefs.
typedef __GLXextFuncPtr (* PFNGLXGETPROCADDRESSPROC) (const GLubyte *procName);
typedef int (* PFNGLXSWAPINTERVALSGIPROC) (int interval);

////////////////////////////////////////////////////////////////////
//       Class : glxGraphicsStateGuardian
// Description : A tiny specialization on GLGraphicsStateGuardian to
//               add some glx-specific information.
////////////////////////////////////////////////////////////////////
class glxGraphicsStateGuardian : public GLGraphicsStateGuardian {
public:
  glxGraphicsStateGuardian(const FrameBufferProperties &properties,
                           glxGraphicsStateGuardian *share_with,
                           int want_hardware,
                           GLXContext context, XVisualInfo *visual,
                           Display *display, int screen
#ifdef HAVE_GLXFBCONFIG
                           , GLXFBConfig fbconfig
#endif  // HAVE_GLXFBCONFIG
                           );

  virtual ~glxGraphicsStateGuardian();

  virtual void reset();

  bool glx_is_at_least_version(int major_version, int minor_version) const;

  int _want_hardware;
  GLXContext _context;
  XVisualInfo *_visual;
  Display *_display;
  int _screen;

#ifdef HAVE_GLXFBCONFIG
  GLXFBConfig _fbconfig;
#endif  // HAVE_GLXFBCONFIG

public:
  bool _supports_swap_control;
  PFNGLXSWAPINTERVALSGIPROC _glXSwapIntervalSGI;

protected:
  virtual void query_gl_version();
  virtual void get_extra_extensions();
  virtual void *get_extension_func(const char *prefix, const char *name);

private:
  void *get_system_func(const char *name);
  void show_glx_client_string(const string &name, int id);
  void show_glx_server_string(const string &name, int id);


  int _glx_version_major, _glx_version_minor;

  void *_libgl_handle;
  bool _checked_get_proc_address;
  PFNGLXGETPROCADDRESSPROC _glxGetProcAddress;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GLGraphicsStateGuardian::init_type();
    register_type(_type_handle, "glxGraphicsStateGuardian",
                  GLGraphicsStateGuardian::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "glxGraphicsStateGuardian.I"

#endif
