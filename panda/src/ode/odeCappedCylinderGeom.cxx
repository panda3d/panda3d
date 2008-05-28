// Filename: odeCappedCylinderGeom.cxx
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
#include "odeCappedCylinderGeom.h"

TypeHandle OdeCappedCylinderGeom::_type_handle;

OdeCappedCylinderGeom::
OdeCappedCylinderGeom(dGeomID id) :
  OdeGeom(id) {
}

OdeCappedCylinderGeom::
OdeCappedCylinderGeom(dReal radius, dReal length) :
  OdeGeom(dCreateCapsule(0, radius, length)) {
}

OdeCappedCylinderGeom::
OdeCappedCylinderGeom(OdeSpace &space, dReal radius, dReal length) :
  OdeGeom(dCreateCapsule(space.get_id(), radius, length)) {
}

OdeCappedCylinderGeom::
~OdeCappedCylinderGeom() {
}
