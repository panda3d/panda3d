/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeSphereGeom.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeSphereGeom.h"

TypeHandle OdeSphereGeom::_type_handle;

OdeSphereGeom::
OdeSphereGeom(dGeomID id) :
  OdeGeom(id) {
}

OdeSphereGeom::
OdeSphereGeom(dReal radius) :
  OdeGeom(dCreateSphere(nullptr, radius)) {
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
