// Filename: odeBoxGeom.cxx
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
#include "odeBoxGeom.h"

TypeHandle OdeBoxGeom::_type_handle;

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
