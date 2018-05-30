/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeCylinderGeom.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeCylinderGeom.h"

TypeHandle OdeCylinderGeom::_type_handle;

OdeCylinderGeom::
OdeCylinderGeom(dGeomID id) :
  OdeGeom(id) {
}

OdeCylinderGeom::
OdeCylinderGeom(dReal radius, dReal length) :
  OdeGeom(dCreateCylinder(nullptr, radius, length)) {
}

OdeCylinderGeom::
OdeCylinderGeom(OdeSpace &space, dReal radius, dReal length) :
  OdeGeom(dCreateCylinder(space.get_id(), radius, length)) {
}

OdeCylinderGeom::
~OdeCylinderGeom() {
}
