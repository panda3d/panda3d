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
 *
 */
CollisionSolid *CollisionBox::
make_copy() {
  return new CollisionBox(*this);
}

/**
 * Compute parameters for each of the box's sides
 */
void CollisionBox::
setup_box(){
  for(int plane = 0; plane < 6; plane++) {
    LPoint3 array[4];
    array[0] = get_point(plane_def[plane][0]);
    array[1] = get_point(plane_def[plane][1]);
    array[2] = get_point(plane_def[plane][2]);
    array[3] = get_point(plane_def[plane][3]);
    setup_points(array, array+4, plane);
  }
}

/**
 * Computes the plane and 2d projection of points that make up this side.
 */
void CollisionBox::
setup_points(const LPoint3 *begin, const LPoint3 *end, int plane) {
  int num_points = end - begin;
  nassertv(num_points >= 3);

  _points[plane].clear();

  // Construct a matrix that rotates the points from the (X,0,Z) plane into
  // the 3-d plane.
  LMatrix4 to_3d_mat;
  calc_to_3d_mat(to_3d_mat, plane);

  // And the inverse matrix rotates points from 3-d space into the 2-d plane.
  _to_2d_mat[plane].invert_from(to_3d_mat);

  // Now project all of the points onto the 2-d plane.

  const LPoint3 *pi;
  for (pi = begin; pi != end; ++pi) {
    LPoint3 point = (*pi) * _to_2d_mat[plane];
    _points[plane].push_back(PointDef(point[0], point[2]));
  }

  nassertv(_points[plane].size() >= 3);

#ifndef NDEBUG
  /*
  // Now make sure the points define a convex polygon.
  if (is_concave()) {
  collide_cat.error() << "Invalid concave CollisionPolygon defined:\n";
  const LPoint3 *pi;
  for (pi = begin; pi != end; ++pi) {
  collide_cat.error(false) << "  " << (*pi) << "\n";
  }
  collide_cat.error(false)
  << "  normal " << normal << " with length " << normal.length() << "\n";
  _points.clear();
  }
  */
#endif

  compute_vectors(_points[plane]);
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
  for(int v = 0; v < 8; v++) {
    _vertex[v] = _vertex[v] * mat;
  }
  for(int p = 0; p < 6 ; p++) {
    _planes[p] = set_plane(p);
  }
  _x = _vertex[0].get_x() - _center.get_x();
  _y = _vertex[0].get_y() - _center.get_y();
  _z = _vertex[0].get_z() - _center.get_z();
  _radius = sqrt(_x * _x + _y * _y + _z * _z);
  setup_box();
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
  return new BoundingSphere(_center, _radius);
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

  LPoint3 orig_center = sphere->get_center() * wrt_mat;
  LPoint3 from_center = orig_center;
  bool moved_from_center = false;
  PN_stdfloat t = 1.0f;
  LPoint3 contact_point(from_center);
  PN_stdfloat actual_t = 1.0f;

  LVector3 from_radius_v =
    LVector3(sphere->get_radius(), 0.0f, 0.0f) * wrt_mat;
  PN_stdfloat from_radius_2 = from_radius_v.length_squared();
  PN_stdfloat from_radius = csqrt(from_radius_2);

  int ip;
  PN_stdfloat max_dist = 0.0;
  PN_stdfloat dist = 0.0;
  bool intersect;
  LPlane plane;
  LVector3 normal;

  for(ip = 0, intersect = false; ip < 6 && !intersect; ip++) {
    plane = get_plane( ip );
    if (_points[ip].size() < 3) {
      continue;
    }
    if (wrt_prev_space != wrt_space) {
      // If we have a delta between the previous position and the current
      // position, we use that to determine some more properties of the
      // collision.
      LPoint3 b = from_center;
      LPoint3 a = sphere->get_center() * wrt_prev_space->get_mat();
      LVector3 delta = b - a;

      // First, there is no collision if the "from" object is definitely
      // moving in the same direction as the plane's normal.
      PN_stdfloat dot = delta.dot(plane.get_normal());
      if (dot > 0.1f) {
        continue; // no intersection
      }

      if (IS_NEARLY_ZERO(dot)) {
        // If we're moving parallel to the plane, the sphere is tested at its
        // final point.  Leave it as it is.

      } else {
/*
 * Otherwise, we're moving into the plane; the sphere is tested at the point
 * along its path that is closest to intersecting the plane.  This may be the
 * actual intersection point, or it may be the starting point or the final
 * point.  dot is equal to the (negative) magnitude of 'delta' along the
 * direction of the plane normal t = ratio of (distance from start pos to
 * plane) to (distance from start pos to end pos), along axis of plane normal
 */
        PN_stdfloat dist_to_p = plane.dist_to_plane(a);
        t = (dist_to_p / -dot);

        // also compute the actual contact point and time of contact for
        // handlers that need it
        actual_t = ((dist_to_p - from_radius) / -dot);
        actual_t = min((PN_stdfloat)1.0, max((PN_stdfloat)0.0, actual_t));
        contact_point = a + (actual_t * delta);

        if (t >= 1.0f) {
          // Leave it where it is.

        } else if (t < 0.0f) {
          from_center = a;
          moved_from_center = true;
        } else {
          from_center = a + t * delta;
          moved_from_center = true;
        }
      }
    }

    normal = (has_effective_normal() && sphere->get_respect_effective_normal()) ? get_effective_normal() : plane.get_normal();

#ifndef NDEBUG
    /*if (!IS_THRESHOLD_EQUAL(normal.length_squared(), 1.0f, 0.001), NULL) {
      std::cout
      << "polygon within " << entry.get_into_node_path()
      << " has normal " << normal << " of length " << normal.length()
      << "\n";
      normal.normalize();
      }*/
#endif

    // The nearest point within the plane to our center is the intersection of
    // the line (center, center - normal) with the plane.

    if (!plane.intersects_line(dist, from_center, -(plane.get_normal()))) {
      // No intersection with plane?  This means the plane's effective normal
      // was within the plane itself.  A useless polygon.
      continue;
    }

    if (dist > from_radius || dist < -from_radius) {
      // No intersection with the plane.
      continue;
    }

    LPoint2 p = to_2d(from_center - dist * plane.get_normal(), ip);
    PN_stdfloat edge_dist = 0.0f;

    const ClipPlaneAttrib *cpa = entry.get_into_clip_planes();
    if (cpa != nullptr) {
      // We have a clip plane; apply it.
      Points new_points;
      if (apply_clip_plane(new_points, cpa, entry.get_into_node_path().get_net_transform(),ip)) {
        // All points are behind the clip plane; just do the default test.
        edge_dist = dist_to_polygon(p, _points[ip]);
      } else if (new_points.empty()) {
        // The polygon is completely clipped.
        continue;
      } else {
        // Test against the clipped polygon.
        edge_dist = dist_to_polygon(p, new_points);
      }
    } else {
      // No clip plane is in effect.  Do the default test.
      edge_dist = dist_to_polygon(p, _points[ip]);
    }

    max_dist = from_radius;

    // Now we have edge_dist, which is the distance from the sphere center to
    // the nearest edge of the polygon, within the polygon's plane.
    // edge_dist<0 means the point is within the polygon.
    if(edge_dist < 0) {
      intersect = true;
      continue;
    }

    if((edge_dist > 0) &&
       ((edge_dist * edge_dist + dist * dist) > from_radius_2)) {
      // No intersection; the circle is outside the polygon.
      continue;
    }

    // The sphere appears to intersect the polygon.  If the edge is less than
    // from_radius away, the sphere may be resting on an edge of the polygon.
    // Determine how far the center of the sphere must remain from the plane,
    // based on its distance from the nearest edge.

    if (edge_dist >= 0.0f) {
      PN_stdfloat max_dist_2 = max(from_radius_2 - edge_dist * edge_dist, (PN_stdfloat)0.0);
      max_dist = csqrt(max_dist_2);
    }

    if (dist > max_dist) {
      // There's no intersection: the sphere is hanging off the edge.
      continue;
    }
    intersect = true;
  }
  if( !intersect )
    return nullptr;

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }

  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  PN_stdfloat into_depth = max_dist - dist;
  if (moved_from_center) {
    // We have to base the depth of intersection on the sphere's final resting
    // point, not the point from which we tested the intersection.
    PN_stdfloat orig_dist;
    plane.intersects_line(orig_dist, orig_center, -normal);
    into_depth = max_dist - orig_dist;
  }

  // Clamp the surface point to the box bounds.
  LPoint3 surface = from_center - normal * dist;
  surface = surface.fmax(_min);
  surface = surface.fmin(_max);

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(surface);
  new_entry->set_interior_point(surface - normal * into_depth);
  new_entry->set_contact_pos(contact_point);
  new_entry->set_contact_normal(plane.get_normal());
  new_entry->set_t(actual_t);

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

  PN_stdfloat t = INT_MAX;
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

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(tris);

  _viz_geom->add_geom(geom, get_solid_viz_state());
  _bounds_viz_geom->add_geom(geom, get_solid_bounds_viz_state());
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
 * Clips the polygon by all of the clip planes named in the clip plane
 * attribute and fills new_points up with the resulting points.
 *
 * The return value is true if the set of points is unmodified (all points are
 * behind all the clip planes), or false otherwise.
 */
bool CollisionBox::
apply_clip_plane(CollisionBox::Points &new_points,
                 const ClipPlaneAttrib *cpa,
                 const TransformState *net_transform, int plane_no) const {
  bool all_in = true;

  int num_planes = cpa->get_num_on_planes();
  bool first_plane = true;

  for (int i = 0; i < num_planes; i++) {
    NodePath plane_path = cpa->get_on_plane(i);
    PlaneNode *plane_node = DCAST(PlaneNode, plane_path.node());
    if ((plane_node->get_clip_effect() & PlaneNode::CE_collision) != 0) {
      CPT(TransformState) new_transform =
        net_transform->invert_compose(plane_path.get_net_transform());

      LPlane plane = plane_node->get_plane() * new_transform->get_mat();
      if (first_plane) {
        first_plane = false;
        if (!clip_polygon(new_points, _points[plane_no], plane, plane_no)) {
          all_in = false;
        }
      } else {
        Points last_points;
        last_points.swap(new_points);
        if (!clip_polygon(new_points, last_points, plane, plane_no)) {
          all_in = false;
        }
      }
    }
  }

  if (!all_in) {
    compute_vectors(new_points);
  }

  return all_in;
}
/**
 * Clips the source_points of the polygon by the indicated clipping plane, and
 * modifies new_points to reflect the new set of clipped points (but does not
 * compute the vectors in new_points).
 *
 * The return value is true if the set of points is unmodified (all points are
 * behind the clip plane), or false otherwise.
 */
bool CollisionBox::
clip_polygon(CollisionBox::Points &new_points,
             const CollisionBox::Points &source_points,
             const LPlane &plane, int plane_no) const {
  new_points.clear();
  if (source_points.empty()) {
    return true;
  }

  LPoint3 from3d;
  LVector3 delta3d;
  if (!plane.intersects_plane(from3d, delta3d, get_plane(plane_no))) {
    // The clipping plane is parallel to the polygon.  The polygon is either
    // all in or all out.
    if (plane.dist_to_plane(get_plane(plane_no).get_point()) < 0.0) {
      // A point within the polygon is behind the clipping plane: the polygon
      // is all in.
      new_points = source_points;
      return true;
    }
    return false;
  }

  // Project the line of intersection into the 2-d plane.  Now we have a 2-d
  // clipping line.
  LPoint2 from2d = to_2d(from3d,plane_no);
  LVector2 delta2d = to_2d(delta3d,plane_no);

  PN_stdfloat a = -delta2d[1];
  PN_stdfloat b = delta2d[0];
  PN_stdfloat c = from2d[0] * delta2d[1] - from2d[1] * delta2d[0];

  // Now walk through the points.  Any point on the left of our line gets
  // removed, and the line segment clipped at the point of intersection.

  // We might increase the number of vertices by as many as 1, if the plane
  // clips off exactly one corner.  (We might also decrease the number of
  // vertices, or keep them the same number.)
  new_points.reserve(source_points.size() + 1);

  LPoint2 last_point = source_points.back()._p;
  bool last_is_in = !is_right(last_point - from2d, delta2d);
  bool all_in = last_is_in;
  Points::const_iterator pi;
  for (pi = source_points.begin(); pi != source_points.end(); ++pi) {
    const LPoint2 &this_point = (*pi)._p;
    bool this_is_in = !is_right(this_point - from2d, delta2d);

    // There appears to be a compiler bug in gcc 4.0: we need to extract this
    // comparison outside of the if statement.
    bool crossed_over = (this_is_in != last_is_in);
    if (crossed_over) {
      // We have just crossed over the clipping line.  Find the point of
      // intersection.
      LVector2 d = this_point - last_point;
      PN_stdfloat denom = (a * d[0] + b * d[1]);
      if (denom != 0.0) {
        PN_stdfloat t = -(a * last_point[0] + b * last_point[1] + c) / denom;
        LPoint2 p = last_point + t * d;

        new_points.push_back(PointDef(p[0], p[1]));
        last_is_in = this_is_in;
      }
    }

    if (this_is_in) {
      // We are behind the clipping line.  Keep the point.
      new_points.push_back(PointDef(this_point[0], this_point[1]));
    } else {
      all_in = false;
    }

    last_point = this_point;
  }

  return all_in;
}


/**
 * Returns the linear distance from the 2-d point to the nearest part of the
 * polygon defined by the points vector.  The result is negative if the point
 * is within the polygon.
 */
PN_stdfloat CollisionBox::
dist_to_polygon(const LPoint2 &p, const CollisionBox::Points &points) const {

  // We know that that the polygon is convex and is defined with the points in
  // counterclockwise order.  Therefore, we simply compare the signed distance
  // to each line segment; we ignore any negative values, and take the minimum
  // of all the positive values.

  // If all values are negative, the point is within the polygon; we therefore
  // return an arbitrary negative result.

  bool got_dist = false;
  PN_stdfloat best_dist = -1.0f;

  size_t num_points = points.size();
  for (size_t i = 0; i < num_points - 1; ++i) {
    PN_stdfloat d = dist_to_line_segment(p, points[i]._p, points[i + 1]._p,
                                   points[i]._v);
    if (d >= 0.0f) {
      if (!got_dist || d < best_dist) {
        best_dist = d;
        got_dist = true;
      }
    }
  }

  PN_stdfloat d = dist_to_line_segment(p, points[num_points - 1]._p, points[0]._p,
                                 points[num_points - 1]._v);
  if (d >= 0.0f) {
    if (!got_dist || d < best_dist) {
      best_dist = d;
      got_dist = true;
    }
  }

  return best_dist;
}

/**
 * Returns the linear distance of p to the line segment defined by f and t,
 * where v = (t - f).normalize(). The result is negative if p is left of the
 * line, positive if it is right of the line.  If the result is positive, it
 * is constrained by endpoints of the line segment (i.e.  the result might be
 * larger than it would be for a straight distance-to-line test).  If the
 * result is negative, we don't bother.
 */
PN_stdfloat CollisionBox::
dist_to_line_segment(const LPoint2 &p,
                     const LPoint2 &f, const LPoint2 &t,
                     const LVector2 &v) {
  LVector2 v1 = (p - f);
  PN_stdfloat d = (v1[0] * v[1] - v1[1] * v[0]);
  if (d < 0.0f) {
    return d;
  }

  // Compute the nearest point on the line.
  LPoint2 q = p + LVector2(-v[1], v[0]) * d;

  // Now constrain that point to the line segment.
  if (v[0] > 0.0f) {
    // X+
    if (v[1] > 0.0f) {
      // Y+
      if (v[0] > v[1]) {
        // X-dominant.
        if (q[0] < f[0]) {
          return (p - f).length();
        } if (q[0] > t[0]) {
          return (p - t).length();
        } else {
          return d;
        }
      } else {
        // Y-dominant.
        if (q[1] < f[1]) {
          return (p - f).length();
        } if (q[1] > t[1]) {
          return (p - t).length();
        } else {
          return d;
        }
      }
    } else {
      // Y-
      if (v[0] > -v[1]) {
        // X-dominant.
        if (q[0] < f[0]) {
          return (p - f).length();
        } if (q[0] > t[0]) {
          return (p - t).length();
        } else {
          return d;
        }
      } else {
        // Y-dominant.
        if (q[1] > f[1]) {
          return (p - f).length();
        } if (q[1] < t[1]) {
          return (p - t).length();
        } else {
          return d;
        }
      }
    }
  } else {
    // X-
    if (v[1] > 0.0f) {
      // Y+
      if (-v[0] > v[1]) {
        // X-dominant.
        if (q[0] > f[0]) {
          return (p - f).length();
        } if (q[0] < t[0]) {
          return (p - t).length();
        } else {
          return d;
        }
      } else {
        // Y-dominant.
        if (q[1] < f[1]) {
          return (p - f).length();
        } if (q[1] > t[1]) {
          return (p - t).length();
        } else {
          return d;
        }
      }
    } else {
      // Y-
      if (-v[0] > -v[1]) {
        // X-dominant.
        if (q[0] > f[0]) {
          return (p - f).length();
        } if (q[0] < t[0]) {
          return (p - t).length();
        } else {
          return d;
        }
      } else {
        // Y-dominant.
        if (q[1] > f[1]) {
          return (p - f).length();
        } if (q[1] < t[1]) {
          return (p - t).length();
        } else {
          return d;
        }
      }
    }
  }
}

/**
 * Returns true if the indicated point is within the polygon's 2-d space,
 * false otherwise.
 */
bool CollisionBox::
point_is_inside(const LPoint2 &p, const CollisionBox::Points &points) const {
  // We insist that the polygon be convex.  This makes things a bit simpler.
  // In the case of a convex polygon, defined with points in counterclockwise
  // order, a point is interior to the polygon iff the point is not right of
  // each of the edges.
  for (int i = 0; i < (int)points.size() - 1; i++) {
    if (is_right(p - points[i]._p, points[i+1]._p - points[i]._p)) {
      return false;
    }
  }
  if (is_right(p - points[points.size() - 1]._p,
               points[0]._p - points[points.size() - 1]._p)) {
    return false;
  }

  return true;
}

/**
 * Now that the _p members of the given points array have been computed, go
 * back and compute all of the _v members.
 */
void CollisionBox::
compute_vectors(Points &points) {
  size_t num_points = points.size();
  for (size_t i = 0; i < num_points; i++) {
    points[i]._v = points[(i + 1) % num_points]._p - points[i]._p;
    points[i]._v.normalize();
  }
}

/**
 * Factory method to generate a CollisionBox object
 */
void CollisionBox::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_CollisionBox);
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
  for(int i=0; i < 8; i++) {
    _vertex[i].write_datagram(me);
  }
  me.add_stdfloat(_radius);
  me.add_stdfloat(_x);
  me.add_stdfloat(_y);
  me.add_stdfloat(_z);
  for(int i=0; i < 6; i++) {
    _planes[i].write_datagram(me);
  }
  for(int i=0; i < 6; i++) {
    _to_2d_mat[i].write_datagram(me);
  }
  for(int i=0; i < 6; i++) {
    me.add_uint16(_points[i].size());
    for (size_t j = 0; j < _points[i].size(); j++) {
      _points[i][j]._p.write_datagram(me);
      _points[i][j]._v.write_datagram(me);
    }
  }
}

/**
 * Factory method to generate a CollisionBox object
 */
TypedWritable *CollisionBox::
make_CollisionBox(const FactoryParams &params) {
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
  for(int i=0; i < 8; i++) {
    _vertex[i].read_datagram(scan);
  }
  _radius = scan.get_stdfloat();
  _x = scan.get_stdfloat();
  _y = scan.get_stdfloat();
  _z = scan.get_stdfloat();
  for(int i=0; i < 6; i++) {
    _planes[i].read_datagram(scan);
  }
  for(int i=0; i < 6; i++) {
    _to_2d_mat[i].read_datagram(scan);
  }
  for(int i=0; i < 6; i++) {
    size_t size = scan.get_uint16();
    for (size_t j = 0; j < size; j++) {
      LPoint2 p;
      LVector2 v;
      p.read_datagram(scan);
      v.read_datagram(scan);
      _points[i].push_back(PointDef(p, v));
    }
  }
}
