// Filename: odeGeom.cxx
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
#include "odeGeom.h"

TypeHandle OdeGeom::_type_handle;

OdeGeom::
OdeGeom(dGeomID id) :
  _id(id) {
  odegeom_cat.debug() << get_type() << "(" << _id << ")\n";
}

OdeGeom::
~OdeGeom() {
}

void OdeGeom::
destroy() {
  if (get_class() == OdeTriMeshGeom::get_geom_class()) {
    OdeTriMeshGeom::unlink_data(_id);
  }
}

void OdeGeom::
get_space(OdeSpace &space) const {
  space._id = dGeomGetSpace(_id);  
}


void OdeGeom::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); 
  out << get_type() << "(id = " << _id << ")";
  #endif //] NDEBUG
}

