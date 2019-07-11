/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionSphere.cxx
 * @author drose
 * @date 2000-04-24
 */

#include "collisionSphere.h"
#include "collisionLine.h"
#include "collisionRay.h"
#include "collisionHandler.h"
#include "collisionEntry.h"
#include "collisionSegment.h"
#include "collisionCapsule.h"
#include "collisionParabola.h"
#include "collisionBox.h"
#include "config_collide.h"
#include "boundingSphere.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "nearly_zero.h"
#include "cmath.h"
#include "mathNumbers.h"
#include "geom.h"
#include "geomTristrips.h"
#include "geomVertexWriter.h"

using std::max;
using std::min;

PStatCollector CollisionSphere::_volume_pcollector(
  "Collision Volumes:CollisionSphere");
PStatCollector CollisionSphere::_test_pcollector(
  "Collision Tests:CollisionSphere");
TypeHandle CollisionSphere::_type_handle;

/**
 *
 */
CollisionSolid *CollisionSphere::
make_copy() {
  return new CollisionSphere(*this);
}

/**
 *
 */
PT(CollisionEntry) CollisionSphere::
test_intersection(const CollisionEntry &entry) const {
  return entry.get_into()->test_intersection_from_sphere(entry);
}

/**
 * Transforms the solid by the indicated matrix.
 */
void CollisionSphere::
xform(const LMatrix4 &mat) {
  _center = _center * mat;

  // This is a little cheesy and fails miserably in the presence of a non-
  // uniform scale.
  LVector3 radius_v = LVector3(_radius, 0.0f, 0.0f) * mat;
  _radius = length(radius_v);
  mark_viz_stale();
  mark_internal_bounds_stale();
}

/**
 * Returns the point in space deemed to be the "origin" of the solid for
 * collision purposes.  The closest intersection point to this origin point is
 * considered to be the most significant.
 */
LPoint3 CollisionSphere::
get_collision_origin() const {
  return get_center();
}

/**
 * Returns a PStatCollector that is used to count the number of bounding
 * volume tests made against a solid of this type in a given frame.
 */
PStatCollector &CollisionSphere::
get_volume_pcollector() {
  return _volume_pcollector;
}

/**
 * Returns a PStatCollector that is used to count the number of intersection
 * tests made against a solid of this type in a given frame.
 */
PStatCollector &CollisionSphere::
get_test_pcollector() {
  return _test_pcollector;
}

/**
 *
 */
void CollisionSphere::
output(std::ostream &out) const {
  out << "sphere, c (" << get_center() << "), r " << get_radius();
}

/**
 *
 */
PT(BoundingVolume) CollisionSphere::
compute_internal_bounds() const {
  return new BoundingSphere(_center, _radius);
}

/**
 *
 */
PT(CollisionEntry) CollisionSphere::
test_intersection_from_sphere(const CollisionEntry &entry) const {
  const CollisionSphere *sphere;
  DCAST_INTO_R(sphere, entry.get_from(), nullptr);

  CPT(TransformState) wrt_space = entry.get_wrt_space();

  const LMatrix4 &wrt_mat = wrt_space->get_mat();

  LPoint3 from_b = sphere->get_center() * wrt_mat;

  LPoint3 into_center(get_center());
  PN_stdfloat into_radius(get_radius());

  LVector3 from_radius_v =
    LVector3(sphere->get_radius(), 0.0f, 0.0f) * wrt_mat;
  PN_stdfloat from_radius = length(from_radius_v);

  LPoint3 into_intersection_point(from_b);
  double t1, t2;
  LPoint3 contact_point(into_intersection_point);
  PN_stdfloat actual_t = 0.0f;

  LVector3 vec = from_b - into_center;
  PN_stdfloat dist2 = dot(vec, vec);
  if (dist2 > (into_radius + from_radius) * (into_radius + from_radius)) {
    // No intersection with the current position.  Check the delta from the
    // previous frame.
    CPT(TransformState) wrt_prev_space = entry.get_wrt_prev_space();
    LPoint3 from_a = sphere->get_center() * wrt_prev_space->get_mat();

    if (!from_a.almost_equal(from_b)) {
      LVector3 from_direction = from_b - from_a;
      if (!intersects_line(t1, t2, from_a, from_direction, from_radius)) {
        // No intersection.
        return nullptr;
      }

      if (t2 < 0.0 || t1 > 1.0) {
        // Both intersection points are before the start of the segment or
        // after the end of the segment.
        return nullptr;
      }

      // doubles, not floats, to satisfy min and max templates.
      actual_t = min(1.0, max(0.0, t1));
      contact_point = from_a + actual_t * (from_b - from_a);

      if (t1 < 0.0) {
        // Point a is within the sphere.  The first intersection point is
        // point a itself.
        into_intersection_point = from_a;
      } else {
        // Point a is outside the sphere, and point b is either inside the
        // sphere or beyond it.  The first intersection point is at t1.
        into_intersection_point = from_a + t1 * from_direction;
      }
    } else {
      // No delta, therefore no intersection.
      return nullptr;
    }
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3 from_center = sphere->get_center() * wrt_mat;

  LVector3 surface_normal;
  LVector3 v(into_intersection_point - into_center);
  PN_stdfloat vec_length = v.length();
  if (IS_NEARLY_ZERO(vec_length)) {
    // If we don't have a collision normal (e.g.  the centers are exactly
    // coincident), then make up an arbitrary normal--any one is as good as
    // any other.
    surface_normal.set(1.0, 0.0, 0.0);
  } else {
    surface_normal = v / vec_length;
  }

  LVector3 eff_normal = (has_effective_normal() && sphere->get_respect_effective_normal()) ? get_effective_normal() : surface_normal;

  LVector3 contact_normal;
  LVector3 v2 = contact_point - into_center;
  PN_stdfloat v2_len = v2.length();
  if (IS_NEARLY_ZERO(v2_len)) {
    // If we don't have a collision normal (e.g.  the centers are exactly
    // coincident), then make up an arbitrary normal--any one is as good as
    // any other.
    contact_normal.set(1.0, 0.0, 0.0);
  } else {
    contact_normal = v2 / v2_len;
  }

  new_entry->set_surface_normal(eff_normal);
  new_entry->set_surface_point(into_center + surface_normal * into_radius);
  new_entry->set_interior_point(from_center - surface_normal * from_radius);
  new_entry->set_contact_pos(contact_point);
  new_entry->set_contact_normal(contact_normal);
  new_entry->set_t(actual_t);

  return new_entry;
}

/**
 *
 */
PT(CollisionEntry) CollisionSphere::
test_intersection_from_line(const CollisionEntry &entry) const {
  const CollisionLine *line;
  DCAST_INTO_R(line, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_origin = line->get_origin() * wrt_mat;
  LVector3 from_direction = line->get_direction() * wrt_mat;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction, 0.0f)) {
    // No intersection.
    return nullptr;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3 into_intersection_point = from_origin + t1 * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && line->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3 normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

/**
 * Double dispatch point for box as a FROM object
 */
PT(CollisionEntry) CollisionSphere::
test_intersection_from_box(const CollisionEntry &entry) const {
  const CollisionBox *box;
  DCAST_INTO_R(box, entry.get_from(), nullptr);

  // Instead of transforming the box into the sphere's coordinate space, we do
  // it the other way around.  It's easier that way.
  const LMatrix4 &wrt_mat = entry.get_inv_wrt_mat();

  LPoint3 center = wrt_mat.xform_point(_center);
  PN_stdfloat radius_sq = wrt_mat.xform_vec(LVector3(0, 0, _radius)).length_squared();

  LPoint3 box_min = box->get_min();
  LPoint3 box_max = box->get_max();

  // Arvo's algorithm.
  PN_stdfloat d = 0;
  PN_stdfloat s;

  if (center[0] < box_min[0]) {
    s = center[0] - box_min[0];
    d += s * s;

  } else if (center[0] > box_max[0]) {
    s = center[0] - box_max[0];
    d += s * s;
  }

  if (center[1] < box_min[1]) {
    s = center[1] - box_min[1];
    d += s * s;

  } else if (center[1] > box_max[1]) {
    s = center[1] - box_max[1];
    d += s * s;
  }

  if (center[2] < box_min[2]) {
    s = center[2] - box_min[2];
    d += s * s;

  } else if (center[2] > box_max[2]) {
    s = center[2] - box_max[2];
    d += s * s;
  }

  if (d > radius_sq) {
    return nullptr;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }

  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  // To get the interior point, clamp the sphere center to the AABB.
  LPoint3 interior = entry.get_wrt_mat().xform_point(center.fmax(box_min).fmin(box_max));
  new_entry->set_interior_point(interior);

  // Now extrapolate the surface point and normal from that.
  LVector3 normal = interior - _center;
  normal.normalize();
  new_entry->set_surface_point(_center + normal * _radius);
  new_entry->set_surface_normal(
    (has_effective_normal() && box->get_respect_effective_normal())
    ? get_effective_normal() : normal);

  return new_entry;
}

/**
 *
 */
PT(CollisionEntry) CollisionSphere::
test_intersection_from_ray(const CollisionEntry &entry) const {
  const CollisionRay *ray;
  DCAST_INTO_R(ray, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_origin = ray->get_origin() * wrt_mat;
  LVector3 from_direction = ray->get_direction() * wrt_mat;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction, 0.0f)) {
    // No intersection.
    return nullptr;
  }

  if (t2 < 0.0) {
    // Both intersection points are before the start of the ray.
    return nullptr;
  }

  t1 = max(t1, 0.0);

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3 into_intersection_point = from_origin + t1 * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && ray->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3 normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

/**
 *
 */
PT(CollisionEntry) CollisionSphere::
test_intersection_from_segment(const CollisionEntry &entry) const {
  const CollisionSegment *segment;
  DCAST_INTO_R(segment, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_a = segment->get_point_a() * wrt_mat;
  LPoint3 from_b = segment->get_point_b() * wrt_mat;
  LVector3 from_direction = from_b - from_a;

  double t1, t2;
  if (!intersects_line(t1, t2, from_a, from_direction, 0.0f)) {
    // No intersection.
    return nullptr;
  }

  if (t2 < 0.0 || t1 > 1.0) {
    // Both intersection points are before the start of the segment or after
    // the end of the segment.
    return nullptr;
  }

  t1 = max(t1, 0.0);

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3 into_intersection_point = from_a + t1 * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && segment->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3 normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

/**
 *
 */
PT(CollisionEntry) CollisionSphere::
test_intersection_from_capsule(const CollisionEntry &entry) const {
  const CollisionCapsule *capsule;
  DCAST_INTO_R(capsule, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_a = capsule->get_point_a() * wrt_mat;
  LPoint3 from_b = capsule->get_point_b() * wrt_mat;
  LVector3 from_direction = from_b - from_a;

  LVector3 from_radius_v =
    LVector3(capsule->get_radius(), 0.0f, 0.0f) * wrt_mat;
  PN_stdfloat from_radius = length(from_radius_v);

  double t1, t2;
  if (!intersects_line(t1, t2, from_a, from_direction, from_radius)) {
    // No intersection.
    return nullptr;
  }

  if (t2 < 0.0 || t1 > 1.0) {
    // Both intersection points are before the start of the capsule or after
    // the end of the capsule.
    return nullptr;
  }

  PN_stdfloat t = (t1 + t2) * (PN_stdfloat)0.5;
  t = max(t, (PN_stdfloat)0.0);
  t = min(t, (PN_stdfloat)1.0);
  LPoint3 inner_point = from_a + t * from_direction;

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LVector3 normal = inner_point - get_center();
  normal.normalize();
  new_entry->set_surface_point(get_center() + normal * get_radius());
  new_entry->set_interior_point(inner_point - normal * from_radius);

  if (has_effective_normal() && capsule->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

/**
 *
 */
PT(CollisionEntry) CollisionSphere::
test_intersection_from_parabola(const CollisionEntry &entry) const {
  const CollisionParabola *parabola;
  DCAST_INTO_R(parabola, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  // Convert the parabola into local coordinate space.
  LParabola local_p(parabola->get_parabola());
  local_p.xform(wrt_mat);

  double t;
  if (!intersects_parabola(t, local_p, parabola->get_t1(), parabola->get_t2(),
                           local_p.calc_point(parabola->get_t1()),
                           local_p.calc_point(parabola->get_t2()))) {
    // No intersection.
    return nullptr;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3 into_intersection_point = local_p.calc_point(t);
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && parabola->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3 normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

/**
 * Fills the _viz_geom GeomNode up with Geoms suitable for rendering this
 * solid.
 */
void CollisionSphere::
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
      vertex.add_data3(compute_point(latitude, longitude0));
      vertex.add_data3(compute_point(latitude, longitude1));
    }
    vertex.add_data3(compute_point(1.0, longitude0));

    strip->add_next_vertices(num_stacks * 2);
    strip->close_primitive();
  }

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(strip);

  _viz_geom->add_geom(geom, get_solid_viz_state());
  _bounds_viz_geom->add_geom(geom, get_solid_bounds_viz_state());
}

/**
 * Determine the point(s) of intersection of a parametric line with the
 * sphere.  The line is infinite in both directions, and passes through "from"
 * and from+delta.  If the line does not intersect the sphere, the function
 * returns false, and t1 and t2 are undefined.  If it does intersect the
 * sphere, it returns true, and t1 and t2 are set to the points along the
 * equation from+t*delta that correspond to the two points of intersection.
 */
bool CollisionSphere::
intersects_line(double &t1, double &t2,
                const LPoint3 &from, const LVector3 &delta,
                PN_stdfloat inflate_radius) const {
  // Solve the equation for the intersection of a line with a sphere using the
  // quadratic equation.

  // A line segment from f to f+d is defined as all P such that P = f + td for
  // 0 <= t <= 1.

  // A sphere with radius r about point c is defined as all P such that r^2 =
  // (P - c)^2.

  // Substituting P in the above we have:

  // r^2 = (f + td - c)^2 = (f^2 + ftd - fc + ftd + t^2d^2 - tdc - fc - tdc +
  // c^2) = t^2(d^2) + t(fd + fd - dc - dc) + (f^2 - fc - fc + c^2) = t^2(d^2)
  // + t(2d(f - c)) + (f - c)^2

  // Thus, the equation is quadratic in t, and we have at^2 + bt + c = 0

  // Where  a = d^2 b = 2d(f - c) c = (f - c)^2 - r^2

  // Solving for t using the quadratic equation gives us the point of
  // intersection along the line segment.  Actually, there are two solutions
  // (since it is quadratic): one for the front of the sphere, and one for the
  // back.  In the case where the line is tangent to the sphere, there is only
  // one solution (and the radical is zero).

  double A = dot(delta, delta);

  LVector3 fc = from - get_center();
  double fc_d2 = dot(fc, fc);
  double radius = get_radius() + inflate_radius;
  double C = fc_d2 - radius * radius;

  if (A == 0.0) {
    // Degenerate case where delta is zero.  This is effectively a test
    // against a point (or sphere, for nonzero inflate_radius).
    t1 = 0.0;
    t2 = 0.0;
    return C < 0.0;
  }

  double B = 2.0f * dot(delta, fc);
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

/**
 * Determine a point of intersection of a parametric parabola with the sphere.
 *
 * We only consider the segment of the parabola between t1 and t2, which has
 * already been computed as corresponding to points p1 and p2.  If there is an
 * intersection, t is set to the parametric point of intersection, and true is
 * returned; otherwise, false is returned.
 */
bool CollisionSphere::
intersects_parabola(double &t, const LParabola &parabola,
                    double t1, double t2,
                    const LPoint3 &p1, const LPoint3 &p2) const {
  if (t1 == t2) {
    // Special case: a single point.
    if ((p1 - _center).length_squared() > _radius * _radius) {
      // No intersection.
      return false;
    }
    t = t1;
    return true;
  }

  // To directly test for intersection between a parabola (quadratic) and a
  // sphere (also quadratic) requires solving a quartic equation.  Doable, but
  // hard, and I'm a programmer, not a mathematician.  So I'll solve it the
  // programmer's way instead, by approximating the parabola with a series of
  // line segments.  Hence, this function works by recursively subdividing the
  // parabola as necessary.

  // First, see if the line segment (p1 - p2) comes sufficiently close to the
  // parabola.  Do this by computing the parametric intervening point and
  // comparing its distance from the linear intervening point.
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

/**
 * Returns a point on the surface of the sphere.  latitude and longitude range
 * from 0.0 to 1.0.  This is used by fill_viz_geom() to create a visible
 * representation of the sphere.
 */
LVertex CollisionSphere::
compute_point(PN_stdfloat latitude, PN_stdfloat longitude) const {
  PN_stdfloat s1, c1;
  csincos(latitude * MathNumbers::pi, &s1, &c1);

  PN_stdfloat s2, c2;
  csincos(longitude * 2.0f * MathNumbers::pi, &s2, &c2);

  LVertex p(s1 * c2, s1 * s2, c1);
  return p * get_radius() + get_center();
}

/**
 * Factory method to generate a CollisionSphere object
 */
void CollisionSphere::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_CollisionSphere);
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void CollisionSphere::
write_datagram(BamWriter *manager, Datagram &me) {
  CollisionSolid::write_datagram(manager, me);
  _center.write_datagram(me);
  me.add_stdfloat(_radius);
}

/**
 * Factory method to generate a CollisionSphere object
 */
TypedWritable *CollisionSphere::
make_CollisionSphere(const FactoryParams &params) {
  CollisionSphere *me = new CollisionSphere;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void CollisionSphere::
fillin(DatagramIterator& scan, BamReader* manager) {
  CollisionSolid::fillin(scan, manager);
  _center.read_datagram(scan);
  _radius = scan.get_stdfloat();
}
