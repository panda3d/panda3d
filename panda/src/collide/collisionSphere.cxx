// Filename: collisionSphere.cxx
// Created by:  drose (24Apr00)
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


#include "collisionSphere.h"
#include "collisionLine.h"
#include "collisionRay.h"
#include "collisionSegment.h"
#include "collisionHandler.h"
#include "collisionEntry.h"
#include "config_collide.h"
#include "boundingSphere.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "geomTristrip.h"
#include "nearly_zero.h"
#include "cmath.h"
#include "mathNumbers.h"
#include "qpgeom.h"
#include "qpgeomTristrips.h"
#include "qpgeomVertexWriter.h"

TypeHandle CollisionSphere::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid *CollisionSphere::
make_copy() {
  return new CollisionSphere(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::test_intersection
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSphere::
test_intersection(const CollisionEntry &entry) const {
  return entry.get_into()->test_intersection_from_sphere(entry);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::xform
//       Access: Public, Virtual
//  Description: Transforms the solid by the indicated matrix.
////////////////////////////////////////////////////////////////////
void CollisionSphere::
xform(const LMatrix4f &mat) {
  _center = _center * mat;

  // This is a little cheesy and fails miserably in the presence of a
  // non-uniform scale.
  LVector3f radius_v = LVector3f(_radius, 0.0f, 0.0f) * mat;
  _radius = length(radius_v);
  mark_viz_stale();
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::get_collision_origin
//       Access: Public, Virtual
//  Description: Returns the point in space deemed to be the "origin"
//               of the solid for collision purposes.  The closest
//               intersection point to this origin point is considered
//               to be the most significant.
////////////////////////////////////////////////////////////////////
LPoint3f CollisionSphere::
get_collision_origin() const {
  return get_center();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionSphere::
output(ostream &out) const {
  out << "sphere, c (" << get_center() << "), r " << get_radius();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::recompute_bound
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
BoundingVolume *CollisionSphere::
recompute_bound() {
  BoundingVolume *bound = BoundedObject::recompute_bound();
  nassertr(bound != (BoundingVolume*)0L, bound);
  nassertr(!_center.is_nan() && !cnan(_radius), bound);
  BoundingSphere sphere(_center, _radius);
  bound->extend_by(&sphere);

  return bound;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::test_intersection_from_sphere
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSphere::
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
  if (dist2 > (into_radius + from_radius) * (into_radius + from_radius)) {
    // No intersection.
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
    surface_normal = vec / vec_length;
  }

  LVector3f normal = (has_effective_normal() && sphere->get_respect_effective_normal()) ? get_effective_normal() : surface_normal;

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(into_center + surface_normal * into_radius);
  new_entry->set_interior_point(from_center - surface_normal * from_radius);

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::test_intersection_from_line
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSphere::
test_intersection_from_line(const CollisionEntry &entry) const {
  const CollisionLine *line;
  DCAST_INTO_R(line, entry.get_from(), 0);

  const LMatrix4f &wrt_mat = entry.get_wrt_mat();

  LPoint3f from_origin = line->get_origin() * wrt_mat;
  LVector3f from_direction = line->get_direction() * wrt_mat;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction)) {
    // No intersection.
    return NULL;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3f into_intersection_point = from_origin + t1 * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && line->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3f normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::test_intersection_from_ray
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSphere::
test_intersection_from_ray(const CollisionEntry &entry) const {
  const CollisionRay *ray;
  DCAST_INTO_R(ray, entry.get_from(), 0);

  const LMatrix4f &wrt_mat = entry.get_wrt_mat();

  LPoint3f from_origin = ray->get_origin() * wrt_mat;
  LVector3f from_direction = ray->get_direction() * wrt_mat;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction)) {
    // No intersection.
    return NULL;
  }

  if (t2 < 0.0) {
    // Both intersection points are before the start of the ray.
    return NULL;
  }

  t1 = max(t1, 0.0);

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3f into_intersection_point = from_origin + t1 * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && ray->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3f normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::test_intersection_from_segment
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSphere::
test_intersection_from_segment(const CollisionEntry &entry) const {
  const CollisionSegment *segment;
  DCAST_INTO_R(segment, entry.get_from(), 0);

  const LMatrix4f &wrt_mat = entry.get_wrt_mat();

  LPoint3f from_a = segment->get_point_a() * wrt_mat;
  LPoint3f from_b = segment->get_point_b() * wrt_mat;
  LVector3f from_direction = from_b - from_a;

  double t1, t2;
  if (!intersects_line(t1, t2, from_a, from_direction)) {
    // No intersection.
    return NULL;
  }

  if (t2 < 0.0 || t1 > 1.0) {
    // Both intersection points are before the start of the segment or
    // after the end of the segment.
    return NULL;
  }

  t1 = max(t1, 0.0);

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3f into_intersection_point = from_a + t1 * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && segment->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3f normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::fill_viz_geom
//       Access: Protected, Virtual
//  Description: Fills the _viz_geom GeomNode up with Geoms suitable
//               for rendering this solid.
////////////////////////////////////////////////////////////////////
void CollisionSphere::
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
       qpGeomUsageHint::UH_static);
    qpGeomVertexWriter vertex(vdata, InternalName::get_vertex());
    
    PT(qpGeomTristrips) strip = new qpGeomTristrips(qpGeomUsageHint::UH_static);
    for (int sl = 0; sl < num_slices; ++sl) {
      float longitude0 = (float)sl / (float)num_slices;
      float longitude1 = (float)(sl + 1) / (float)num_slices;
      vertex.add_data3f(compute_point(0.0, longitude0));
      for (int st = 1; st < num_stacks; ++st) {
        float latitude = (float)st / (float)num_stacks;
        vertex.add_data3f(compute_point(latitude, longitude0));
        vertex.add_data3f(compute_point(latitude, longitude1));
      }
      vertex.add_data3f(compute_point(1.0, longitude0));

      strip->add_next_vertices(num_stacks * 2);
      strip->close_primitive();
    }

    PT(qpGeom) geom = new qpGeom;
    geom->set_vertex_data(vdata);
    geom->add_primitive(strip);

    _viz_geom->add_geom(geom, get_solid_viz_state());
    _bounds_viz_geom->add_geom(geom, get_solid_bounds_viz_state());

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
        verts.push_back(compute_point(latitude, longitude0));
        verts.push_back(compute_point(latitude, longitude1));
      }
      verts.push_back(compute_point(1.0, longitude0));
      
      lengths.push_back(num_stacks * 2);
    }
    
    sphere->set_coords(verts);
    sphere->set_lengths(lengths);
    sphere->set_num_prims(num_slices);
    
    _viz_geom->add_geom(sphere, get_solid_viz_state());
    _bounds_viz_geom->add_geom(sphere, get_solid_bounds_viz_state());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::intersects_line
//       Access: Protected
//  Description: Determine the point(s) of intersection of a parametric
//               line with the sphere.  The line is infinite in both
//               directions, and passes through "from" and from+delta.
//               If the line does not intersect the sphere, the
//               function returns false, and t1 and t2 are undefined.
//               If it does intersect the sphere, it returns true, and
//               t1 and t2 are set to the points along the equation
//               from+t*delta that correspond to the two points of
//               intersection.
////////////////////////////////////////////////////////////////////
bool CollisionSphere::
intersects_line(double &t1, double &t2,
                const LPoint3f &from, const LVector3f &delta) const {
  // Solve the equation for the intersection of a line with a sphere
  // using the quadratic equation.

  // A line segment from f to f+d is defined as all P such that
  // P = f + td for 0 <= t <= 1.

  // A sphere with radius r about point c is defined as all P such
  // that r^2 = (P - c)^2.

  // Subsituting P in the above we have:

  // r^2 = (f + td - c)^2 =
  // (f^2 + ftd - fc + ftd + t^2d^2 - tdc - fc - tdc + c^2) =
  // t^2(d^2) + t(fd + fd - dc - dc) + (f^2 - fc - fc + c^2) =
  // t^2(d^2) + t(2d(f - c)) + (f - c)^2

  // Thus, the equation is quadratic in t, and we have
  // at^2 + bt + c = 0

  // Where  a = d^2
  //        b = 2d(f - c)
  //        c = (f - c)^2 - r^2

  // Solving for t using the quadratic equation gives us the point of
  // intersection along the line segment.  Actually, there are two
  // solutions (since it is quadratic): one for the front of the
  // sphere, and one for the back.  In the case where the line is
  // tangent to the sphere, there is only one solution (and the
  // radical is zero).

  double A = dot(delta, delta);

  nassertr(A != 0.0, false);

  LVector3f fc = from - get_center();
  double B = 2.0f* dot(delta, fc);
  double fc_d2 = dot(fc, fc);
  double C = fc_d2 - get_radius() * get_radius();

  double radical = B*B - 4.0*A*C;

  if (IS_NEARLY_ZERO(radical)) {
    // Tangent.
    t1 = t2 = -B /(2.0*A);
    return true;

  } else if (radical < 0.0) {
    // No real roots: no intersection with the line.
    return false;
  }

  double reciprocal_2A = 1.0/(2.0*A);
  double sqrt_radical = sqrtf(radical);
  t1 = ( -B - sqrt_radical ) * reciprocal_2A;
  t2 = ( -B + sqrt_radical ) * reciprocal_2A;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::compute_point
//       Access: Protected
//  Description: Returns a point on the surface of the sphere.
//               latitude and longitude range from 0.0 to 1.0.  This
//               is used by fill_viz_geom() to create a visible
//               representation of the sphere.
////////////////////////////////////////////////////////////////////
Vertexf CollisionSphere::
compute_point(float latitude, float longitude) const {
  float s1, c1;
  csincos(latitude * MathNumbers::pi_f, &s1, &c1);

  float s2, c2;
  csincos(longitude * 2.0f * MathNumbers::pi_f, &s2, &c2);

  Vertexf p(s1 * c2, s1 * s2, c1);
  return p * get_radius() + get_center();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a CollisionSphere object
////////////////////////////////////////////////////////////////////
void CollisionSphere::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_CollisionSphere);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void CollisionSphere::
write_datagram(BamWriter *manager, Datagram &me) {
  CollisionSolid::write_datagram(manager, me);
  _center.write_datagram(me);
  me.add_float32(_radius);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::make_CollisionSphere
//       Access: Protected
//  Description: Factory method to generate a CollisionSphere object
////////////////////////////////////////////////////////////////////
TypedWritable *CollisionSphere::
make_CollisionSphere(const FactoryParams &params) {
  CollisionSphere *me = new CollisionSphere;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void CollisionSphere::
fillin(DatagramIterator& scan, BamReader* manager) {
  CollisionSolid::fillin(scan, manager);
  _center.read_datagram(scan);
  _radius = scan.get_float32();
}
