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
  odegeom_cat.debug() << "~" << get_type() << "(" << _id << ")\n";
}

void OdeGeom::
destroy() {
  if (get_class() == OdeTriMeshGeom::get_geom_class()) {
    OdeTriMeshData::unlink_data(_id);
  }
}

OdeSpace OdeGeom::
get_space() const {
  return OdeSpace(dGeomGetSpace(_id));
}


void OdeGeom::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); 
  out << get_type() << "(id = " << _id << ")";
  #endif //] NDEBUG
}

OdeBoxGeom OdeGeom::
convert_to_box() const {
  nassertr(_id != 0, OdeBoxGeom((dGeomID)0));
  nassertr(get_class() == GC_box, OdeBoxGeom((dGeomID)0));
  return OdeBoxGeom(_id);
}

OdeCappedCylinderGeom OdeGeom::
convert_to_capped_cylinder() const {
  nassertr(_id != 0, OdeCappedCylinderGeom((dGeomID)0));
  nassertr(get_class() == GC_capped_cylinder, OdeCappedCylinderGeom((dGeomID)0));
  return OdeCappedCylinderGeom(_id);
}

/*
OdeConvexGeom OdeGeom::
convert_to_convex() const {
  nassertr(_id != 0, OdeConvexGeom((dGeomID)0));
  nassertr(get_class() == GC_convex, OdeConvexGeom((dGeomID)0));
  return OdeConvexGeom(_id);
}
*/

OdeCylinderGeom OdeGeom::
convert_to_cylinder() const {
  nassertr(_id != 0, OdeCylinderGeom((dGeomID)0));
  nassertr(get_class() == GC_cylinder, OdeCylinderGeom((dGeomID)0));
  return OdeCylinderGeom(_id);
}

/*
OdeHeightfieldGeom OdeGeom::
convert_to_heightfield() const {
  nassertr(_id != 0, OdeHeightfieldGeom((dGeomID)0));
  nassertr(get_class() == GC_heightfield, OdeHeightfieldGeom((dGeomID)0));
  return OdeHeightfieldGeom(_id);
}
*/

OdePlaneGeom OdeGeom::
convert_to_plane() const {
  nassertr(_id != 0, OdePlaneGeom((dGeomID)0));
  nassertr(get_class() == GC_plane, OdePlaneGeom((dGeomID)0));
  return OdePlaneGeom(_id);
}

OdeRayGeom OdeGeom::
convert_to_ray() const {
  nassertr(_id != 0, OdeRayGeom((dGeomID)0));
  nassertr(get_class() == GC_ray, OdeRayGeom((dGeomID)0));
  return OdeRayGeom(_id);
}

OdeSphereGeom OdeGeom::
convert_to_sphere() const {
  nassertr(_id != 0, OdeSphereGeom((dGeomID)0));
  nassertr(get_class() == GC_sphere, OdeSphereGeom((dGeomID)0));
  return OdeSphereGeom(_id);
}

OdeTriMeshGeom OdeGeom::
convert_to_tri_mesh() const {
  nassertr(_id != 0, OdeTriMeshGeom((dGeomID)0));
  nassertr(get_class() == GC_tri_mesh, OdeTriMeshGeom((dGeomID)0));
  return OdeTriMeshGeom(_id);
}

OdeSimpleSpace OdeGeom::
convert_to_simple_space() const {
  nassertr(_id != 0, OdeSimpleSpace((dSpaceID)0));
  nassertr(get_class() == GC_simple_space, OdeSimpleSpace((dSpaceID)0));
  return OdeSimpleSpace((dSpaceID)_id);
}

OdeHashSpace OdeGeom::
convert_to_hash_space() const {
  nassertr(_id != 0, OdeHashSpace((dSpaceID)0));
  nassertr(get_class() == GC_hash_space, OdeHashSpace((dSpaceID)0));
  return OdeHashSpace((dSpaceID)_id);
}

OdeQuadTreeSpace OdeGeom::
convert_to_quad_tree_space() const {
  nassertr(_id != 0, OdeQuadTreeSpace((dSpaceID)0));
  nassertr(get_class() == GC_quad_tree_space, OdeQuadTreeSpace((dSpaceID)0));
  return OdeQuadTreeSpace((dSpaceID)_id);
}

