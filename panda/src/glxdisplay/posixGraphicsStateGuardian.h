// Filename: posixGraphicsStateGuardian.h
// Created by:  drose (14Jan12)
//
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

#ifndef POSIXGRAPHICSSTATEGUARDIAN_H
#define POSIXGRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"

#include "glgsg.h"

////////////////////////////////////////////////////////////////////
//       Class : PosixGraphicsStateGuardian
// Description : This GSG is used only for CallbackGraphicsWindow
//               (which might not be using the glx interfaces), to add
//               the ability to peek in libGL.so to find the extension
//               functions.
////////////////////////////////////////////////////////////////////
class PosixGraphicsStateGuardian : public GLGraphicsStateGuardian {
public:
  PosixGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe);
  ~PosixGraphicsStateGuardian();

protected:
  virtual void *do_get_extension_func(const char *name);
  void *get_system_func(const char *name);

private:
  void *_libgl_handle;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GLGraphicsStateGuardian::init_type();
    register_type(_type_handle, "PosixGraphicsStateGuardian",
                  GLGraphicsStateGuardian::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "posixGraphicsStateGuardian.I"

#endif
