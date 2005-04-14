// Filename: collisionInvSphere.cxx
// Created by:  drose (05Jan05)
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

#include "collisionInvSphere.h"
#include "collisionSphere.h"
#include "collisionLine.h"
#include "collisionRay.h"
#include "collisionSegment.h"
#include "collisionHandler.h"
#include "collisionEntry.h"
#include "config_collide.h"
#include "omniBoundingVolume.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "geomTristrip.h"
#include "nearly_zero.h"
#include "qpgeom.h"
#include "qpgeomTristrips.h"
#include "qpgeomVertexWriter.h"

TypeHandle CollisionInvSphere::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid *CollisionInvSphere::
make_copy() {
  return new CollisionInvSphere(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::test_intersection
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionInvSphere::
test_intersection(const CollisionEntry &) const {
  report_undefined_from_intersection(get_type());
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionInvSphere::
output(ostream &out) const {
  out << "invsphere, c (" << get_center() << "), r " << get_radius();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::recompute_bound
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
BoundingVolume *CollisionInvSphere::
recompute_bound() {
  BoundedObject::recompute_bound();
  // An inverse sphere always has an infinite bounding volume, since
  // everything outside the sphere is solid matter.
  return set_bound_ptr(new OmniBoundingVolume());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::test_intersection_from_sphere
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionInvSphere::
test_intersection_from_sphere(const CollisionEntry &entry) const {
  const CollisionSphere *sphere;
  DCAST_INTO_R(sphere, entry.get_from(), 0);

  const LMatrix4f &wrt_mat = entry.get_wrt_mat();

  LPoint3f from_center = sphere->get_center() * wrt_mat;
  LVector3f from_radius_v =
    LVector3f(sphere->get_radius(), 0.0f, 0.0f) * wrt_mat;
  float from_radius = length(from_radius_v);

  LPoint3f into_center = get_center();
  float into_radius = get_radius();

  LVector3f vec = from_center - into_center;
  float dist2 = dot(vec, vec);
  if (dist2 < (into_radius - from_radius) * (into_radius - from_radius)) {
    // No intersection--the sphere is within the hollow.
    return NULL;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LVector3f surface_normal;
  float vec_length = vec.length();
  if (IS_NEARLY_ZERO(vec_length)) {
    // If we don't have a collision normal (e.g. the centers are
    // exactly coincident), then make up an arbitrary normal--any one
    // is as good as any other.
    surface_normal.set(1.0, 0.0, 0.0);
  } else {
    surface_normal = -vec / vec_length;
  }

  LVector3f normal = (has_effective_normal() && sphere->get_respect_effective_normal()) ? get_effective_normal() : surface_normal;

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(into_center - surface_normal * into_radius);
  new_entry->set_interior_point(from_center - surface_normal * from_radius);

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::test_intersection_from_line
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionInvSphere::
test_intersection_from_line(const CollisionEntry &entry) const {
  const CollisionLine *line;
  DCAST_INTO_R(line, entry.get_from(), 0);

  const LMatrix4f &wrt_mat = entry.get_wrt_mat();

  LPoint3f from_origin = line->get_origin() * wrt_mat;
  LVector3f from_direction = line->get_direction() * wrt_mat;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction)) {
    // The line is in the middle of space, and therefore intersects
    // the sphere.
    t1 = t2 = 0.0;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3f into_intersection_point = from_origin + t2 * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && line->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3f normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(-normal);
  }

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::test_intersection_from_ray
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionInvSphere::
test_intersection_from_ray(const CollisionEntry &entry) const {
  const CollisionRay *ray;
  DCAST_INTO_R(ray, entry.get_from(), 0);

  const LMatrix4f &wrt_mat = entry.get_wrt_mat();

  LPoint3f from_origin = ray->get_origin() * wrt_mat;
  LVector3f from_direction = ray->get_direction() * wrt_mat;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction)) {
    // The ray is in the middle of space, and therefore intersects
    // the sphere.
    t1 = t2 = 0.0;
  }

  t2 = max(t2, 0.0);

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3f into_intersection_point;
  into_intersection_point = from_origin + t2 * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && ray->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3f normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(-normal);
  }

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::test_intersection_from_segment
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionInvSphere::
test_intersection_from_segment(const CollisionEntry &entry) const {
  const CollisionSegment *segment;
  DCAST_INTO_R(segment, entry.get_from(), 0);

  const LMatrix4f &wrt_mat = entry.get_wrt_mat();

  LPoint3f from_a = segment->get_point_a() * wrt_mat;
  LPoint3f from_b = segment->get_point_b() * wrt_mat;
  LVector3f from_direction = from_b - from_a;

  double t1, t2;
  if (!intersects_line(t1, t2, from_a, from_direction)) {
    // The segment is in the middle of space, and therefore intersects
    // the sphere.
    t1 = t2 = 0.0;
  }
  
  double t;
  if (t2 <= 0.0) {
    // The segment is completely below the shell.
    t = 0.0;

  } else if (t1 >= 1.0) {
    // The segment is completely above the shell.
    t = 1.0;

  } else if (t2 <= 1.0) {
    // The bottom edge of the segment intersects the shell.
    t = min(t2, 1.0);

  } else if (t1 >= 0.0) {
    // The top edge of the segment intersects the shell.
    t = max(t1, 0.0);

  } else {
    // Neither edge of the segment intersects the shell.  It follows
    // that both intersection points are within the hollow center of
    // the sphere; therefore, there is no intersection.
    return NULL;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3f into_intersection_point = from_a + t * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && segment->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3f normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(-normal);
  }

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::fill_viz_geom
//       Access: Protected, Virtual
//  Description: Fills the _viz_geom GeomNode up with Geoms suitable
//               for rendering this solid.
////////////////////////////////////////////////////////////////////
void CollisionInvSphere::
fill_viz_geom() {
  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "Recomputing viz for " << *this << "\n";
  }

  static const int num_slices = 16;
  static const int num_stacks = 8;

  if (use_qpgeom) {
    PT(qpGeomVertexData) vdata = new qpGeomVertexData
      ("collision", qpGeomVertexFormat::get_v3(),
       qpGeom::UH_static);
    qpGeomVertexWriter vertex(vdata, InternalName::get_vertex());
    
    PT(qpGeomTristrips) strip = new qpGeomTristrips(qpGeom::UH_static);
    for (int sl = 0; sl < num_slices; ++sl) {
      float longitude0 = (float)sl / (float)num_slices;
      float longitude1 = (float)(sl + 1) / (float)num_slices;
      vertex.add_data3f(compute_point(0.0, longitude0));
      for (int st = 1; st < num_stacks; ++st) {
        float latitude = (float)st / (float)num_stacks;
        vertex.add_data3f(compute_point(latitude, longitude1));
        vertex.add_data3f(compute_point(latitude, longitude0));
      }
      vertex.add_data3f(compute_point(1.0, longitude0));

      strip->add_next_vertices(num_stacks * 2);
      strip->close_primitive();
    }

    PT(qpGeom) geom = new qpGeom;
    geom->set_vertex_data(vdata);
    geom->add_primitive(strip);

    _viz_geom->add_geom(geom, get_solid_viz_state());

  } else {
    GeomTristrip *sphere = new GeomTristrip;
    PTA_Vertexf verts;
    PTA_int lengths;
    verts.reserve((num_stacks * 2) * num_slices);
    lengths.reserve(num_slices);
    
    for (int sl = 0; sl < num_slices; sl++) {
      float longitude0 = (float)sl / (float)num_slices;
      float longitude1 = (float)(sl + 1) / (float)num_slices;
      verts.push_back(compute_point(0.0, longitude0));
      for (int st = 1; st < num_stacks; st++) {
        float latitude = (float)st / (float)num_stacks;
        verts.push_back(compute_point(latitude, longitude1));
        verts.push_back(compute_point(latitude, longitude0));
      }
      verts.push_back(compute_point(1.0, longitude0));
      
      lengths.push_back(num_stacks * 2);
    }
    
    sphere->set_coords(verts);
    sphere->set_lengths(lengths);
    sphere->set_num_prims(num_slices);

    _viz_geom->add_geom(sphere, get_solid_viz_state());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a CollisionInvSphere object
////////////////////////////////////////////////////////////////////
void CollisionInvSphere::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_CollisionInvSphere);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void CollisionInvSphere::
write_datagram(BamWriter *manager, Datagram &me) {
  CollisionSphere::write_datagram(manager, me);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::make_CollisionInvSphere
//       Access: Protected
//  Description: Factory method to generate a CollisionInvSphere object
////////////////////////////////////////////////////////////////////
TypedWritable *CollisionInvSphere::
make_CollisionInvSphere(const FactoryParams &params) {
  CollisionInvSphere *me = new CollisionInvSphere;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void CollisionInvSphere::
fillin(DatagramIterator& scan, BamReader* manager) {
  CollisionSphere::fillin(scan, manager);
}

