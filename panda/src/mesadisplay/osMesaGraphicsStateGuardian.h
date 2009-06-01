// Filename: osMesaGraphicsStateGuardian.h
// Created by:  drose (09Feb04)
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

#ifndef OSMESAGRAPHICSSTATEGUARDIAN_H
#define OSMESAGRAPHICSSTATEGUARDIAN_H

#include "pandabase.h"
#include "mesagsg.h"

////////////////////////////////////////////////////////////////////
//       Class : OSMesaGraphicsStateGuardian
// Description : A tiny specialization on MesaGraphicsStateGuardian to
//               add a reference to the OSMesaContext.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAMESA OSMesaGraphicsStateGuardian : public MesaGraphicsStateGuardian {
public:
  OSMesaGraphicsStateGuardian(GraphicsEngine *engine, GraphicsPipe *pipe,
                              OSMesaGraphicsStateGuardian *share_with);
  virtual ~OSMesaGraphicsStateGuardian();

  OSMesaContext _context;

protected:
  virtual void *do_get_extension_func(const char *prefix, const char *name);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MesaGraphicsStateGuardian::init_type();
    register_type(_type_handle, "OSMesaGraphicsStateGuardian",
                  MesaGraphicsStateGuardian::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "osMesaGraphicsStateGuardian.I"

#endif
