// Filename: collisionTube.cxx
// Created by:  drose (25Sep03)
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

#include "collisionTube.h"
#include "collisionSphere.h"
#include "collisionLine.h"
#include "collisionRay.h"
#include "collisionSegment.h"
#include "collisionHandler.h"
#include "collisionEntry.h"
#include "collisionParabola.h"
#include "config_collide.h"
#include "look_at.h"
#include "geom.h"
#include "geomNode.h"
#include "geometricBoundingVolume.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "cmath.h"
#include "transformState.h"
#include "geom.h"
#include "geomTristrips.h"
#include "geomVertexWriter.h"
#include "boundingSphere.h"

PStatCollector CollisionTube::_volume_pcollector("Collision Volumes:CollisionTube");
PStatCollector CollisionTube::_test_pcollector("Collision Tests:CollisionTube");
TypeHandle CollisionTube::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid *CollisionTube::
make_copy() {
  return new CollisionTube(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::xform
//       Access: Public, Virtual
//  Description: Transforms the solid by the indicated matrix.
////////////////////////////////////////////////////////////////////
void CollisionTube::
xform(const LMatrix4 &mat) {
  _a = _a * mat;
  _b = _b * mat;

  // This is a little cheesy and fails miserably in the presence of a
  // non-uniform scale.
  LVector3 radius_v = LVector3(_radius, 0.0f, 0.0f) * mat;
  _radius = length(radius_v);

  recalc_internals();
  CollisionSolid::xform(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::get_collision_origin
//       Access: Public, Virtual
//  Description: Returns the point in space deemed to be the "origin"
//               of the solid for collision purposes.  The closest
//               intersection point to this origin point is considered
//               to be the most significant.
////////////////////////////////////////////////////////////////////
LPoint3 CollisionTube::
get_collision_origin() const {
  return get_point_a();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::get_volume_pcollector
//       Access: Public, Virtual
//  Description: Returns a PStatCollector that is used to count the
//               number of bounding volume tests made against a solid
//               of this type in a given frame.
////////////////////////////////////////////////////////////////////
PStatCollector &CollisionTube::
get_volume_pcollector() {
  return _volume_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::get_test_pcollector
//       Access: Public, Virtual
//  Description: Returns a PStatCollector that is used to count the
//               number of intersection tests made against a solid
//               of this type in a given frame.
////////////////////////////////////////////////////////////////////
PStatCollector &CollisionTube::
get_test_pcollector() {
  return _test_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionTube::
output(ostream &out) const {
  out << "tube, a (" << _a << "), b (" << _b << "), r " << _radius;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::compute_internal_bounds
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(BoundingVolume) CollisionTube::
compute_internal_bounds() const {
  PT(BoundingVolume) bound = CollisionSolid::compute_internal_bounds();

  if (bound->is_of_type(GeometricBoundingVolume::get_class_type())) {
    GeometricBoundingVolume *gbound;
    DCAST_INTO_R(gbound, bound, bound);

    LVector3 vec = (_b - _a);
    if (vec.normalize()) {
      // The bounding volume includes both endpoints, plus a little
      // bit more to include the radius in both directions.
      LPoint3 points[2];
      points[0] = _a - vec * _radius;
      points[1] = _b + vec * _radius;

      gbound->around(points, points + 2);

    } else {
      // Both endpoints are coincident; therefore, the bounding volume
      // is a sphere.
      BoundingSphere sphere(_a, _radius);
      gbound->extend_by(&sphere);
    }
  }

  return bound;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::test_intersection_from_sphere
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionTube::
test_intersection_from_sphere(const CollisionEntry &entry) const {
  const CollisionSphere *sphere;
  DCAST_INTO_R(sphere, entry.get_from(), 0);

  CPT(TransformState) wrt_space = entry.get_wrt_space();
  CPT(TransformState) wrt_prev_space = entry.get_wrt_prev_space();

  const LMatrix4 &wrt_mat = wrt_space->get_mat();

  LPoint3 from_a = sphere->get_center() * wrt_mat;
  LPoint3 from_b = from_a;

  LPoint3 contact_point;
  PN_stdfloat actual_t = 0.0f;

  if (wrt_prev_space != wrt_space) {
    // If the sphere is moving relative to the tube, it becomes a tube
    // itself.
    from_a = sphere->get_center() * wrt_prev_space->get_mat();
  }

  LVector3 from_direction = from_b - from_a;

  LVector3 from_radius_v =
    LVector3(sphere->get_radius(), 0.0f, 0.0f) * wrt_mat;
  PN_stdfloat from_radius = length(from_radius_v);

  double t1, t2;
  if (!intersects_line(t1, t2, from_a, from_direction, from_radius)) {
    // No intersection.
    return NULL;
  }

  if (t2 < 0.0 || t1 > 1.0) {
    // Both intersection points are before the start of the segment or
    // after the end of the segment.
    return NULL;
  }

  // doubles, not floats, to satisfy min and max templates.
  actual_t = min(1.0, max(0.0, t1));
  contact_point = from_a + actual_t * (from_b - from_a);

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path() << " into "
      << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3 into_intersection_point;
  if (t2 > 1.0) {
    // Point b is within the tube.  The first intersection point is
    // point b itself.
    into_intersection_point = from_b;
  } else {
    // Point b is outside the tube, and point a is either inside the
    // tube or beyond it.  The first intersection point is at t2.
    into_intersection_point = from_a + t2 * from_direction;
  }
  set_intersection_point(new_entry, into_intersection_point, from_radius);

  LPoint3 fake_contact_point;
  LVector3 contact_normal;
  calculate_surface_point_and_normal(contact_point,
                                     from_radius,
                                     fake_contact_point,
                                     contact_normal);
  new_entry->set_contact_pos(contact_point);
  new_entry->set_contact_normal(contact_normal);
  new_entry->set_t(actual_t);

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::test_intersection_from_line
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionTube::
test_intersection_from_line(const CollisionEntry &entry) const {
  const CollisionLine *line;
  DCAST_INTO_R(line, entry.get_from(), 0);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_origin = line->get_origin() * wrt_mat;
  LVector3 from_direction = line->get_direction() * wrt_mat;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction, 0.0f)) {
    // No intersection.
    return NULL;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3 into_intersection_point = from_origin + t1 * from_direction;
  set_intersection_point(new_entry, into_intersection_point, 0.0);

  if (has_effective_normal() && line->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());

  } else {
    LVector3 normal = into_intersection_point * _inv_mat;
    if (normal[1] > _length) {
      // The point is within the top endcap.
      normal[1] -= _length;
    } else if (normal[1] > 0.0f) {
      // The point is within the cylinder body.
      normal[1] = 0;
    }
    normal = normalize(normal * _mat);
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::test_intersection_from_ray
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionTube::
test_intersection_from_ray(const CollisionEntry &entry) const {
  const CollisionRay *ray;
  DCAST_INTO_R(ray, entry.get_from(), 0);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_origin = ray->get_origin() * wrt_mat;
  LVector3 from_direction = ray->get_direction() * wrt_mat;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction, 0.0f)) {
    // No intersection.
    return NULL;
  }

  if (t2 < 0.0) {
    // Both intersection points are before the start of the ray.
    return NULL;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3 into_intersection_point;
  if (t1 < 0.0) {
    // Point a is within the tube.  The first intersection point is
    // point a itself.
    into_intersection_point = from_origin;
  } else {
    // Point a is outside the tube.  The first intersection point is
    // at t1.
    into_intersection_point = from_origin + t1 * from_direction;
  }
  set_intersection_point(new_entry, into_intersection_point, 0.0);

  if (has_effective_normal() && ray->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());

  } else {
    LVector3 normal = into_intersection_point * _inv_mat;
    if (normal[1] > _length) {
      // The point is within the top endcap.
      normal[1] -= _length;
    } else if (normal[1] > 0.0f) {
      // The point is within the cylinder body.
      normal[1] = 0;
    }
    normal = normalize(normal * _mat);
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::test_intersection_from_segment
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionTube::
test_intersection_from_segment(const CollisionEntry &entry) const {
  const CollisionSegment *segment;
  DCAST_INTO_R(segment, entry.get_from(), 0);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_a = segment->get_point_a() * wrt_mat;
  LPoint3 from_b = segment->get_point_b() * wrt_mat;
  LVector3 from_direction = from_b - from_a;

  double t1, t2;
  if (!intersects_line(t1, t2, from_a, from_direction, 0.0f)) {
    // No intersection.
    return NULL;
  }

  if (t2 < 0.0 || t1 > 1.0) {
    // Both intersection points are before the start of the segment or
    // after the end of the segment.
    return NULL;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3 into_intersection_point;
  if (t1 < 0.0) {
    // Point a is within the tube.  The first intersection point is
    // point a itself.
    into_intersection_point = from_a;
  } else {
    // Point a is outside the tube, and point b is either inside the
    // tube or beyond it.  The first intersection point is at t1.
    into_intersection_point = from_a + t1 * from_direction;
  }
  set_intersection_point(new_entry, into_intersection_point, 0.0);

  if (has_effective_normal() && segment->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());

  } else {
    LVector3 normal = into_intersection_point * _inv_mat;
    if (normal[1] > _length) {
      // The point is within the top endcap.
      normal[1] -= _length;
    } else if (normal[1] > 0.0f) {
      // The point is within the cylinder body.
      normal[1] = 0;
    }
    normal = normalize(normal * _mat);
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::test_intersection_from_parabola
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionTube::
test_intersection_from_parabola(const CollisionEntry &entry) const {
  const CollisionParabola *parabola;
  DCAST_INTO_R(parabola, entry.get_from(), 0);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  // Convert the parabola into local coordinate space.
  LParabola local_p(parabola->get_parabola());
  local_p.xform(wrt_mat);

  double t;
  if (!intersects_parabola(t, local_p, parabola->get_t1(), parabola->get_t2(),
                           local_p.calc_point(parabola->get_t1()),
                           local_p.calc_point(parabola->get_t2()))) {
    // No intersection.
    return NULL;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3 into_intersection_point = local_p.calc_point(t);
  set_intersection_point(new_entry, into_intersection_point, 0.0);

  if (has_effective_normal() && parabola->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());

  } else {
    LVector3 normal = into_intersection_point * _inv_mat;
    if (normal[1] > _length) {
      // The point is within the top endcap.
      normal[1] -= _length;
    } else if (normal[1] > 0.0f) {
      // The point is within the cylinder body.
      normal[1] = 0;
    }
    normal = normalize(normal * _mat);
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::fill_viz_geom
//       Access: Protected, Virtual
//  Description: Fills the _viz_geom GeomNode up with Geoms suitable
//               for rendering this solid.
////////////////////////////////////////////////////////////////////
void CollisionTube::
fill_viz_geom() {
  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "Recomputing viz for " << *this << "\n";
  }

  // Generate the vertices such that we draw a tube with one endpoint
  // at (0, 0, 0), and another at (0, length, 0).  Then we'll rotate
  // and translate it into place with the appropriate look_at matrix.
  LVector3 direction = (_b - _a);
  PN_stdfloat length = direction.length();

  PT(GeomVertexData) vdata = new GeomVertexData
    ("collision", GeomVertexFormat::get_v3(),
     Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  
  PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_static);
  // Generate the first endcap.
  static const int num_slices = 8;
  static const int num_rings = 4;
  int ri, si;
  for (ri = 0; ri < num_rings; ri++) {
    for (si = 0; si <= num_slices; si++) {
      vertex.add_data3(calc_sphere1_vertex(ri, si, num_rings, num_slices));
      vertex.add_data3(calc_sphere1_vertex(ri + 1, si, num_rings, num_slices));
    }
    strip->add_next_vertices((num_slices + 1) * 2);
    strip->close_primitive();
  }
  
  // Now the cylinder sides.
  for (si = 0; si <= num_slices; si++) {
    vertex.add_data3(calc_sphere1_vertex(num_rings, si, num_rings, num_slices));
    vertex.add_data3(calc_sphere2_vertex(num_rings, si, num_rings, num_slices,
                                          length));
  }
  strip->add_next_vertices((num_slices + 1) * 2);
  strip->close_primitive();
  
  // And the second endcap.
  for (ri = num_rings - 1; ri >= 0; ri--) {
    for (si = 0; si <= num_slices; si++) {
      vertex.add_data3(calc_sphere2_vertex(ri + 1, si, num_rings, num_slices, length));
      vertex.add_data3(calc_sphere2_vertex(ri, si, num_rings, num_slices, length));
    }
    strip->add_next_vertices((num_slices + 1) * 2);
    strip->close_primitive();
  }
  
  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(strip);
  
  // Now transform the vertices to their actual location.
  LMatrix4 mat;
  look_at(mat, direction, LVector3(0.0f, 0.0f, 1.0f), CS_zup_right);
  mat.set_row(3, _a);
  geom->transform_vertices(mat);
  
  _viz_geom->add_geom(geom, get_solid_viz_state());
  _bounds_viz_geom->add_geom(geom, get_solid_bounds_viz_state());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::recalc_internals
//       Access: Private
//  Description: Should be called internally to recompute the matrix
//               and length when the properties of the tube have
//               changed.
////////////////////////////////////////////////////////////////////
void CollisionTube::
recalc_internals() {
  LVector3 direction = (_b - _a);
  _length = direction.length();

  look_at(_mat, direction, LVector3(0.0f, 0.0f, 1.0f), CS_zup_right);
  _mat.set_row(3, _a);
  _inv_mat.invert_from(_mat);

  mark_viz_stale();
  mark_internal_bounds_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::calc_sphere1_vertex
//       Access: Private
//  Description: Calculates a particular vertex on the surface of the
//               first endcap hemisphere, for use in generating the
//               viz geometry.
////////////////////////////////////////////////////////////////////
LVertex CollisionTube::
calc_sphere1_vertex(int ri, int si, int num_rings, int num_slices) {
  PN_stdfloat r = (PN_stdfloat)ri / (PN_stdfloat)num_rings;
  PN_stdfloat s = (PN_stdfloat)si / (PN_stdfloat)num_slices;

  // Find the point on the rim, based on the slice.
  PN_stdfloat theta = s * 2.0f * MathNumbers::pi;
  PN_stdfloat x_rim = ccos(theta);
  PN_stdfloat z_rim = csin(theta);

  // Now pull that point in towards the pole, based on the ring.
  PN_stdfloat phi = r * 0.5f * MathNumbers::pi;
  PN_stdfloat to_pole = csin(phi);

  PN_stdfloat x = _radius * x_rim * to_pole;
  PN_stdfloat y = -_radius * ccos(phi);
  PN_stdfloat z = _radius * z_rim * to_pole;

  return LVertex(x, y, z);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::calc_sphere2_vertex
//       Access: Private
//  Description: Calculates a particular vertex on the surface of the
//               second endcap hemisphere, for use in generating the
//               viz geometry.
////////////////////////////////////////////////////////////////////
LVertex CollisionTube::
calc_sphere2_vertex(int ri, int si, int num_rings, int num_slices,
                    PN_stdfloat length) {
  PN_stdfloat r = (PN_stdfloat)ri / (PN_stdfloat)num_rings;
  PN_stdfloat s = (PN_stdfloat)si / (PN_stdfloat)num_slices;

  // Find the point on the rim, based on the slice.
  PN_stdfloat theta = s * 2.0f * MathNumbers::pi;
  PN_stdfloat x_rim = ccos(theta);
  PN_stdfloat z_rim = csin(theta);

  // Now pull that point in towards the pole, based on the ring.
  PN_stdfloat phi = r * 0.5f * MathNumbers::pi;
  PN_stdfloat to_pole = csin(phi);

  PN_stdfloat x = _radius * x_rim * to_pole;
  PN_stdfloat y = length + _radius * ccos(phi);
  PN_stdfloat z = _radius * z_rim * to_pole;

  return LVertex(x, y, z);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::intersects_line
//       Access: Private
//  Description: Determine the point(s) of intersection of a parametric
//               line with the tube.  The line is infinite in both
//               directions, and passes through "from" and from+delta.
//               If the line does not intersect the tube, the
//               function returns false, and t1 and t2 are undefined.
//               If it does intersect the tube, it returns true, and
//               t1 and t2 are set to the points along the equation
//               from+t*delta that correspond to the two points of
//               intersection.
////////////////////////////////////////////////////////////////////
bool CollisionTube::
intersects_line(double &t1, double &t2,
                const LPoint3 &from0, const LVector3 &delta0, 
                PN_stdfloat inflate_radius) const {
  // Convert the line into our canonical coordinate space: the tube is
  // aligned with the y axis.
  LPoint3 from = from0 * _inv_mat;
  LVector3 delta = delta0 * _inv_mat;

  PN_stdfloat radius = _radius + inflate_radius;

  // Now project the line into the X-Z plane to test for intersection
  // with a 2-d circle around the origin.  The equation for this is
  // very similar to the formula for the intersection of a line with a
  // sphere; see CollisionSphere::intersects_line() for the complete
  // derivation.  It's a little bit simpler because the circle is
  // centered on the origin.
  LVector2 from2(from[0], from[2]);
  LVector2 delta2(delta[0], delta[2]);

  double A = dot(delta2, delta2);

  if (IS_NEARLY_ZERO(A)) {
    // If the delta2 is 0, the line is perpendicular to the X-Z plane.
    // The whole line intersects with the infinite cylinder if the
    // point is within the circle.
    if (from2.dot(from2) > radius * radius) {
      // Nope, the 2-d point is outside the circle, so no
      // intersection.
      return false;
    }

    if (IS_NEARLY_ZERO(delta[1])) {
      // Actually, the whole delta vector is 0, so the line is just a
      // point.  In this case, (since we have already shown the point
      // is within the infinite cylinder), we intersect if and only if
      // the three-dimensional point is between the endcaps.
      if (from[1] < -radius || from[1] > _length + radius) {
        // Way out.
        return false;
      }
      if (from[1] < 0.0f) {
        // Possibly within the first endcap.
        if (from.dot(from) > radius * radius) {
          return false;
        }
      } else if (from[1] > _length) {
        // Possibly within the second endcap.
        from[1] -= _length;
        if (from.dot(from) > radius * radius) {
          return false;
        }
      }

      // The point is within the tube!
      t1 = t2 = 0.0;
      return true;
    }

    // The 2-d point is within the circle, so compute our intersection
    // points to include the entire vertical slice of the cylinder.
    t1 = (-radius - from[1]) / delta[1];
    t2 = (_length + radius - from[1]) / delta[1];

  } else {
    // The line is not perpendicular to the X-Z plane, so its
    // projection into the plane is 2-d line.  Test that 2-d line for
    // intersection with the circular projection of the cylinder.

    double B = 2.0f * dot(delta2, from2);
    double fc_d2 = dot(from2, from2);
    double C = fc_d2 - radius * radius;
    
    double radical = B*B - 4.0*A*C;
    
    if (IS_NEARLY_ZERO(radical)) {
      // Tangent.
      t1 = t2 = -B / (2.0*A);
      
    } else if (radical < 0.0) {
      // No real roots: no intersection with the line.
      return false;
      
    } else {
      double reciprocal_2A = 1.0 / (2.0 * A);
      double sqrt_radical = sqrtf(radical);
      t1 = ( -B - sqrt_radical ) * reciprocal_2A;
      t2 = ( -B + sqrt_radical ) * reciprocal_2A;
    }
  }

  // Now we need to verify that the intersection points fall within
  // the length of the cylinder.
  PN_stdfloat t1_y = from[1] + t1 * delta[1];
  PN_stdfloat t2_y = from[1] + t2 * delta[1];

  if (t1_y < -radius && t2_y < -radius) {
    // Both points are way off the bottom of the tube; no
    // intersection.
    return false;
  } else if (t1_y > _length + radius && t2_y > _length + radius) {
    // Both points are way off the top of the tube; no intersection.
    return false;
  }

  if (t1_y < 0.0f) {
    // The starting point is off the bottom of the tube.  Test the
    // line against the first endcap.
    double t1a, t2a;
    if (!sphere_intersects_line(t1a, t2a, 0.0f, from, delta, inflate_radius)) {
      // If there's no intersection with the endcap, there can't be an
      // intersection with the cylinder.
      return false;
    }
    t1 = t1a;

  } else if (t1_y > _length) {
    // The starting point is off the top of the tube.  Test the
    // line against the second endcap.
    double t1b, t2b;
    if (!sphere_intersects_line(t1b, t2b, _length, from, delta, inflate_radius)) {
      // If there's no intersection with the endcap, there can't be an
      // intersection with the cylinder.
      return false;
    }
    t1 = t1b;
  }

  if (t2_y < 0.0f) {
    // The ending point is off the bottom of the tube.  Test the
    // line against the first endcap.
    double t1a, t2a;
    if (!sphere_intersects_line(t1a, t2a, 0.0f, from, delta, inflate_radius)) {
      // If there's no intersection with the endcap, there can't be an
      // intersection with the cylinder.
      return false;
    }
    t2 = t2a;

  } else if (t2_y > _length) {
    // The ending point is off the top of the tube.  Test the
    // line against the second endcap.
    double t1b, t2b;
    if (!sphere_intersects_line(t1b, t2b, _length, from, delta, inflate_radius)) {
      // If there's no intersection with the endcap, there can't be an
      // intersection with the cylinder.
      return false;
    }
    t2 = t2b;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::sphere_intersects_line
//       Access: Private
//  Description: After confirming that the line intersects an infinite
//               cylinder, test whether it intersects one or the other
//               endcaps.  The y parameter specifies the center of the
//               sphere (and hence the particular endcap.
////////////////////////////////////////////////////////////////////
bool CollisionTube::
sphere_intersects_line(double &t1, double &t2, PN_stdfloat center_y,
                       const LPoint3 &from, const LVector3 &delta,
                       PN_stdfloat inflate_radius) const {
  // See CollisionSphere::intersects_line() for a derivation of the
  // formula here.
  PN_stdfloat radius = _radius + inflate_radius;

  double A = dot(delta, delta);

  nassertr(A != 0.0, false);

  LVector3 fc = from;
  fc[1] -= center_y;
  double B = 2.0f* dot(delta, fc);
  double fc_d2 = dot(fc, fc);
  double C = fc_d2 - radius * radius;

  double radical = B*B - 4.0*A*C;

  if (IS_NEARLY_ZERO(radical)) {
    // Tangent.
    t1 = t2 = -B / (2.0 * A);
    return true;

  } else if (radical < 0.0) {
    // No real roots: no intersection with the line.
    return false;
  }

  double reciprocal_2A = 1.0 / (2.0 * A);
  double sqrt_radical = sqrtf(radical);
  t1 = ( -B - sqrt_radical ) * reciprocal_2A;
  t2 = ( -B + sqrt_radical ) * reciprocal_2A;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::intersects_parabola
//       Access: Protected
//  Description: Determine a point of intersection of a parametric
//               parabola with the tube.
//
//               We only consider the segment of the parabola between
//               t1 and t2, which has already been computed as
//               corresponding to points p1 and p2.  If there is an
//               intersection, t is set to the parametric point of
//               intersection, and true is returned; otherwise, false
//               is returned.
////////////////////////////////////////////////////////////////////
bool CollisionTube::
intersects_parabola(double &t, const LParabola &parabola,
                    double t1, double t2,
                    const LPoint3 &p1, const LPoint3 &p2) const {
  // I don't even want to think about the math to do this calculation
  // directly--it's even worse than sphere-parabola.  So I'll use the
  // recursive subdivision solution again, just like I did for
  // sphere-parabola.

  // First, see if the line segment (p1 - p2) comes sufficiently close
  // to the parabola.  Do this by computing the parametric intervening
  // point and comparing its distance from the linear intervening
  // point.
  double tmid = (t1 + t2) * 0.5;

  if (tmid != t1 && tmid != t2) {
    LPoint3 pmid = parabola.calc_point(tmid);
    LPoint3 pmid2 = (p1 + p2) * 0.5f;

    if ((pmid - pmid2).length_squared() > 0.001f) {
      // Subdivide.
      if (intersects_parabola(t, parabola, t1, tmid, p1, pmid)) {
        return true;
      }
      return intersects_parabola(t, parabola, tmid, t2, pmid, p2);
    }
  }

  // The line segment is sufficiently close; compare the segment itself.
  double t1a, t2a;
  if (!intersects_line(t1a, t2a, p1, p2 - p1, 0.0f)) {
    return false;
  }

  if (t2a < 0.0 || t1a > 1.0) {
    return false;
  }

  t = max(t1a, 0.0);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::calculate_surface_point_and_normal
//       Access: Private
//  Description: Calculates a point that is exactly on the surface
//               of the tube and its corresponding normal, given
//               a point that is supposedly on the surface of the
//               tube.
////////////////////////////////////////////////////////////////////
void CollisionTube::
calculate_surface_point_and_normal(const LPoint3 &surface_point,
                                   double extra_radius,
                                   LPoint3 &result_point,
                                   LVector3 &result_normal) const {
  // Convert the point into our canonical space for analysis.
  LPoint3 point = surface_point * _inv_mat;
  LVector3 normal;

  if (point[1] <= 0.0) {
    // The point is on the first endcap.
    normal = point;
    if (!normal.normalize()) {
      normal.set(0.0, -1.0, 0.0);
    }
    point = normal * _radius;

  } else if (point[1] >= _length) {
    // The point is on the second endcap.
    normal.set(point[0], point[1] - _length, point[2]);
    if (!normal.normalize()) {
      normal.set(0.0, 1.0, 0.0);
    }
    point = normal * _radius;
    point[1] += _length;

  } else {
    // The point is within the cylinder part.
    LVector2d normal2d(point[0], point[2]);
    if (!normal2d.normalize()) {
      normal2d.set(0.0, 1.0);
    }
    normal.set(normal2d[0], 0.0, normal2d[1]);
    point.set(normal[0] * _radius, point[1], normal[2] * _radius);
  }

  // Now convert the point and normal back into real space.
  result_point = point * _mat;
  result_normal = normal * _mat;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::set_intersection_point
//       Access: Private
//  Description: After an intersection has been detected, record the
//               computed intersection point in the CollisionEntry,
//               and also compute the relevant normal based on that
//               point.
////////////////////////////////////////////////////////////////////
void CollisionTube::
set_intersection_point(CollisionEntry *new_entry, 
                       const LPoint3 &into_intersection_point, 
                       double extra_radius) const {
  LPoint3 point;
  LVector3 normal;

  calculate_surface_point_and_normal(into_intersection_point,
                                     extra_radius,
                                     point,
                                     normal);

  if (has_effective_normal() && new_entry->get_from()->get_respect_effective_normal()) {
    normal = get_effective_normal();
  }

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(point);
  // Also adjust the original point into the tube by the amount of
  // extra_radius, which should put it on the surface of the tube if
  // our collision was tangential.
  new_entry->set_interior_point(into_intersection_point - normal * extra_radius);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               CollisionTube.
////////////////////////////////////////////////////////////////////
void CollisionTube::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void CollisionTube::
write_datagram(BamWriter *manager, Datagram &dg) {
  CollisionSolid::write_datagram(manager, dg);
  _a.write_datagram(dg);
  _b.write_datagram(dg);
  dg.add_stdfloat(_radius);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type CollisionTube is encountered
//               in the Bam file.  It should create the CollisionTube
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *CollisionTube::
make_from_bam(const FactoryParams &params) {
  CollisionTube *node = new CollisionTube();
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTube::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new CollisionTube.
////////////////////////////////////////////////////////////////////
void CollisionTube::
fillin(DatagramIterator &scan, BamReader *manager) {
  CollisionSolid::fillin(scan, manager);
  _a.read_datagram(scan);
  _b.read_datagram(scan);
  _radius = scan.get_stdfloat();
  recalc_internals();
}
