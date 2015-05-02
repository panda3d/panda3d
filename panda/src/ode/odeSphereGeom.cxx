// Filename: odeSphereGeom.cxx
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
#include "odeSphereGeom.h"

TypeHandle OdeSphereGeom::_type_handle;

OdeSphereGeom::
OdeSphereGeom(dGeomID id) :
  OdeGeom(id) {
}

OdeSphereGeom::
OdeSphereGeom(dReal radius) :
  OdeGeom(dCreateSphere(0, radius)) {
}

OdeSphereGeom::
OdeSphereGeom(OdeGeom &geom) :
  OdeGeom(geom.get_id()) {
  nassertv(dGeomGetClass(_id) == dSphereClass);
}

OdeSphereGeom::
OdeSphereGeom(OdeSpace &space, dReal radius) :
  OdeGeom(dCreateSphere(space.get_id(), radius)) {
}

OdeSphereGeom::
~OdeSphereGeom() {
}
