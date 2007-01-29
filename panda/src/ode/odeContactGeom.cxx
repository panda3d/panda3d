// Filename: odeContactGeom.cxx
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
#include "odeContactGeom.h"

TypeHandle OdeContactGeom::_type_handle;

OdeContactGeom::
OdeContactGeom() : 
  _contact_geom() {
}

OdeContactGeom::
OdeContactGeom(const OdeContactGeom &copy) : 
  _contact_geom() {
  _contact_geom.pos[0] = copy._contact_geom.pos[0];
  _contact_geom.pos[1] = copy._contact_geom.pos[1];
  _contact_geom.pos[2] = copy._contact_geom.pos[2];
  _contact_geom.normal[0] = copy._contact_geom.normal[0];
  _contact_geom.normal[1] = copy._contact_geom.normal[1];
  _contact_geom.normal[2] = copy._contact_geom.normal[2];
  _contact_geom.depth = copy._contact_geom.depth;
  _contact_geom.g1 = copy._contact_geom.g1;
  _contact_geom.g2 = copy._contact_geom.g2;
  _contact_geom.side1 = copy._contact_geom.side1;
  _contact_geom.side2 = copy._contact_geom.side2;
}

OdeContactGeom::
OdeContactGeom(const dContactGeom &contact_geom) : 
  _contact_geom() {
  _contact_geom.pos[0] = contact_geom.pos[0];
  _contact_geom.pos[1] = contact_geom.pos[1];
  _contact_geom.pos[2] = contact_geom.pos[2];
  _contact_geom.normal[0] = contact_geom.normal[0];
  _contact_geom.normal[1] = contact_geom.normal[1];
  _contact_geom.normal[2] = contact_geom.normal[2];
  _contact_geom.depth = contact_geom.depth;
  _contact_geom.g1 = contact_geom.g1;
  _contact_geom.g2 = contact_geom.g2;
  _contact_geom.side1 = contact_geom.side1;
  _contact_geom.side2 = contact_geom.side2;
}

OdeContactGeom::
~OdeContactGeom() {
}

const dContactGeom* OdeContactGeom::
get_contact_geom_ptr() const {
  return &_contact_geom;
}

void OdeContactGeom::
operator = (const OdeContactGeom &copy) {
  _contact_geom.pos[0] = copy._contact_geom.pos[0];
  _contact_geom.pos[1] = copy._contact_geom.pos[1];
  _contact_geom.pos[2] = copy._contact_geom.pos[2];
  _contact_geom.normal[0] = copy._contact_geom.normal[0];
  _contact_geom.normal[1] = copy._contact_geom.normal[1];
  _contact_geom.normal[2] = copy._contact_geom.normal[2];
  _contact_geom.depth = copy._contact_geom.depth;
  _contact_geom.g1 = copy._contact_geom.g1;
  _contact_geom.g2 = copy._contact_geom.g2;
  _contact_geom.side1 = copy._contact_geom.side1;
  _contact_geom.side2 = copy._contact_geom.side2;
}

