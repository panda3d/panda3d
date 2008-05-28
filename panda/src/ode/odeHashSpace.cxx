// Filename: odeHashSpace.cxx
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
#include "odeHashSpace.h"

TypeHandle OdeHashSpace::_type_handle;

OdeHashSpace::
OdeHashSpace(dSpaceID id) :
  OdeSpace(id) {
}

OdeHashSpace::
OdeHashSpace() :
  OdeSpace(dHashSpaceCreate(0)) {
}

OdeHashSpace::
OdeHashSpace(OdeSpace &space) :
  OdeSpace(dHashSpaceCreate(space.get_id())) {
}

OdeHashSpace::
~OdeHashSpace() {
}

