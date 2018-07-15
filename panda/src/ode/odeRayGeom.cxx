/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeRayGeom.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeRayGeom.h"

TypeHandle OdeRayGeom::_type_handle;

OdeRayGeom::
OdeRayGeom(dGeomID id) :
  OdeGeom(id) {
}

OdeRayGeom::
OdeRayGeom(dReal length) :
  OdeGeom(dCreateRay(nullptr, length)) {
}

OdeRayGeom::
OdeRayGeom(OdeSpace &space, dReal length) :
  OdeGeom(dCreateRay(space.get_id(), length)) {
}

OdeRayGeom::
~OdeRayGeom() {
}
