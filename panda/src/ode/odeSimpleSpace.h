// Filename: odeSimpleSpace.h
// Created by:  joswilso (27Dec06)
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

#ifndef ODESIMPLESPACE_H
#define ODESIMPLESPACE_H

#include "pandabase.h"
#include "luse.h"

#include "ode_includes.h"
#include "odeSpace.h"


////////////////////////////////////////////////////////////////////
//       Class : OdeSimpleSpace
// Description : 
////////////////////////////////////////////////////////////////////c
class EXPCL_PANDAODE OdeSimpleSpace : public OdeSpace {
  friend class OdeSpace;
  friend class OdeGeom;

public:
  OdeSimpleSpace(dSpaceID id);

PUBLISHED:
  OdeSimpleSpace();
  OdeSimpleSpace(OdeSpace &space);
  virtual ~OdeSimpleSpace();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OdeSpace::init_type();
    register_type(_type_handle, "OdeSimpleSpace",
                  OdeSpace::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "odeSimpleSpace.I"

#endif

