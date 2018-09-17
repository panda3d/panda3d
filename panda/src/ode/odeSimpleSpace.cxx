/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeSimpleSpace.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeSimpleSpace.h"

TypeHandle OdeSimpleSpace::_type_handle;

OdeSimpleSpace::
OdeSimpleSpace(dSpaceID id) :
  OdeSpace(id) {
}

OdeSimpleSpace::
OdeSimpleSpace() :
  OdeSpace(dSimpleSpaceCreate(nullptr)) {
}

OdeSimpleSpace::
OdeSimpleSpace(OdeSpace &space) :
  OdeSpace(dSimpleSpaceCreate(space.get_id())) {
}

OdeSimpleSpace::
~OdeSimpleSpace() {
}
