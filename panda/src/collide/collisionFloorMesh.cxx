// Filename: collisionPlane.cxx
// Created by:  drose (25Apr00)
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


#include "collisionFloorMesh.h"
#include "collisionHandler.h"
#include "collisionEntry.h"
#include "collisionSphere.h"
#include "collisionLine.h"
#include "collisionRay.h"
#include "collisionSegment.h"
#include "config_collide.h"
#include "pointerToArray.h"
#include "geomNode.h"
#include "geom.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "boundingPlane.h"
#include "geom.h"
#include "geomTrifans.h"
#include "geomLinestrips.h"
#include "geomVertexWriter.h"
#include <algorithm>
PStatCollector CollisionFloorMesh::_volume_pcollector("Collision Volumes:CollisionFloorMesh");
PStatCollector CollisionFloorMesh::_test_pcollector("Collision Tests:CollisionFloorMesh");
TypeHandle CollisionFloorMesh::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid *CollisionFloorMesh::
make_copy() {
  return new CollisionFloorMesh(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::xform
//       Access: Public, Virtual
//  Description: Transforms the solid by the indicated matrix.
////////////////////////////////////////////////////////////////////
void CollisionFloorMesh::
xform(const LMatrix4f &mat) {
  CollisionSolid::xform(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::get_collision_origin
//       Access: Public, Virtual
//  Description: Returns the point in space deemed to be the "origin"
//               of the solid for collision purposes.  The closest
//               intersection point to this origin point is considered
//               to be the most significant.
////////////////////////////////////////////////////////////////////
LPoint3f CollisionFloorMesh::
get_collision_origin() const {
  // No real sensible origin exists for a plane.  We return 0, 0, 0,
  // without even bothering to ensure that that point exists on the
  // plane.
  return LPoint3f::origin();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPlane::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionFloorMesh::
output(ostream &out) const {
  out << "cfloor";
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionFloorMesh::compute_internal_bounds
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(BoundingVolume) CollisionFloorMesh::
compute_internal_bounds() const {
  return new BoundingBox(LPoint3f(0,0,0), LPoint3f(1,1,1));
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionFloorMesh::test_intersection_from_ray
//       Access: Public, Virtual
//  Description: must be a vertical Ray!!!
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionFloorMesh::
test_intersection_from_ray(const CollisionEntry &entry) const {
  const CollisionRay *ray;
  DCAST_INTO_R(ray, entry.get_from(), 0);
  const LMatrix4f &wrt_mat = entry.get_wrt_mat();
  LPoint3f from_origin = ray->get_origin() * wrt_mat;
  LVector3f from_direction = ray->get_direction() * wrt_mat;
  
  double fx=from_origin[0];
  double fy=from_origin[1];

  CollisionFloorMesh::Triangles::const_iterator ti;
  bool collided = false;
  for (ti=_triangles.begin();ti< _triangles.end();++ti) {
    TriangleIndices tri = *ti;
    LPoint3d p0=_vertices[tri.p1];
    LPoint3d p1=_vertices[tri.p2];
    LPoint3d p2=_vertices[tri.p3];
    double p0x = p0[0];
    double p1x = p1[0];
    double p2x = p2[0];

    double p0y = p0[1];
    double p1y = p1[1];
    double p2y = p2[1];
    
    double e0x,e0y,e1x,e1y,e2x,e2y;
    double u,v;

    e0x = fx - p0x; e0y = fy - p0y;
    e1x = p1x - p0x; e1y = p1y - p0y;
    e2x = p2x - p0x; e2y = p2y - p0y;
    if (e1x==0) {  
      if (e2x == 0) continue; 
      u = e0x/e2x;
      if (u<0 || u>1) continue;     
      if (e1y == 0) continue;
      v = ( e0y - (e2y*u))/e1y;
      if (v<0) continue; 
    } else {
      double d = (e2y * e1x)-(e2x * e1y);
      if (d==0) continue; 
      u = ((e0y * e1x) - (e0x * e1y))/d;
      if (u<0 || u>1) continue;
      v = (e0x - (e2x * u)) / e1x;
      if (v<0) continue;
      if (u + v > 1) continue; 
    }
    //we collided!!
    double mag = u + v;
    double p0z = p0[2];
    double p1z = p1[2];
    double p2z = p2[2];
    

    double uz = (p2z - p0z) *  mag;
    double vz = (p1z - p0z) *  mag;
    double ratio = u/(u+v);
    double finalz = vz+((uz - vz) * ratio);
    PT(CollisionEntry) new_entry = new CollisionEntry(entry);    
    
    new_entry->set_surface_normal(LPoint3f(0,0,1));
    new_entry->set_surface_point(LPoint3f(fx,fy,finalz));
    return new_entry;
  }
  return NULL;
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionFloorMesh::fill_viz_geom
//       Access: Protected, Virtual
//  Description: Fills the _viz_geom GeomNode up with Geoms suitable
//               for rendering this solid.
////////////////////////////////////////////////////////////////////
void CollisionFloorMesh::
fill_viz_geom() {
  
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionFloorMesh::get_volume_pcollector
//       Access: Public, Virtual
//  Description: Returns a PStatCollector that is used to count the
//               number of bounding volume tests made against a solid
//               of this type in a given frame.
////////////////////////////////////////////////////////////////////
PStatCollector &CollisionFloorMesh::
get_volume_pcollector() {
  return _volume_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionFloorMesh::get_test_pcollector
//       Access: Public, Virtual
//  Description: Returns a PStatCollector that is used to count the
//               number of intersection tests made against a solid
//               of this type in a given frame.
////////////////////////////////////////////////////////////////////
PStatCollector &CollisionFloorMesh::
get_test_pcollector() {
  return _test_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionFloorMesh::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void CollisionFloorMesh::
write_datagram(BamWriter *manager, Datagram &me)
{
  CollisionSolid::write_datagram(manager, me);
  me.add_uint16(_vertices.size());
  for (size_t i = 0; i < _vertices.size(); i++) {
    _vertices[i].write_datagram(me);
  }
  me.add_uint16(_triangles.size());
  for (size_t i = 0; i < _triangles.size(); i++) {
    me.add_uint16(_triangles[i].p1);
    me.add_uint16(_triangles[i].p2);
    me.add_uint16(_triangles[i].p3);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionFloorMesh::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void CollisionFloorMesh::
fillin(DatagramIterator& scan, BamReader* manager)
{
  CollisionSolid::fillin(scan, manager);

  unsigned int num_verts = scan.get_uint16();
  for (size_t i = 0; i < num_verts; i++) {
    LPoint3d vert;
    vert.read_datagram(scan);

    _vertices.push_back(vert);
  }
  unsigned int num_tris = scan.get_uint16();
  for (size_t i = 0; i < num_tris; i++) {
    CollisionFloorMesh::TriangleIndices tri;

    tri.p1 = scan.get_uint32();
    tri.p2 = scan.get_uint32();
    tri.p3 = scan.get_uint32();
    
    _triangles.push_back(tri);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::make_CollisionPolygon
//       Access: Protected
//  Description: Factory method to generate a CollisionPolygon object
////////////////////////////////////////////////////////////////////
TypedWritable* CollisionFloorMesh::
make_CollisionFloorMesh(const FactoryParams &params) {
  CollisionFloorMesh *me = new CollisionFloorMesh;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a CollisionPolygon object
////////////////////////////////////////////////////////////////////
void CollisionFloorMesh::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_CollisionFloorMesh);
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionFloorMesh::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionFloorMesh::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << (*this) << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionFloorMesh::add_triangle
//       Access: Published
//  Description: store a triangle for processing
////////////////////////////////////////////////////////////////////
void CollisionFloorMesh::
add_triangle(unsigned int pointA, unsigned int pointB, unsigned int pointC) {
  CollisionFloorMesh::TriangleIndices  tri;
  tri.p1 = pointA;
  tri.p2 = pointB;
  tri.p3 = pointC;
  _triangles.push_back(tri);
}
