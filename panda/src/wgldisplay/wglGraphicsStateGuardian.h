// Filename: wglGraphicsStateGuardian.h
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

#ifndef WGLGRAPHICSSTATEGUARDIAN_H
#define WGLGRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"

#include "glgsg.h"

// This must be included after we have included glgsg.h (which
// includes gl.h).
#include "wglext.h"

////////////////////////////////////////////////////////////////////
//       Class : wglGraphicsStateGuardian
// Description : A tiny specialization on GLGraphicsStateGuardian to
//               add some wgl-specific information.
////////////////////////////////////////////////////////////////////
class wglGraphicsStateGuardian : public GLGraphicsStateGuardian {
public:
  wglGraphicsStateGuardian(const FrameBufferProperties &properties, 
                           wglGraphicsStateGuardian *share_with,
                           int pfnum);
  virtual ~wglGraphicsStateGuardian();

  INLINE int get_pfnum() const;
  INLINE bool made_context() const;
  INLINE HGLRC get_context(HDC hdc);

  virtual void reset();

protected:
  virtual void get_extra_extensions();

private:
  void make_context(HDC hdc);
  HGLRC get_share_context() const;
  void redirect_share_pool(wglGraphicsStateGuardian *share_with);

  // We have to save a pointer to the GSG we intend to share texture
  // context with, since we don't create our own context in the
  // constructor.
  PT(wglGraphicsStateGuardian) _share_with;

  // All windows that share a particular GL context must also share
  // the same pixel format; therefore, we store the pixel format
  // number in the GSG.
  int _pfnum;

  bool _made_context;
  HGLRC _context;

public:
  bool _supports_pbuffer;

  PFNWGLCREATEPBUFFERARBPROC _wglCreatePbufferARB;
  PFNWGLGETPBUFFERDCARBPROC _wglGetPbufferDCARB;
  PFNWGLRELEASEPBUFFERDCARBPROC _wglReleasePbufferDCARB;
  PFNWGLDESTROYPBUFFERARBPROC _wglDestroyPbufferARB;
  PFNWGLQUERYPBUFFERARBPROC _wglQueryPbufferARB;

  bool _supports_pixel_format;

  PFNWGLGETPIXELFORMATATTRIBIVARBPROC _wglGetPixelFormatAttribivARB;
  PFNWGLGETPIXELFORMATATTRIBFVARBPROC _wglGetPixelFormatAttribfvARB;
  PFNWGLCHOOSEPIXELFORMATARBPROC _wglChoosePixelFormatARB;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GLGraphicsStateGuardian::init_type();
    register_type(_type_handle, "wglGraphicsStateGuardian",
                  GLGraphicsStateGuardian::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "wglGraphicsStateGuardian.I"

#endif
