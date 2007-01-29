// Filename: odeSphereGeom.cxx
// Created by:  joswilso (27Dec06)
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

#include "config_ode.h"
#include "odeSphereGeom.h"

TypeHandle OdeSphereGeom::_type_handle;

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
