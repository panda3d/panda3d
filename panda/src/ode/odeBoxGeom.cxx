// Filename: odeBoxGeom.cxx
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

#include "config_ode.h"
#include "odeBoxGeom.h"

TypeHandle OdeBoxGeom::_type_handle;

OdeBoxGeom::
OdeBoxGeom(dGeomID id) :
  OdeGeom(id) {
}

OdeBoxGeom::
OdeBoxGeom(dReal lx, dReal ly, dReal lz) :
  OdeGeom(dCreateBox(0, lx, ly, lz)) {
}

OdeBoxGeom::
OdeBoxGeom(OdeSpace &space, dReal lx, dReal ly, dReal lz) :
  OdeGeom(dCreateBox(space.get_id(), lx, ly, lz)) {
}

OdeBoxGeom::
OdeBoxGeom(OdeSpace &space, const LVecBase3f &size) :
  OdeGeom(dCreateBox(space.get_id(), size[0], size[1], size[2])) {
}

OdeBoxGeom::
~OdeBoxGeom() {
}
