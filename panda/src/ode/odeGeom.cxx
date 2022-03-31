/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeGeom.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeGeom.h"
#include "odeSimpleSpace.h"
#include "odeQuadTreeSpace.h"
#include "odeHashSpace.h"

#include "odeTriMeshGeom.h"
#include "odeTriMeshData.h"
#include "odeBoxGeom.h"
#include "odeCappedCylinderGeom.h"
#include "odeCylinderGeom.h"
#include "odePlaneGeom.h"
#include "odeRayGeom.h"
#include "odeSphereGeom.h"

// OdeGeom::GeomSurfaceMap OdeGeom::_geom_surface_map;
// OdeGeom::GeomCollideIdMap OdeGeom::_geom_collide_id_map;
TypeHandle OdeGeom::_type_handle;

OdeGeom::
OdeGeom(dGeomID id) :
  _id(id) {
  if (odegeom_cat.is_debug()) {
    odegeom_cat.debug() << get_type() << "(" << _id << ")\n";
  }
}

OdeGeom::
~OdeGeom() {
  if (odegeom_cat.is_debug()) {
    odegeom_cat.debug() << "~" << get_type() << "(" << _id << ")\n";
  }
  /*
  GeomSurfaceMap::iterator iter = _geom_surface_map.find(this->get_id());
  if (iter != _geom_surface_map.end()) {
    _geom_surface_map.erase(iter);
  }

  GeomCollideIdMap::iterator iter2 = _geom_collide_id_map.find(this->get_id());
  if (iter2 != _geom_collide_id_map.end()) {
    _geom_collide_id_map.erase(iter2);
  }
  */
}

/*
int OdeGeom::
get_surface_type()
{
  return get_space().get_surface_type(this->get_id());
}

int OdeGeom::
get_collide_id()
{
  return get_space().get_collide_id(this->get_id());
}

void OdeGeom::
set_surface_type(int surface_type)
{
    get_space().set_surface_type(surface_type, this->get_id());
}

int OdeGeom::
set_collide_id(int collide_id)
{
    return get_space().set_collide_id(collide_id, this->get_id());
}


int OdeGeom::
test_collide_id(int collide_id)
{

    odegeom_cat.debug() << "test_collide_id start" << "\n";
    int first = get_space().set_collide_id(collide_id, this->get_id());
    odegeom_cat.debug() << "returns" << first << "\n";
    odegeom_cat.debug() << "test_collide_id middle" << "\n";
    int test = get_space().get_collide_id(this->get_id());
    odegeom_cat.debug() << "test_collide_id stop" << "\n";
    return test;
}
*/

void OdeGeom::
destroy() {
  if (get_class() == OdeTriMeshGeom::get_geom_class()) {
    OdeTriMeshData::unlink_data(_id);
  }
  dGeomDestroy(_id);
}

OdeSpace OdeGeom::
get_space() const {
  return OdeSpace(dGeomGetSpace(_id));
}


void OdeGeom::
write(std::ostream &out, unsigned int indent) const {
  out.width(indent);
  out << get_type() << "(id = " << _id << ")";
}

OdeBoxGeom OdeGeom::
convert_to_box() const {
  nassertr(_id != nullptr, OdeBoxGeom(nullptr));
  nassertr(get_class() == GC_box, OdeBoxGeom(nullptr));
  return OdeBoxGeom(_id);
}

OdeCappedCylinderGeom OdeGeom::
convert_to_capped_cylinder() const {
  nassertr(_id != nullptr, OdeCappedCylinderGeom(nullptr));
  nassertr(get_class() == GC_capped_cylinder, OdeCappedCylinderGeom(nullptr));
  return OdeCappedCylinderGeom(_id);
}

/*
OdeConvexGeom OdeGeom::
convert_to_convex() const {
  nassertr(_id != nullptr, OdeConvexGeom(nullptr));
  nassertr(get_class() == GC_convex, OdeConvexGeom(nullptr));
  return OdeConvexGeom(_id);
}
*/

OdeCylinderGeom OdeGeom::
convert_to_cylinder() const {
  nassertr(_id != nullptr, OdeCylinderGeom(nullptr));
  nassertr(get_class() == GC_cylinder, OdeCylinderGeom(nullptr));
  return OdeCylinderGeom(_id);
}

/*
OdeHeightfieldGeom OdeGeom::
convert_to_heightfield() const {
  nassertr(_id != nullptr, OdeHeightfieldGeom(nullptr));
  nassertr(get_class() == GC_heightfield, OdeHeightfieldGeom(nullptr));
  return OdeHeightfieldGeom(_id);
}
*/

OdePlaneGeom OdeGeom::
convert_to_plane() const {
  nassertr(_id != nullptr, OdePlaneGeom(nullptr));
  nassertr(get_class() == GC_plane, OdePlaneGeom(nullptr));
  return OdePlaneGeom(_id);
}

OdeRayGeom OdeGeom::
convert_to_ray() const {
  nassertr(_id != nullptr, OdeRayGeom(nullptr));
  nassertr(get_class() == GC_ray, OdeRayGeom(nullptr));
  return OdeRayGeom(_id);
}

OdeSphereGeom OdeGeom::
convert_to_sphere() const {
  nassertr(_id != nullptr, OdeSphereGeom(nullptr));
  nassertr(get_class() == GC_sphere, OdeSphereGeom(nullptr));
  return OdeSphereGeom(_id);
}

OdeTriMeshGeom OdeGeom::
convert_to_tri_mesh() const {
  nassertr(_id != nullptr, OdeTriMeshGeom(nullptr));
  nassertr(get_class() == GC_tri_mesh, OdeTriMeshGeom(nullptr));
  return OdeTriMeshGeom(_id);
}

OdeSimpleSpace OdeGeom::
convert_to_simple_space() const {
  nassertr(_id != nullptr, OdeSimpleSpace(nullptr));
  nassertr(get_class() == GC_simple_space, OdeSimpleSpace(nullptr));
  return OdeSimpleSpace((dSpaceID)_id);
}

OdeHashSpace OdeGeom::
convert_to_hash_space() const {
  nassertr(_id != nullptr, OdeHashSpace(nullptr));
  nassertr(get_class() == GC_hash_space, OdeHashSpace(nullptr));
  return OdeHashSpace((dSpaceID)_id);
}

OdeQuadTreeSpace OdeGeom::
convert_to_quad_tree_space() const {
  nassertr(_id != nullptr, OdeQuadTreeSpace(nullptr));
  nassertr(get_class() == GC_quad_tree_space, OdeQuadTreeSpace(nullptr));
  return OdeQuadTreeSpace((dSpaceID)_id);
}

OdeGeom::
operator bool () const {
  return (_id != nullptr);
}
