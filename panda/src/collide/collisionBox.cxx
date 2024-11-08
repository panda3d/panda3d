/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionBox.cxx
 * @author amith tudur
 * @date 2009-07-31
 */

#include "collisionBox.h"
#include "collisionLine.h"
#include "collisionRay.h"
#include "collisionSphere.h"
#include "collisionSegment.h"
#include "collisionParabola.h"
#include "collisionCapsule.h"
#include "collisionHandler.h"
#include "collisionEntry.h"
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
#include "geomLines.h"
#include "geomTriangles.h"
#include "geomVertexWriter.h"
#include "config_mathutil.h"
#include "dcast.h"

#include <math.h>

using std::max;
using std::min;

PStatCollector CollisionBox::_volume_pcollector("Collision Volumes:CollisionBox");
PStatCollector CollisionBox::_test_pcollector("Collision Tests:CollisionBox");
TypeHandle CollisionBox::_type_handle;

const int CollisionBox::plane_def[6][4] = {
  {0, 4, 5, 1},
  {4, 6, 7, 5},
  {6, 2, 3, 7},
  {2, 0, 1, 3},
  {1, 5, 7, 3},
  {2, 6, 4, 0},
};

/**
 * Helper function to calculate the intersection between a line segment and a
 * sphere.  t is filled with the first position along the line segment where
 * the intersection hits.
 */
static bool
intersect_segment_sphere(double &t,
                         const LPoint3 &from, const LVector3 &delta,
                         const LPoint3 &center, double radius_sq) {
  double A2 = dot(delta, delta) * 2;

  LVector3 fc = from - center;
  double fc_d2 = dot(fc, fc);
  double C = fc_d2 - radius_sq;

  if (UNLIKELY(A2 == 0.0)) {
    // Degenerate case where delta is zero.  This is effectively a test
    // against a point (or sphere, for nonzero inflate_radius).
    t = 0.0;
    return C < 0.0;
  }

  double B = 2.0f * dot(delta, fc);
  double radical = B*B - 2.0*A2*C;

  if (radical < 0.0) {
    // No real roots: no intersection with the line.
    return false;
  }

  t = (-B - csqrt(radical)) / A2;
  return true;
}

/**
 * Helper function to calculate the intersection between a line segment and a
 * capsule.  t is filled with the first position along the line segment where
 * the intersection hits.
 *
 * Code derived from a book by Christer Ericson.
 */
static bool
intersect_segment_capsule(double &t,
                          const LPoint3 &from_a, const LVector3 &delta_a,
                          const LPoint3 &from_b, const LPoint3 &to_b,
                          double radius_sq) {
  LVector3 m = from_a - from_b;
  LVector3 delta_b = to_b - from_b;
  PN_stdfloat md = m.dot(delta_b);
  PN_stdfloat nd = delta_a.dot(delta_b);
  PN_stdfloat dd = delta_b.dot(delta_b);
  if (md < 0 && md + nd < 0) {
    return intersect_segment_sphere(t, from_a, delta_a, from_b, radius_sq);
  }
  if (md > dd && md + nd > dd) {
    return intersect_segment_sphere(t, from_a, delta_a, to_b, radius_sq);
  }
  PN_stdfloat nn = delta_a.dot(delta_a);
  PN_stdfloat mn = m.dot(delta_a);
  PN_stdfloat a = dd * nn - nd * nd;
  PN_stdfloat k = m.dot(m) - radius_sq;
  PN_stdfloat c = dd * k - md * md;
  if (IS_NEARLY_ZERO(a)) {
    // Segments run parallel
    if (c > 0.0f) {
      return false;
    }
    if (md < 0.0f) {
      return intersect_segment_sphere(t, from_a, delta_a, from_b, radius_sq);
    }
    else if (md > dd) {
      return intersect_segment_sphere(t, from_a, delta_a, to_b, radius_sq);
    }
    else {
      t = 0.0;
    }
    return true;
  }
  PN_stdfloat b = dd * mn - nd * md;
  PN_stdfloat discr = b * b - a * c;
  if (discr < 0.0f) {
    return false;
  }
  t = (-b - csqrt(discr)) / a;
  if (t < 0.0 || t > 1.0) {
    return false;
  }
  if (md + t * nd < 0) {
    return intersect_segment_sphere(t, from_a, delta_a, from_b, radius_sq);
  }
  else if (md + t * nd > dd) {
    return intersect_segment_sphere(t, from_a, delta_a, to_b, radius_sq);
  }
  return true;
}

/**
 *
 */
CollisionSolid *CollisionBox::
make_copy() {
  return new CollisionBox(*this);
}

/**
 * First Dispatch point for box as a FROM object
 */
PT(CollisionEntry) CollisionBox::
test_intersection(const CollisionEntry &entry) const {
  return entry.get_into()->test_intersection_from_box(entry);
}

/**
 * Transforms the solid by the indicated matrix.
 */
void CollisionBox::
xform(const LMatrix4 &mat) {
  _min = _min * mat;
  _max = _max * mat;
  _center = _center * mat;
  for(int p = 0; p < 6 ; p++) {
    _planes[p] = set_plane(p);
  }
  mark_viz_stale();
  mark_internal_bounds_stale();
}

/**
 * Returns the point in space deemed to be the "origin" of the solid for
 * collision purposes.  The closest intersection point to this origin point is
 * considered to be the most significant.
 */
LPoint3 CollisionBox::
get_collision_origin() const {
  return _center;
}

/**
 * Returns a PStatCollector that is used to count the number of bounding
 * volume tests made against a solid of this type in a given frame.
 */
PStatCollector &CollisionBox::
get_volume_pcollector() {
  return _volume_pcollector;
}

/**
 * Returns a PStatCollector that is used to count the number of intersection
 * tests made against a solid of this type in a given frame.
 */
PStatCollector &CollisionBox::
get_test_pcollector() {
  return _test_pcollector;
}

/**
 *
 */
void CollisionBox::
output(std::ostream &out) const {
  out << "box, (" << get_min() << ") to (" << get_max() << ")";
}

/**
 * Sphere is chosen as the Bounding Volume type for speed and efficiency
 */
PT(BoundingVolume) CollisionBox::
compute_internal_bounds() const {
  LPoint3 vertex = get_point_aabb(0);
  PN_stdfloat x = vertex.get_x() - _center.get_x();
  PN_stdfloat y = vertex.get_y() - _center.get_y();
  PN_stdfloat z = vertex.get_z() - _center.get_z();
  PN_stdfloat radius = sqrt(x * x + y * y + z * z);
  return new BoundingSphere(_center, radius);
}

/**
 * Double dispatch point for sphere as FROM object
 */
PT(CollisionEntry) CollisionBox::
test_intersection_from_sphere(const CollisionEntry &entry) const {

  const CollisionSphere *sphere;
  DCAST_INTO_R(sphere, entry.get_from(), nullptr);

  CPT(TransformState) wrt_space = entry.get_wrt_space();
  CPT(TransformState) wrt_prev_space = entry.get_wrt_prev_space();

  const LMatrix4 &wrt_mat = wrt_space->get_mat();

  LPoint3 center = wrt_mat.xform_point(sphere->get_center());
  PN_stdfloat radius_sq = wrt_mat.xform_vec(LVector3(0, 0, sphere->get_radius())).length_squared();

  bool had_prev = false;
  LPoint3 prev_center = center;
  double t = 0;

  if (wrt_space != wrt_prev_space) {
    prev_center = wrt_prev_space->get_mat().xform_point(sphere->get_center());
  }
  LPoint3 contact_center = prev_center;

  // First, just test the starting point of the sphere.
  LVector3 vec = (prev_center - _min).fmin(0) + (prev_center - _max).fmax(0);
  PN_stdfloat vec_lsq = vec.length_squared();
  if (vec_lsq > radius_sq) {
    if (wrt_space == wrt_prev_space) {
      return nullptr;
    }

    // We must effectively do a capsule-into-box test.
    LVector3 delta = center - prev_center;
    if (!intersects_capsule(t, prev_center, delta, radius_sq)) {
      return nullptr;
    }
    contact_center = prev_center + delta * t;

    // This is used to calculate the surface normal, which must always be
    // opposed to the movement direction!
    vec = (contact_center - _min).fmin(0) + (contact_center - _max).fmax(0);
    if ((vec[0] > 0) == (delta[0] > 0)) vec[0] = 0;
    if ((vec[1] > 0) == (delta[1] > 0)) vec[1] = 0;
    if ((vec[2] > 0) == (delta[2] > 0)) vec[2] = 0;

    had_prev = true;
  }
  else if (vec_lsq == 0.0f) {
    // It's completely inside.
    vec = prev_center - _center;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }

  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  int axis;
  if (abs(vec[0]) > abs(vec[1])) {
    if (abs(vec[0]) > abs(vec[2])) {
      axis = 0;
    } else {
      axis = 2;
    }
  } else {
    if (abs(vec[1]) > abs(vec[2])) {
      axis = 1;
    } else {
      axis = 2;
    }
  }

  LPoint3 surface_point = contact_center.fmax(_min).fmin(_max);
  surface_point[axis] = vec[axis] > 0 ? _max[axis] : _min[axis];

  LVector3 normal(0, 0, 0);
  normal[axis] = (vec[axis] > 0) * 2 - 1;

  LPoint3 interior_point = surface_point;
  if (had_prev) {
    interior_point += (center - contact_center);
  } else {
    LVector3 other = surface_point - contact_center;
    other[axis] = 0.0f;
    interior_point[axis] = center[axis] - std::copysign(std::max((PN_stdfloat)0, csqrt(radius_sq - other.length_squared())), vec[axis]);
  }

  new_entry->set_interior_point(interior_point);
  new_entry->set_surface_point(surface_point);
  new_entry->set_surface_normal(
    (has_effective_normal() && sphere->get_respect_effective_normal())
    ? get_effective_normal() : normal);
  new_entry->set_contact_pos(contact_center);
  new_entry->set_contact_normal(normal);
  new_entry->set_t(t);

  return new_entry;
}

/**
 *
 */
PT(CollisionEntry) CollisionBox::
test_intersection_from_line(const CollisionEntry &entry) const {
  const CollisionLine *line;
  DCAST_INTO_R(line, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_origin = line->get_origin() * wrt_mat;
  LVector3 from_direction = line->get_direction() * wrt_mat;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction)) {
    // No intersection.
    return nullptr;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3 point = from_origin + t1 * from_direction;
  new_entry->set_surface_point(point);

  if (has_effective_normal() && line->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3 normal(
      IS_NEARLY_EQUAL(point[0], _max[0]) - IS_NEARLY_EQUAL(point[0], _min[0]),
      IS_NEARLY_EQUAL(point[1], _max[1]) - IS_NEARLY_EQUAL(point[1], _min[1]),
      IS_NEARLY_EQUAL(point[2], _max[2]) - IS_NEARLY_EQUAL(point[2], _min[2])
    );
    normal.normalize();
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

/**
 * Double dispatch point for ray as a FROM object
 */
PT(CollisionEntry) CollisionBox::
test_intersection_from_ray(const CollisionEntry &entry) const {
  const CollisionRay *ray;
  DCAST_INTO_R(ray, entry.get_from(), nullptr);
  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_origin = ray->get_origin() * wrt_mat;
  LVector3 from_direction = ray->get_direction() * wrt_mat;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction) || (t1 < 0.0 && t2 < 0.0)) {
    // No intersection.
    return nullptr;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  if (t1 < 0.0) {
    // The origin is inside the box, so we take the exit as our surface point.
    new_entry->set_interior_point(from_origin);
    t1 = t2;
  }

  LPoint3 point = from_origin + t1 * from_direction;
  new_entry->set_surface_point(point);

  if (has_effective_normal() && ray->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3 normal(
      IS_NEARLY_EQUAL(point[0], _max[0]) - IS_NEARLY_EQUAL(point[0], _min[0]),
      IS_NEARLY_EQUAL(point[1], _max[1]) - IS_NEARLY_EQUAL(point[1], _min[1]),
      IS_NEARLY_EQUAL(point[2], _max[2]) - IS_NEARLY_EQUAL(point[2], _min[2])
    );
    normal.normalize();
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

PT(CollisionEntry) CollisionBox::
test_intersection_from_parabola(const CollisionEntry &entry) const {
  const CollisionParabola *parabola;
  DCAST_INTO_R(parabola, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  // Convert the parabola into local coordinate space.
  LParabola local_p(parabola->get_parabola());
  local_p.xform(wrt_mat);

  PN_stdfloat t = FLT_MAX;
  PN_stdfloat t1, t2;
  int intersecting_face = -1;
  for (int i = 0; i < get_num_planes(); i++) {
    LPlane face = get_plane(i);
    if (!face.intersects_parabola(t1, t2, local_p)) {
      // the parabola does not intersect this face, skip to the next one
      continue;
    }
    PN_stdfloat ts[2] = {t1, t2};
    // iterate through the t values to see if each of them are within our
    // parabola and the intersection point is behind all other faces
    for (int j = 0; j < 2; j++) {
      PN_stdfloat cur_t = ts[j];
      if (cur_t > t) {
        // we are looking for the earliest t value
        // if this t value is greater, don't bother checking it
        continue;
      }
      if (cur_t >= parabola->get_t1() && cur_t <= parabola->get_t2()) {
        // the parabola does intersect this plane, now we check
        // if the intersection point is behind all other planes
        bool behind = true;
        for (int k = 0; k < get_num_planes(); k++) {
          if (k == i) {
            // no need to check the intersecting face
            continue;
          }
          if (get_plane(k).dist_to_plane(local_p.calc_point(cur_t)) > 0.0f) {
            // our point is in front of this face, turns out the parabola
            // does not collide with the box at this point
            behind = false;
            break;
          }
        }
        if (behind) {
          // the parabola does indeed collide with the box at this point
          t = cur_t;
          intersecting_face = i;
        }
      }
    }
  }

  if (intersecting_face != -1) {
    if (collide_cat.is_debug()) {
      collide_cat.debug()
        << "intersection detected from " << entry.get_from_node_path()
        << " into " << entry.get_into_node_path() << "\n";
    }
    PT(CollisionEntry) new_entry = new CollisionEntry(entry);

    LPlane face = get_plane(intersecting_face);

    LPoint3 into_intersection_point = local_p.calc_point(t);
    LVector3 normal = (has_effective_normal() && parabola->get_respect_effective_normal()) ? get_effective_normal() : face.get_normal();

    new_entry->set_surface_point(into_intersection_point);
    new_entry->set_surface_normal(normal);
    return new_entry;
  } else {
    return nullptr;
  }
}

/**
 * Double dispatch point for segment as a FROM object
 */
PT(CollisionEntry) CollisionBox::
test_intersection_from_segment(const CollisionEntry &entry) const {
  const CollisionSegment *seg;
  DCAST_INTO_R(seg, entry.get_from(), nullptr);
  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_origin = seg->get_point_a() * wrt_mat;
  LPoint3 from_extent = seg->get_point_b() * wrt_mat;
  LVector3 from_direction = from_extent - from_origin;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction) ||
      (t1 < 0.0 && t2 < 0.0) || (t1 > 1.0 && t2 > 1.0)) {
    // No intersection.
    return nullptr;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  // In case the segment is entirely inside the cube, we consider the point
  // closest to the surface as our entry point.
  if (t1 < (1.0 - t2)) {
    std::swap(t1, t2);
  }

  // Our interior point is the closest point to t2 that is inside the segment.
  new_entry->set_interior_point(from_origin + std::min(std::max(t2, 0.0), 1.0) * from_direction);

  LPoint3 point = from_origin + t1 * from_direction;
  new_entry->set_surface_point(point);

  if (has_effective_normal() && seg->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3 normal(
      IS_NEARLY_EQUAL(point[0], _max[0]) - IS_NEARLY_EQUAL(point[0], _min[0]),
      IS_NEARLY_EQUAL(point[1], _max[1]) - IS_NEARLY_EQUAL(point[1], _min[1]),
      IS_NEARLY_EQUAL(point[2], _max[2]) - IS_NEARLY_EQUAL(point[2], _min[2])
    );
    normal.normalize();
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

/**
 * Double dispatch point for capsule as a FROM object
 */
PT(CollisionEntry) CollisionBox::
test_intersection_from_capsule(const CollisionEntry &entry) const {
  const CollisionCapsule *capsule;
  DCAST_INTO_R(capsule, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_a = capsule->get_point_a() * wrt_mat;
  LPoint3 from_b = capsule->get_point_b() * wrt_mat;
  LVector3 from_direction = from_b - from_a;
  PN_stdfloat radius_sq = wrt_mat.xform_vec(LVector3(0, 0, capsule->get_radius())).length_squared();
  PN_stdfloat radius = csqrt(radius_sq);

  LPoint3 box_min = get_min();
  LPoint3 box_max = get_max();
  LVector3 dimensions = box_max - box_min;

  // The method below is inspired by Christer Ericson's book Real-Time
  // Collision Detection.  Instead of testing a capsule against a box, we test
  // a segment against an box that is oversized by the capsule radius.

  // First, we test if the line segment intersects a box with its faces
  // expanded outwards by the capsule radius.  If not, there is no collision.
  double t1, t2;
  if (!intersects_line(t1, t2, from_a, from_direction, radius)) {
    return nullptr;
  }

  if (t2 < 0.0 || t1 > 1.0) {
    return nullptr;
  }

  t1 = std::min(1.0, std::max(0.0, (t1 + t2) * 0.5));
  LPoint3 point = from_a + from_direction * t1;

  // We now have a point of intersection between the line segment and the
  // oversized box.  Check on how many axes it lies outside the box.  If it is
  // less than two, we know that it does not lie in one of the rounded regions
  // of the oversized rounded box, and it is a guaranteed hit.  Otherwise, we
  // will need to test against the edge regions.
  if ((point[0] < box_min[0] || point[0] > box_max[0]) +
      (point[1] < box_min[1] || point[1] > box_max[1]) +
      (point[2] < box_min[2] || point[2] > box_max[2]) > 1) {
    // Test the capsule against each edge of the box.
    static const struct {
      LPoint3 point;
      int axis;
    } edges[] = {
      {{0, 0, 0}, 0},
      {{0, 1, 0}, 0},
      {{0, 0, 1}, 0},
      {{0, 1, 1}, 0},
      {{0, 0, 0}, 1},
      {{0, 0, 1}, 1},
      {{1, 0, 0}, 1},
      {{1, 0, 1}, 1},
      {{0, 0, 0}, 2},
      {{0, 1, 0}, 2},
      {{1, 0, 0}, 2},
      {{1, 1, 0}, 2},
    };

    PN_stdfloat best_dist_sq = FLT_MAX;

    for (int i = 0; i < 12; ++i) {
      LPoint3 vertex = edges[i].point;
      vertex.componentwise_mult(dimensions);
      vertex += box_min;
      LVector3 delta(0);
      delta[edges[i].axis] = dimensions[edges[i].axis];
      double u1, u2;
      CollisionCapsule::calc_closest_segment_points(u1, u2, from_a, from_direction, vertex, delta);
      PN_stdfloat dist_sq = ((from_a + from_direction * u1) - (vertex + delta * u2)).length_squared();
      if (dist_sq < best_dist_sq) {
        best_dist_sq = dist_sq;
      }
    }

    if (best_dist_sq > radius_sq) {
      // It is not actually touching any edge.
      return nullptr;
    }
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  // Which is the longest axis?
  LVector3 diff = point - _center;
  diff[0] /= dimensions[0];
  diff[1] /= dimensions[1];
  diff[2] /= dimensions[2];
  int axis = 0;
  if (cabs(diff[0]) > cabs(diff[1])) {
    if (cabs(diff[0]) > cabs(diff[2])) {
      axis = 0;
    } else {
      axis = 2;
    }
  } else {
    if (cabs(diff[1]) > cabs(diff[2])) {
      axis = 1;
    } else {
      axis = 2;
    }
  }
  LVector3 normal(0);
  normal[axis] = std::copysign(1, diff[axis]);

  LPoint3 clamped = point.fmax(box_min).fmin(box_max);
  LPoint3 surface_point = clamped;
  surface_point[axis] = (diff[axis] >= 0.0f) ? box_max[axis] : box_min[axis];

  // Is the point inside the box?
  LVector3 interior_vec;
  if (clamped != point) {
    // No, it is outside.  The interior point is in the direction of the
    // surface point.
    interior_vec = point - surface_point;
    if (!interior_vec.normalize()) {
      interior_vec = normal;
    }
  } else {
    // It is inside.  I think any point will work for this.
    interior_vec = normal;
  }
  new_entry->set_interior_point(point - interior_vec * radius);
  new_entry->set_surface_point(surface_point);

  if (has_effective_normal() && capsule->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

/**
 * Double dispatch point for box as a FROM object
 */
PT(CollisionEntry) CollisionBox::
test_intersection_from_box(const CollisionEntry &entry) const {
  const CollisionBox *box;
  DCAST_INTO_R(box, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 diff = wrt_mat.xform_point_general(box->get_center()) - _center;
  LVector3 from_extents = box->get_dimensions() * 0.5f;
  LVector3 into_extents = get_dimensions() * 0.5f;

  LVecBase3 box_x = wrt_mat.get_row3(0);
  LVecBase3 box_y = wrt_mat.get_row3(1);
  LVecBase3 box_z = wrt_mat.get_row3(2);

  // To make the math simpler, normalize the box basis vectors, instead
  // applying the scale to the box dimensions.  Note that this doesn't work
  // for a non-uniform scales applied after a rotation, since that has the
  // possibility of making the box no longer a box.
  PN_stdfloat l;
  l = box_x.length();
  from_extents[0] *= l;
  box_x /= l;
  l = box_y.length();
  from_extents[1] *= l;
  box_y /= l;
  l = box_z.length();
  from_extents[2] *= l;
  box_z /= l;

  PN_stdfloat r1, r2;
  PN_stdfloat min_pen = 0;
  PN_stdfloat pen;
  int axis = 0;

  // SAT test for the three axes of the into cube.
  r1 = into_extents[0];
  r2 = cabs(box_x[0] * from_extents[0]) +
       cabs(box_y[0] * from_extents[1]) +
       cabs(box_z[0] * from_extents[2]);
  pen = r1 + r2 - cabs(diff[0]);
  if (pen < 0) {
    return nullptr;
  }
  min_pen = pen;

  r1 = into_extents[1];
  r2 = cabs(box_x[1] * from_extents[0]) +
       cabs(box_y[1] * from_extents[1]) +
       cabs(box_z[1] * from_extents[2]);
  pen = r1 + r2 - cabs(diff[1]);
  if (pen < 0) {
    return nullptr;
  }
  if (pen < min_pen) {
    min_pen = pen;
    axis = 1;
  }

  r1 = into_extents[2];
  r2 = cabs(box_x[2] * from_extents[0]) +
       cabs(box_y[2] * from_extents[1]) +
       cabs(box_z[2] * from_extents[2]);
  pen = r1 + r2 - cabs(diff[2]);
  if (pen < 0) {
    return nullptr;
  }
  if (pen < min_pen) {
    min_pen = pen;
    axis = 2;
  }

  // SAT test for the three axes of the from cube.
  r1 = cabs(box_x[0] * into_extents[0]) +
       cabs(box_x[1] * into_extents[1]) +
       cabs(box_x[2] * into_extents[2]);
  r2 = from_extents[0];
  pen = r1 + r2 - cabs(diff.dot(box_x));
  if (pen < 0) {
    return nullptr;
  }
  if (pen < min_pen) {
    min_pen = pen;
  }

  r1 = cabs(box_y[0] * into_extents[0]) +
       cabs(box_y[1] * into_extents[1]) +
       cabs(box_y[2] * into_extents[2]);
  r2 = from_extents[1];
  pen = r1 + r2 - cabs(diff.dot(box_y));
  if (pen < 0) {
    return nullptr;
  }
  if (pen < min_pen) {
    min_pen = pen;
  }

  r1 = cabs(box_z[0] * into_extents[0]) +
       cabs(box_z[1] * into_extents[1]) +
       cabs(box_z[2] * into_extents[2]);
  r2 = from_extents[2];
  pen = r1 + r2 - cabs(diff.dot(box_z));
  if (pen < 0) {
    return nullptr;
  }
  if (pen < min_pen) {
    min_pen = pen;
  }

  // SAT test of the nine cross products.
  r1 = into_extents[1] * cabs(box_x[2]) + into_extents[2] * cabs(box_x[1]);
  r2 = from_extents[1] * cabs(box_z[0]) + from_extents[2] * cabs(box_y[0]);
  if (cabs(diff[2] * box_x[1] - diff[1] * box_x[2]) > r1 + r2) {
      return nullptr;
  }

  r1 = into_extents[1] * cabs(box_y[2]) + into_extents[2] * cabs(box_y[1]);
  r2 = from_extents[0] * cabs(box_z[0]) + from_extents[2] * cabs(box_x[0]);
  if (cabs(diff[2] * box_y[1] - diff[1] * box_y[2]) > r1 + r2) {
      return nullptr;
  }

  r1 = into_extents[1] * cabs(box_z[2]) + into_extents[2] * cabs(box_z[1]);
  r2 = from_extents[0] * cabs(box_y[0]) + from_extents[1] * cabs(box_x[0]);
  if (cabs(diff[2] * box_z[1] - diff[1] * box_z[2]) > r1 + r2) {
      return nullptr;
  }

  r1 = into_extents[0] * cabs(box_x[2]) + into_extents[2] * cabs(box_x[0]);
  r2 = from_extents[1] * cabs(box_z[1]) + from_extents[2] * cabs(box_y[1]);
  if (cabs(diff[0] * box_x[2] - diff[2] * box_x[0]) > r1 + r2) {
      return nullptr;
  }

  r1 = into_extents[0] * cabs(box_y[2]) + into_extents[2] * cabs(box_y[0]);
  r2 = from_extents[0] * cabs(box_z[1]) + from_extents[2] * cabs(box_x[1]);
  if (cabs(diff[0] * box_y[2] - diff[2] * box_y[0]) > r1 + r2) {
      return nullptr;
  }

  r1 = into_extents[0] * cabs(box_z[2]) + into_extents[2] * cabs(box_z[0]);
  r2 = from_extents[0] * cabs(box_y[1]) + from_extents[1] * cabs(box_x[1]);
  if (cabs(diff[0] * box_z[2] - diff[2] * box_z[0]) > r1 + r2) {
      return nullptr;
  }

  r1 = into_extents[0] * cabs(box_x[1]) + into_extents[1] * cabs(box_x[0]);
  r2 = from_extents[1] * cabs(box_z[2]) + from_extents[2] * cabs(box_y[2]);
  if (cabs(diff[1] * box_x[0] - diff[0] * box_x[1]) > r1 + r2) {
      return nullptr;
  }

  r1 = into_extents[0] * cabs(box_y[1]) + into_extents[1] * cabs(box_y[0]);
  r2 = from_extents[0] * cabs(box_z[2]) + from_extents[2] * cabs(box_x[2]);
  if (cabs(diff[1] * box_y[0] - diff[0] * box_y[1]) > r1 + r2) {
      return nullptr;
  }

  r1 = into_extents[0] * cabs(box_z[1]) + into_extents[1] * cabs(box_z[0]);
  r2 = from_extents[0] * cabs(box_y[2]) + from_extents[1] * cabs(box_x[2]);
  if (cabs(diff[1] * box_z[0] - diff[0] * box_z[1]) > r1 + r2) {
      return nullptr;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  // This isn't always the correct surface point.  However, it seems to be
  // enough to let the pusher do the right thing.
  LPoint3 surface(
    min(max(diff[0], -into_extents[0]), into_extents[0]),
    min(max(diff[1], -into_extents[1]), into_extents[1]),
    min(max(diff[2], -into_extents[2]), into_extents[2]));

  // Create the normal along the axis of least penetration.
  LVector3 normal(0);
  PN_stdfloat diff_axis = diff[axis];
  int sign = (diff_axis >= 0) ? 1 : -1;
  normal[axis] = sign;
  surface[axis] = into_extents[axis] * sign;

  new_entry->set_surface_point(surface + _center);

  // Does not generate the correct depth.  Needs fixing.
  new_entry->set_interior_point(surface + _center + normal * -min_pen);

  if (has_effective_normal() && box->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

/**
 * Fills the _viz_geom GeomNode up with Geoms suitable for rendering this
 * solid.
 */
void CollisionBox::
fill_viz_geom() {
  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "Recomputing viz for " << *this << "\n";
  }

  PT(GeomVertexData) vdata = new GeomVertexData
    ("collision", GeomVertexFormat::get_v3(),
     Geom::UH_static);

  vdata->unclean_set_num_rows(8);

  {
    GeomVertexWriter vertex(vdata, InternalName::get_vertex());
    vertex.set_data3(_min[0], _min[1], _min[2]);
    vertex.set_data3(_min[0], _max[1], _min[2]);
    vertex.set_data3(_max[0], _max[1], _min[2]);
    vertex.set_data3(_max[0], _min[1], _min[2]);

    vertex.set_data3(_min[0], _min[1], _max[2]);
    vertex.set_data3(_min[0], _max[1], _max[2]);
    vertex.set_data3(_max[0], _max[1], _max[2]);
    vertex.set_data3(_max[0], _min[1], _max[2]);
  }

  PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);

  // Bottom
  tris->add_vertices(0, 1, 2);
  tris->add_vertices(2, 3, 0);

  // Top
  tris->add_vertices(4, 7, 6);
  tris->add_vertices(6, 5, 4);

  // Sides
  tris->add_vertices(0, 4, 1);
  tris->add_vertices(1, 4, 5);

  tris->add_vertices(1, 5, 2);
  tris->add_vertices(2, 5, 6);

  tris->add_vertices(2, 6, 3);
  tris->add_vertices(3, 6, 7);

  tris->add_vertices(3, 7, 0);
  tris->add_vertices(0, 7, 4);

  PT(GeomLines) lines = new GeomLines(Geom::UH_static);

  // Bottom
  lines->add_vertices(0, 1);
  lines->add_vertices(1, 2);
  lines->add_vertices(0, 3);
  lines->add_vertices(2, 3);

  // Top
  lines->add_vertices(4, 5);
  lines->add_vertices(5, 6);
  lines->add_vertices(4, 7);
  lines->add_vertices(6, 7);

  // Sides
  lines->add_vertices(0, 4);
  lines->add_vertices(1, 5);
  lines->add_vertices(2, 6);
  lines->add_vertices(3, 7);

  PT(Geom) geom1 = new Geom(vdata);
  geom1->add_primitive(tris);

  PT(Geom) geom2 = new Geom(vdata);
  geom2->add_primitive(lines);

  _viz_geom->add_geom(geom1, get_solid_viz_state());
  _viz_geom->add_geom(geom2, get_wireframe_viz_state());

  _bounds_viz_geom->add_geom(geom1, get_solid_bounds_viz_state());
  _bounds_viz_geom->add_geom(geom2, get_wireframe_viz_state());
}

/**
 * Determine the point(s) of intersection of a parametric line with the box.
 * The line is infinite in both directions, and passes through "from" and
 * from+delta.  If the line does not intersect the box, the function returns
 * false, and t1 and t2 are undefined.  If it does intersect the box, it
 * returns true, and t1 and t2 are set to the points along the equation
 * from+t*delta that correspond to the two points of intersection.
 */
bool CollisionBox::
intersects_line(double &t1, double &t2,
                const LPoint3 &from, const LVector3 &delta,
                PN_stdfloat inflate_size) const {

  LPoint3 bmin = _min - LVector3(inflate_size);
  LPoint3 bmax = _max + LVector3(inflate_size);

  double tmin = -DBL_MAX;
  double tmax = DBL_MAX;

  for (int i = 0; i < 3; ++i) {
    PN_stdfloat d = delta[i];
    if (!IS_NEARLY_ZERO(d)) {
      double tmin2 = (bmin[i] - from[i]) / d;
      double tmax2 = (bmax[i] - from[i]) / d;
      if (tmin2 > tmax2) {
        std::swap(tmin2, tmax2);
      }
      tmin = std::max(tmin, tmin2);
      tmax = std::min(tmax, tmax2);

      if (tmin > tmax) {
        return false;
      }

    } else if (from[i] < bmin[i] || from[i] > bmax[i]) {
      // The line is entirely parallel in this dimension.
      return false;
    }
  }

  t1 = tmin;
  t2 = tmax;
  return true;
}

/**
 * Determine the first point of intersection of the given capsule with the box.
 */
bool CollisionBox::
intersects_capsule(double &t, const LPoint3 &from, const LVector3 &delta,
                   PN_stdfloat radius_sq) const {
  // First, we check whether the line segment intersects with the box
  // expanded by the sphere's radius.
  double t2;
  if (!intersects_line(t, t2, from, delta, csqrt(radius_sq)) ||
      t > 1.0 || t2 < 0.0) {
    return false;
  }

  LPoint3 intersection = from + delta * t;

  // The following technique is derived from a book by Christer Ericson.
  int u = 0, v = 0;
  if (intersection[0] < _min[0]) u |= 4;
  if (intersection[1] < _min[1]) u |= 2;
  if (intersection[2] < _min[2]) u |= 1;
  if (intersection[0] > _max[0]) v |= 4;
  if (intersection[1] > _max[1]) v |= 2;
  if (intersection[2] > _max[2]) v |= 1;

  int m = u | v;
  if (m == 7) {
    double tmin = DBL_MAX;
    LPoint3 vertex = get_point_aabb(v);
    if (intersect_segment_capsule(t, from, delta, vertex,
                                  get_point_aabb(v ^ 4), radius_sq)) {
      tmin = std::min(t, tmin);
    }
    if (intersect_segment_capsule(t, from, delta, vertex,
                                  get_point_aabb(v ^ 2), radius_sq)) {
      tmin = std::min(t, tmin);
    }
    if (intersect_segment_capsule(t, from, delta, vertex,
                                  get_point_aabb(v ^ 1), radius_sq)) {
      tmin = std::min(t, tmin);
    }
    if (tmin == DBL_MAX) {
      return false;
    }
    t = tmin;
  }
  else if ((m & (m - 1)) != 0) {
    // There's just one edge to test.
    LPoint3 edge_v1 = get_point_aabb(u ^ 7);
    LPoint3 edge_v2 = get_point_aabb(v);
    return intersect_segment_capsule(t, from, delta, edge_v1, edge_v2, radius_sq);
  }

  return true;
}

/**
 * Factory method to generate a CollisionBox object
 */
void CollisionBox::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void CollisionBox::
write_datagram(BamWriter *manager, Datagram &me) {
  CollisionSolid::write_datagram(manager, me);
  _center.write_datagram(me);
  _min.write_datagram(me);
  _max.write_datagram(me);
  for (int i = 0; i < 8; ++i) {
    get_point_aabb(i).write_datagram(me);
  }
  LPoint3 vertex = get_point_aabb(0);
  PN_stdfloat x = vertex.get_x() - _center.get_x();
  PN_stdfloat y = vertex.get_y() - _center.get_y();
  PN_stdfloat z = vertex.get_z() - _center.get_z();
  PN_stdfloat radius = sqrt(x * x + y * y + z * z);
  me.add_stdfloat(radius);
  me.add_stdfloat(x);
  me.add_stdfloat(y);
  me.add_stdfloat(z);
  for (int i = 0; i < 6; ++i) {
    _planes[i].write_datagram(me);
  }
  LMatrix4 to_2d_mat[6];
  for (int i = 0; i < 6; ++i) {
    LMatrix4 to_3d_mat;
    look_at(to_3d_mat, -get_plane(i).get_normal(),
            LVector3(0.0f, 0.0f, 1.0f), CS_zup_right);
    to_3d_mat.set_row(3, get_plane(i).get_point());

    to_2d_mat[i].invert_from(to_3d_mat);
    to_2d_mat[i].write_datagram(me);
  }
  for (int i = 0; i < 6; ++i) {
    me.add_uint16(4);
    LPoint2 points[4];
    for (size_t j = 0; j < 4; j++) {
      points[j] = (get_point(plane_def[i][j]) * to_2d_mat[i]).get_xz();
    }
    for (size_t j = 0; j < 4; j++) {
      LPoint2 vec = points[(i + 1) % 4] - points[i];
      vec.normalize();
      points[j].write_datagram(me);
      vec.write_datagram(me);
    }
  }
}

/**
 * Factory method to generate a CollisionBox object
 */
TypedWritable *CollisionBox::
make_from_bam(const FactoryParams &params) {
  CollisionBox *me = new CollisionBox;
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
void CollisionBox::
fillin(DatagramIterator& scan, BamReader* manager) {
  CollisionSolid::fillin(scan, manager);
  _center.read_datagram(scan);
  _min.read_datagram(scan);
  _max.read_datagram(scan);
  for (int i = 0; i < 8; ++i) {
    LPoint3 vertex;
    vertex.read_datagram(scan);
  }
  scan.get_stdfloat();
  scan.get_stdfloat();
  scan.get_stdfloat();
  scan.get_stdfloat();
  for (int i = 0; i < 6; ++i) {
    _planes[i].read_datagram(scan);
  }
  for (int i = 0; i < 6; ++i) {
    LMatrix4 to_2d_mat;
    to_2d_mat.read_datagram(scan);
  }
  for (int i = 0; i < 6; ++i) {
    size_t size = scan.get_uint16();
    for (size_t j = 0; j < size; ++j) {
      LPoint2 p;
      LVector2 v;
      p.read_datagram(scan);
      v.read_datagram(scan);
    }
  }
}
