// Filename: odeContactGeom.cxx
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
#include "odeContactGeom.h"

TypeHandle OdeContactGeom::_type_handle;

OdeContactGeom::
OdeContactGeom() : 
  _contact_geom() {
}

OdeContactGeom::
OdeContactGeom(const OdeContactGeom &copy) : 
  _contact_geom() {
  *this = copy._contact_geom;
}

OdeContactGeom::
OdeContactGeom(const dContactGeom &copy) : 
  _contact_geom() {
  *this = copy;
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
  *this = copy._contact_geom;
}

void OdeContactGeom::
operator = (const dContactGeom &contact_geom) {
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

