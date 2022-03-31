/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeConvexGeom.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeConvexGeom.h"

TypeHandle OdeConvexGeom::_type_handle;

OdeConvexGeom::
OdeConvexGeom(dGeomID id) :
  OdeGeom(id) {
}

OdeConvexGeom::
OdeConvexGeom(dReal radius, dReal length) :
  OdeGeom(dCreateConvex(nullptr, radius, length)) {
}

OdeConvexGeom::
OdeConvexGeom(OdeSpace &space, dReal radius, dReal length) :
  OdeGeom(dCreateConvex(space.get_id(), radius, length)) {
}

OdeConvexGeom::
~OdeConvexGeom() {
}
