// Filename: wglGraphicsStateGuardian.h
// Created by:  drose (27Jan03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef WGLGRAPHICSSTATEGUARDIAN_H
#define WGLGRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"

#include "glGraphicsStateGuardian.h"

////////////////////////////////////////////////////////////////////
//       Class : wglGraphicsStateGuardian
// Description : A tiny specialization on GLGraphicsStateGuardian to
//               add some wgl-specific information.
////////////////////////////////////////////////////////////////////
class wglGraphicsStateGuardian : public GLGraphicsStateGuardian {
public:
  wglGraphicsStateGuardian(const FrameBufferProperties &properties, int pfnum);
  virtual ~wglGraphicsStateGuardian();

  INLINE int get_pfnum() const;
  INLINE HGLRC get_context(HDC hdc);

private:
  void make_context(HDC hdc);

  // All windows that share a particular GL context must also share
  // the same pixel format; therefore, we store the pixel format
  // number in the GSG.
  int _pfnum;

  bool _made_context;
  HGLRC _context;

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
