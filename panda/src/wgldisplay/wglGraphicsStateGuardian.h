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
  wglGraphicsStateGuardian(const FrameBufferProperties &properties);
  virtual ~wglGraphicsStateGuardian();

  HGLRC _context;
  int _pfnum;
  PIXELFORMATDESCRIPTOR _pixelformat;

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
