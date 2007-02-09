// Filename: odePlaneGeom.cxx
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
#include "odePlaneGeom.h"

TypeHandle OdePlaneGeom::_type_handle;

OdePlaneGeom::
OdePlaneGeom(dReal a, dReal b, dReal c, dReal d) :
  OdeGeom(dCreatePlane(0, a, b, c, d)) {
}

OdePlaneGeom::
OdePlaneGeom(const LVecBase4f &params) :
  OdeGeom(dCreatePlane(0, params[0], params[1], params[2], params[3])) {
}

OdePlaneGeom::
OdePlaneGeom(OdeSpace &space, dReal a, dReal b, dReal c, dReal d) :
  OdeGeom(dCreatePlane(space.get_id(), a, b, c, d)) {
}

OdePlaneGeom::
OdePlaneGeom(OdeSpace &space, const LVecBase4f &params) :
  OdeGeom(dCreatePlane(space.get_id(), params[0], params[1], params[2], params[3])) {
}

OdePlaneGeom::
~OdePlaneGeom() {
}
