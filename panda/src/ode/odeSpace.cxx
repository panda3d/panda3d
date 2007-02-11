// Filename: odeSpace.cxx
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
#include "odeSpace.h"

TypeHandle OdeSpace::_type_handle;

OdeSpace::
OdeSpace(dSpaceID id) : 
  _id(id) {
}

OdeSpace::
~OdeSpace() {
}

void OdeSpace::
destroy() {
  dSpaceDestroy(_id);
}

int OdeSpace::
query(const OdeGeom& geom) const {
  return dSpaceQuery(_id, geom.get_id());
}

int OdeSpace::
query(const OdeSpace& space) const {
  return dSpaceQuery(_id, (dGeomID)space.get_id());
}

void OdeSpace::
add(OdeSpace& space) {
  dSpaceAdd(_id, (dGeomID)space.get_id());
}

void OdeSpace::
remove(OdeSpace& space) {
  dSpaceRemove(_id, (dGeomID)space.get_id());
}

void OdeSpace::
add(OdeGeom& geom) {
  dSpaceAdd(_id, geom.get_id());
}

void OdeSpace::
remove(OdeGeom& geom) {
  dSpaceRemove(_id, geom.get_id());
}

void OdeSpace::
clean() {
  dSpaceClean(_id);
}

OdeGeom OdeSpace::
get_geom(int i) {
  return OdeGeom(dSpaceGetGeom(_id, i));
}

void OdeSpace::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out << "" << get_type() << "(id = " << _id << ")";
  #endif //] NDEBUG
}

OdeSimpleSpace OdeSpace::
convert_to_simple_space() const {
  nassertr(_id != 0, OdeSimpleSpace((dSpaceID)0));
  nassertr(get_class() == OdeGeom::GC_simple_space, OdeSimpleSpace((dSpaceID)0));
  return OdeSimpleSpace(_id);
}

OdeHashSpace OdeSpace::
convert_to_hash_space() const {
  nassertr(_id != 0, OdeHashSpace((dSpaceID)0));
  nassertr(get_class() == OdeGeom::GC_hash_space, OdeHashSpace((dSpaceID)0));
  return OdeHashSpace(_id);
}

OdeQuadTreeSpace OdeSpace::
convert_to_quad_tree_space() const {
  nassertr(_id != 0, OdeQuadTreeSpace((dSpaceID)0));
  nassertr(get_class() == OdeGeom::GC_quad_tree_space, OdeQuadTreeSpace((dSpaceID)0));
  return OdeQuadTreeSpace(_id);
}

