// Filename: collisionInvSphere.cxx
// Created by:  drose (05Jan05)
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
#include "nearly_zero.h"
#include "geom.h"
#include "geomTristrips.h"
#include "geomVertexWriter.h"

PStatCollector CollisionInvSphere::_volume_pcollector("Collision Volumes:CollisionInvSphere");
PStatCollector CollisionInvSphere::_test_pcollector("Collision Tests:CollisionInvSphere");
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
//     Function: CollisionInvSphere::get_volume_pcollector
//       Access: Public, Virtual
//  Description: Returns a PStatCollector that is used to count the
//               number of bounding volume tests made against a solid
//               of this type in a given frame.
////////////////////////////////////////////////////////////////////
PStatCollector &CollisionInvSphere::
get_volume_pcollector() {
  return _volume_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionInvSphere::get_test_pcollector
//       Access: Public, Virtual
//  Description: Returns a PStatCollector that is used to count the
//               number of intersection tests made against a solid
//               of this type in a given frame.
////////////////////////////////////////////////////////////////////
PStatCollector &CollisionInvSphere::
get_test_pcollector() {
  return _test_pcollector;
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
//     Function: CollisionInvSphere::compute_internal_bounds
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(BoundingVolume) CollisionInvSphere::
compute_internal_bounds() const {
  // An inverse sphere always has an infinite bounding volume, since
  // everything outside the sphere is solid matter.
  return new OmniBoundingVolume();
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

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_center = sphere->get_center() * wrt_mat;
  LVector3 from_radius_v =
    LVector3(sphere->get_radius(), 0.0f, 0.0f) * wrt_mat;
  PN_stdfloat from_radius = length(from_radius_v);

  LPoint3 into_center = get_center();
  PN_stdfloat into_radius = get_radius();

  LVector3 vec = from_center - into_center;
  PN_stdfloat dist2 = dot(vec, vec);
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

  LVector3 surface_normal;
  PN_stdfloat vec_length = vec.length();
  if (IS_NEARLY_ZERO(vec_length)) {
    // If we don't have a collision normal (e.g. the centers are
    // exactly coincident), then make up an arbitrary normal--any one
    // is as good as any other.
    surface_normal.set(1.0, 0.0, 0.0);
  } else {
    surface_normal = vec / -vec_length;
  }

  LVector3 normal = (has_effective_normal() && sphere->get_respect_effective_normal()) ? get_effective_normal() : surface_normal;

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

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_origin = line->get_origin() * wrt_mat;
  LVector3 from_direction = line->get_direction() * wrt_mat;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction, 0.0f)) {
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

  LPoint3 into_intersection_point = from_origin + t2 * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && line->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3 normal = into_intersection_point - get_center();
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

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_origin = ray->get_origin() * wrt_mat;
  LVector3 from_direction = ray->get_direction() * wrt_mat;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction, 0.0f)) {
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

  LPoint3 into_intersection_point;
  into_intersection_point = from_origin + t2 * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && ray->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3 normal = into_intersection_point - get_center();
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

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_a = segment->get_point_a() * wrt_mat;
  LPoint3 from_b = segment->get_point_b() * wrt_mat;
  LVector3 from_direction = from_b - from_a;

  double t1, t2;
  if (!intersects_line(t1, t2, from_a, from_direction, 0.0f)) {
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

  LPoint3 into_intersection_point = from_a + t * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && segment->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3 normal = into_intersection_point - get_center();
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

  PT(GeomVertexData) vdata = new GeomVertexData
    ("collision", GeomVertexFormat::get_v3(),
     Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  
  PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_static);
  for (int sl = 0; sl < num_slices; ++sl) {
    PN_stdfloat longitude0 = (PN_stdfloat)sl / (PN_stdfloat)num_slices;
    PN_stdfloat longitude1 = (PN_stdfloat)(sl + 1) / (PN_stdfloat)num_slices;
    vertex.add_data3(compute_point(0.0, longitude0));
    for (int st = 1; st < num_stacks; ++st) {
      PN_stdfloat latitude = (PN_stdfloat)st / (PN_stdfloat)num_stacks;
      vertex.add_data3(compute_point(latitude, longitude1));
      vertex.add_data3(compute_point(latitude, longitude0));
    }
    vertex.add_data3(compute_point(1.0, longitude0));
    
    strip->add_next_vertices(num_stacks * 2);
    strip->close_primitive();
  }
  
  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(strip);
  
  _viz_geom->add_geom(geom, get_solid_viz_state());
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

