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

////////////////////////////////////////////////////////////////////
//       Class : glxGraphicsStateGuardian
// Description : A tiny specialization on GLGraphicsStateGuardian to
//               add some glx-specific information.
////////////////////////////////////////////////////////////////////
class glxGraphicsStateGuardian : public GLGraphicsStateGuardian {
public:
  glxGraphicsStateGuardian(const FrameBufferProperties &properties,
                           GLXContext context, GLXFBConfig fbconfig,
                           Display *display, int screen);
  virtual ~glxGraphicsStateGuardian();

  GLXContext _context;
  GLXFBConfig _fbconfig;
  Display *_display;
  int _screen;

protected:
  virtual void get_extra_extensions();

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
