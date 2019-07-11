/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionPolygon.cxx
 * @author drose
 * @date 2000-04-25
 */

#include "collisionPolygon.h"

#include "collisionHandler.h"
#include "collisionEntry.h"
#include "collisionSphere.h"
#include "collisionCapsule.h"
#include "collisionLine.h"
#include "collisionRay.h"
#include "collisionSegment.h"
#include "collisionParabola.h"
#include "config_collide.h"
#include "cullTraverserData.h"
#include "boundingBox.h"
#include "pointerToArray.h"
#include "geomNode.h"
#include "geom.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "transformState.h"
#include "clipPlaneAttrib.h"
#include "nearly_zero.h"
#include "geom.h"
#include "geomTrifans.h"
#include "geomLinestrips.h"
#include "geomVertexWriter.h"
#include "renderState.h"
#include "epvector.h"

#include <algorithm>

using std::max;
using std::min;

PStatCollector CollisionPolygon::_volume_pcollector("Collision Volumes:CollisionPolygon");
PStatCollector CollisionPolygon::_test_pcollector("Collision Tests:CollisionPolygon");
TypeHandle CollisionPolygon::_type_handle;

/**
 *
 */
CollisionPolygon::
CollisionPolygon(const CollisionPolygon &copy) :
  CollisionPlane(copy),
  _points(copy._points),
  _to_2d_mat(copy._to_2d_mat)
{
}

/**
 *
 */
CollisionSolid *CollisionPolygon::
make_copy() {
  return new CollisionPolygon(*this);
}

/**
 * Verifies that the indicated set of points will define a valid
 * CollisionPolygon: that is, at least three non-collinear points, with no
 * points repeated.
 *
 * This does not check that the polygon defined is convex; that check is made
 * later, once we have projected the points to 2-d space where the decision is
 * easier.
 */
bool CollisionPolygon::
verify_points(const LPoint3 *begin, const LPoint3 *end) {
  int num_points = end - begin;
  if (num_points < 3) {
    return false;
  }

  bool all_ok = true;

  // First, check for repeated or invalid points.
  const LPoint3 *pi;
  for (pi = begin; pi != end && all_ok; ++pi) {
    if ((*pi).is_nan()) {
      all_ok = false;
    } else {
      // Make sure no points are repeated.
      const LPoint3 *pj;
      for (pj = begin; pj != pi && all_ok; ++pj) {
        if ((*pj) == (*pi)) {
          all_ok = false;
        }
      }
    }
  }

  if (all_ok) {
    // Create a plane to determine the planarity of the first three points (or
    // the first two points and the nth point thereafter, in case the first
    // three points happen to be collinear).
    bool got_normal = false;
    for (int i = 2; i < num_points && !got_normal; i++) {
      LPlane plane(begin[0], begin[1], begin[i]);
      LVector3 normal = plane.get_normal();
      PN_stdfloat normal_length = normal.length();
      got_normal = IS_THRESHOLD_EQUAL(normal_length, 1.0f, 0.001f);
    }

    if (!got_normal) {
      all_ok = false;
    }
  }

  return all_ok;
}

/**
 * Returns true if the CollisionPolygon is valid (that is, it has at least
 * three vertices), or false otherwise.
 */
bool CollisionPolygon::
is_valid() const {
  return (_points.size() >= 3);
}

/**
 * Returns true if the CollisionPolygon appears to be concave, or false if it
 * is safely convex.
 */
bool CollisionPolygon::
is_concave() const {
  if (_points.size() < 3) {
    // It's not even a valid polygon.
    return true;
  }

  LPoint2 p0 = _points[0]._p;
  LPoint2 p1 = _points[1]._p;
  PN_stdfloat dx1 = p1[0] - p0[0];
  PN_stdfloat dy1 = p1[1] - p0[1];
  p0 = p1;
  p1 = _points[2]._p;

  PN_stdfloat dx2 = p1[0] - p0[0];
  PN_stdfloat dy2 = p1[1] - p0[1];
  int asum = ((dx1 * dy2 - dx2 * dy1 >= 0.0f) ? 1 : 0);

  for (size_t i = 0; i < _points.size() - 1; i++) {
    p0 = p1;
    p1 = _points[(i+3) % _points.size()]._p;

    dx1 = dx2;
    dy1 = dy2;
    dx2 = p1[0] - p0[0];
    dy2 = p1[1] - p0[1];
    int csum = ((dx1 * dy2 - dx2 * dy1 >= 0.0f) ? 1 : 0);

    if (csum ^ asum) {
      // Oops, the polygon is concave.
      return true;
    }
  }

  // The polygon is safely convex.
  return false;
}

/**
 * Transforms the solid by the indicated matrix.
 */
void CollisionPolygon::
xform(const LMatrix4 &mat) {
  // We need to convert all the vertices to 3-d for this operation, and then
  // convert them back.  Hopefully we won't lose too much precision during all
  // of this.

  if (collide_cat.is_spam()) {
    collide_cat.spam()
      << "CollisionPolygon transformed by:\n";
    mat.write(collide_cat.spam(false), 2);
    if (_points.empty()) {
      collide_cat.spam(false)
        << "  (no points)\n";
    }
  }

  if (!_points.empty()) {
    LMatrix4 to_3d_mat;
    rederive_to_3d_mat(to_3d_mat);

    epvector<LPoint3> verts;
    verts.reserve(_points.size());
    Points::const_iterator pi;
    for (pi = _points.begin(); pi != _points.end(); ++pi) {
      verts.push_back(to_3d((*pi)._p, to_3d_mat) * mat);
    }

    const LPoint3 *verts_begin = &verts[0];
    const LPoint3 *verts_end = verts_begin + verts.size();
    setup_points(verts_begin, verts_end);
  }

  CollisionSolid::xform(mat);
}

/**
 * Returns the point in space deemed to be the "origin" of the solid for
 * collision purposes.  The closest intersection point to this origin point is
 * considered to be the most significant.
 */
LPoint3 CollisionPolygon::
get_collision_origin() const {
  LMatrix4 to_3d_mat;
  rederive_to_3d_mat(to_3d_mat);

  LPoint2 median = _points[0]._p;
  for (int n = 1; n < (int)_points.size(); n++) {
    median += _points[n]._p;
  }
  median /= _points.size();

  return to_3d(median, to_3d_mat);
}

/**
 * Returns a GeomNode that may be rendered to visualize the CollisionSolid.
 * This is used during the cull traversal to render the CollisionNodes that
 * have been made visible.
 */
PT(PandaNode) CollisionPolygon::
get_viz(const CullTraverser *trav, const CullTraverserData &data,
        bool bounds_only) const {
  const ClipPlaneAttrib *cpa = DCAST(ClipPlaneAttrib, data._state->get_attrib(ClipPlaneAttrib::get_class_slot()));
  if (cpa == nullptr) {
    // Fortunately, the polygon is not clipped.  This is the normal, easy
    // case.
    return CollisionSolid::get_viz(trav, data, bounds_only);
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "drawing polygon with clip plane " << *cpa << "\n";
  }

  // The polygon is clipped.  We need to render it clipped.  We could just
  // turn on the ClipPlaneAttrib state and render the full polygon, letting
  // the hardware do the clipping, but we get fancy and clip it by hand
  // instead, just to prove that our clipping algorithm works properly.  This
  // does require some more dynamic work.
  Points new_points;
  if (apply_clip_plane(new_points, cpa, data.get_net_transform(trav))) {
    // All points are behind the clip plane; just draw the original polygon.
    return CollisionSolid::get_viz(trav, data, bounds_only);
  }

  if (new_points.empty()) {
    // All points are in front of the clip plane; draw nothing.
    return nullptr;
  }

  // Draw the clipped polygon.
  PT(GeomNode) viz_geom_node = new GeomNode("viz");
  PT(GeomNode) bounds_viz_geom_node = new GeomNode("bounds_viz");
  draw_polygon(viz_geom_node, bounds_viz_geom_node, new_points);

  if (bounds_only) {
    return bounds_viz_geom_node;
  } else {
    return viz_geom_node;
  }
}

/**
 * Returns a PStatCollector that is used to count the number of bounding
 * volume tests made against a solid of this type in a given frame.
 */
PStatCollector &CollisionPolygon::
get_volume_pcollector() {
  return _volume_pcollector;
}

/**
 * Returns a PStatCollector that is used to count the number of intersection
 * tests made against a solid of this type in a given frame.
 */
PStatCollector &CollisionPolygon::
get_test_pcollector() {
  return _test_pcollector;
}

/**
 *
 */
void CollisionPolygon::
output(std::ostream &out) const {
  out << "cpolygon, (" << get_plane()
      << "), " << _points.size() << " vertices";
}

/**
 *
 */
void CollisionPolygon::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << (*this) << "\n";
  Points::const_iterator pi;
  for (pi = _points.begin(); pi != _points.end(); ++pi) {
    indent(out, indent_level + 2) << (*pi)._p << "\n";
  }

  LMatrix4 to_3d_mat;
  rederive_to_3d_mat(to_3d_mat);
  out << "In 3-d space:\n";
  for (pi = _points.begin(); pi != _points.end(); ++pi) {
    LVertex vert = to_3d((*pi)._p, to_3d_mat);
    indent(out, indent_level + 2) << vert << "\n";
  }
}

/**
 *
 */
PT(BoundingVolume) CollisionPolygon::
compute_internal_bounds() const {
  if (_points.empty()) {
    return new BoundingBox;
  }

  LMatrix4 to_3d_mat;
  rederive_to_3d_mat(to_3d_mat);

  Points::const_iterator pi = _points.begin();
  LPoint3 p = to_3d((*pi)._p, to_3d_mat);

  LPoint3 x = p;
  LPoint3 n = p;

  for (++pi; pi != _points.end(); ++pi) {
    p = to_3d((*pi)._p, to_3d_mat);

    n.set(min(n[0], p[0]),
          min(n[1], p[1]),
          min(n[2], p[2]));
    x.set(max(x[0], p[0]),
          max(x[1], p[1]),
          max(x[2], p[2]));
  }

  return new BoundingBox(n, x);
}

/**
 * This is part of the double-dispatch implementation of test_intersection().
 * It is called when the "from" object is a sphere.
 */
PT(CollisionEntry) CollisionPolygon::
test_intersection_from_sphere(const CollisionEntry &entry) const {
  if (_points.size() < 3) {
    return nullptr;
  }

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

  if (wrt_prev_space != wrt_space) {
    // If we have a delta between the previous position and the current
    // position, we use that to determine some more properties of the
    // collision.
    LPoint3 b = from_center;
    LPoint3 a = sphere->get_center() * wrt_prev_space->get_mat();
    LVector3 delta = b - a;

    // First, there is no collision if the "from" object is definitely moving
    // in the same direction as the plane's normal.
    PN_stdfloat dot = delta.dot(get_normal());
    if (dot > 0.1f) {
      return nullptr;
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
      PN_stdfloat dist_to_p = dist_to_plane(a);
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

  LVector3 normal = (has_effective_normal() && sphere->get_respect_effective_normal()) ? get_effective_normal() : get_normal();
#ifndef NDEBUG
  if (!IS_THRESHOLD_EQUAL(normal.length_squared(), 1.0f, 0.001)) {
    collide_cat.info()
      << "polygon within " << entry.get_into_node_path()
      << " has normal " << normal << " of length " << normal.length()
      << "\n";
    normal.normalize();
  }
#endif

  // The nearest point within the plane to our center is the intersection of
  // the line (center, center - normal) with the plane.
  PN_stdfloat dist;
  if (!get_plane().intersects_line(dist, from_center, -get_normal())) {
    // No intersection with plane?  This means the plane's effective normal
    // was within the plane itself.  A useless polygon.
    return nullptr;
  }

  if (dist > from_radius || dist < -from_radius) {
    // No intersection with the plane.
    return nullptr;
  }

  LPoint2 p = to_2d(from_center - dist * get_normal());
  PN_stdfloat edge_dist = 0.0f;

  const ClipPlaneAttrib *cpa = entry.get_into_clip_planes();
  if (cpa != nullptr) {
    // We have a clip plane; apply it.
    Points new_points;
    if (apply_clip_plane(new_points, cpa, entry.get_into_node_path().get_net_transform())) {
      // All points are behind the clip plane; just do the default test.
      edge_dist = dist_to_polygon(p, _points);

    } else if (new_points.empty()) {
      // The polygon is completely clipped.
      return nullptr;

    } else {
      // Test against the clipped polygon.
      edge_dist = dist_to_polygon(p, new_points);
    }

  } else {
    // No clip plane is in effect.  Do the default test.
    edge_dist = dist_to_polygon(p, _points);
  }

  // Now we have edge_dist, which is the distance from the sphere center to
  // the nearest edge of the polygon, within the polygon's plane.

  if (edge_dist > from_radius) {
    // No intersection; the circle is outside the polygon.
    return nullptr;
  }

  // The sphere appears to intersect the polygon.  If the edge is less than
  // from_radius away, the sphere may be resting on an edge of the polygon.
  // Determine how far the center of the sphere must remain from the plane,
  // based on its distance from the nearest edge.

  PN_stdfloat max_dist = from_radius;
  if (edge_dist >= 0.0f) {
    PN_stdfloat max_dist_2 = max(from_radius_2 - edge_dist * edge_dist, (PN_stdfloat)0.0);
    max_dist = csqrt(max_dist_2);
  }

  if (dist > max_dist) {
    // There's no intersection: the sphere is hanging off the edge.
    return nullptr;
  }

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
    get_plane().intersects_line(orig_dist, orig_center, -normal);
    into_depth = max_dist - orig_dist;
  }

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(from_center - normal * dist);
  new_entry->set_interior_point(from_center - normal * (dist + into_depth));
  new_entry->set_contact_pos(contact_point);
  new_entry->set_contact_normal(get_normal());
  new_entry->set_t(actual_t);

  return new_entry;
}

/**
 * This is part of the double-dispatch implementation of test_intersection().
 * It is called when the "from" object is a line.
 */
PT(CollisionEntry) CollisionPolygon::
test_intersection_from_line(const CollisionEntry &entry) const {
  if (_points.size() < 3) {
    return nullptr;
  }

  const CollisionLine *line;
  DCAST_INTO_R(line, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_origin = line->get_origin() * wrt_mat;
  LVector3 from_direction = line->get_direction() * wrt_mat;

  PN_stdfloat t;
  if (!get_plane().intersects_line(t, from_origin, from_direction)) {
    // No intersection.
    return nullptr;
  }

  LPoint3 plane_point = from_origin + t * from_direction;
  LPoint2 p = to_2d(plane_point);

  const ClipPlaneAttrib *cpa = entry.get_into_clip_planes();
  if (cpa != nullptr) {
    // We have a clip plane; apply it.
    Points new_points;
    if (apply_clip_plane(new_points, cpa, entry.get_into_node_path().get_net_transform())) {
      // All points are behind the clip plane.
      if (!point_is_inside(p, _points)) {
        return nullptr;
      }

    } else {
      if (new_points.size() < 3) {
        return nullptr;
      }
      if (!point_is_inside(p, new_points)) {
        return nullptr;
      }
    }

  } else {
    // No clip plane is in effect.  Do the default test.
    if (!point_is_inside(p, _points)) {
      return nullptr;
    }
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LVector3 normal = (has_effective_normal() && line->get_respect_effective_normal()) ? get_effective_normal() : get_normal();

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(plane_point);

  return new_entry;
}

/**
 * This is part of the double-dispatch implementation of test_intersection().
 * It is called when the "from" object is a ray.
 */
PT(CollisionEntry) CollisionPolygon::
test_intersection_from_ray(const CollisionEntry &entry) const {
  if (_points.size() < 3) {
    return nullptr;
  }

  const CollisionRay *ray;
  DCAST_INTO_R(ray, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_origin = ray->get_origin() * wrt_mat;
  LVector3 from_direction = ray->get_direction() * wrt_mat;

  PN_stdfloat t;
  if (!get_plane().intersects_line(t, from_origin, from_direction)) {
    // No intersection.
    return nullptr;
  }

  if (t < 0.0f) {
    // The intersection point is before the start of the ray.
    return nullptr;
  }

  LPoint3 plane_point = from_origin + t * from_direction;
  LPoint2 p = to_2d(plane_point);

  const ClipPlaneAttrib *cpa = entry.get_into_clip_planes();
  if (cpa != nullptr) {
    // We have a clip plane; apply it.
    Points new_points;
    if (apply_clip_plane(new_points, cpa, entry.get_into_node_path().get_net_transform())) {
      // All points are behind the clip plane.
      if (!point_is_inside(p, _points)) {
        return nullptr;
      }

    } else {
      if (new_points.size() < 3) {
        return nullptr;
      }
      if (!point_is_inside(p, new_points)) {
        return nullptr;
      }
    }

  } else {
    // No clip plane is in effect.  Do the default test.
    if (!point_is_inside(p, _points)) {
      return nullptr;
    }
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LVector3 normal = (has_effective_normal() && ray->get_respect_effective_normal()) ? get_effective_normal() : get_normal();

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(plane_point);

  return new_entry;
}

/**
 * This is part of the double-dispatch implementation of test_intersection().
 * It is called when the "from" object is a segment.
 */
PT(CollisionEntry) CollisionPolygon::
test_intersection_from_segment(const CollisionEntry &entry) const {
  if (_points.size() < 3) {
    return nullptr;
  }

  const CollisionSegment *segment;
  DCAST_INTO_R(segment, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_a = segment->get_point_a() * wrt_mat;
  LPoint3 from_b = segment->get_point_b() * wrt_mat;
  LPoint3 from_direction = from_b - from_a;

  PN_stdfloat t;
  if (!get_plane().intersects_line(t, from_a, from_direction)) {
    // No intersection.
    return nullptr;
  }

  if (t < 0.0f || t > 1.0f) {
    // The intersection point is before the start of the segment or after the
    // end of the segment.
    return nullptr;
  }

  LPoint3 plane_point = from_a + t * from_direction;
  LPoint2 p = to_2d(plane_point);

  const ClipPlaneAttrib *cpa = entry.get_into_clip_planes();
  if (cpa != nullptr) {
    // We have a clip plane; apply it.
    Points new_points;
    if (apply_clip_plane(new_points, cpa, entry.get_into_node_path().get_net_transform())) {
      // All points are behind the clip plane.
      if (!point_is_inside(p, _points)) {
        return nullptr;
      }

    } else {
      if (new_points.size() < 3) {
        return nullptr;
      }
      if (!point_is_inside(p, new_points)) {
        return nullptr;
      }
    }

  } else {
    // No clip plane is in effect.  Do the default test.
    if (!point_is_inside(p, _points)) {
      return nullptr;
    }
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LVector3 normal = (has_effective_normal() && segment->get_respect_effective_normal()) ? get_effective_normal() : get_normal();

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(plane_point);

  return new_entry;
}

/**
 * This is part of the double-dispatch implementation of test_intersection().
 * It is called when the "from" object is a parabola.
 */
PT(CollisionEntry) CollisionPolygon::
test_intersection_from_parabola(const CollisionEntry &entry) const {
  if (_points.size() < 3) {
    return nullptr;
  }

  const CollisionParabola *parabola;
  DCAST_INTO_R(parabola, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  // Convert the parabola into local coordinate space.
  LParabola local_p(parabola->get_parabola());
  local_p.xform(wrt_mat);

  PN_stdfloat t1, t2;
  if (!get_plane().intersects_parabola(t1, t2, local_p)) {
    // No intersection.
    return nullptr;
  }

  PN_stdfloat t;
  if (t1 >= parabola->get_t1() && t1 <= parabola->get_t2()) {
    if (t2 >= parabola->get_t1() && t2 <= parabola->get_t2()) {
      // Both intersection points are within our segment of the parabola.
      // Choose the first of the two.
      t = min(t1, t2);
    } else {
      // Only t1 is within our segment.
      t = t1;
    }

  } else if (t2 >= parabola->get_t1() && t2 <= parabola->get_t2()) {
    // Only t2 is within our segment.
    t = t2;

  } else {
    // Neither intersection point is within our segment.
    return nullptr;
  }

  LPoint3 plane_point = local_p.calc_point(t);
  LPoint2 p = to_2d(plane_point);

  const ClipPlaneAttrib *cpa = entry.get_into_clip_planes();
  if (cpa != nullptr) {
    // We have a clip plane; apply it.
    Points new_points;
    if (apply_clip_plane(new_points, cpa, entry.get_into_node_path().get_net_transform())) {
      // All points are behind the clip plane.
      if (!point_is_inside(p, _points)) {
        return nullptr;
      }

    } else {
      if (new_points.size() < 3) {
        return nullptr;
      }
      if (!point_is_inside(p, new_points)) {
        return nullptr;
      }
    }

  } else {
    // No clip plane is in effect.  Do the default test.
    if (!point_is_inside(p, _points)) {
      return nullptr;
    }
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LVector3 normal = (has_effective_normal() && parabola->get_respect_effective_normal()) ? get_effective_normal() : get_normal();

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(plane_point);

  return new_entry;
}

/**
 * This is part of the double-dispatch implementation of test_intersection().
 * It is called when the "from" object is a capsule.
 */
PT(CollisionEntry) CollisionPolygon::
test_intersection_from_capsule(const CollisionEntry &entry) const {
  if (_points.size() < 3) {
    return nullptr;
  }

  const CollisionCapsule *capsule;
  DCAST_INTO_R(capsule, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();
  LMatrix4 plane_mat = wrt_mat * _to_2d_mat;

  LPoint3 from_a = capsule->get_point_a() * plane_mat;
  LPoint3 from_b = capsule->get_point_b() * plane_mat;

  LVector3 from_radius_v =
    LVector3(capsule->get_radius(), 0.0f, 0.0f) * wrt_mat;
  PN_stdfloat from_radius_sq = from_radius_v.length_squared();

  // Check if the capsule is colliding with the plane at all.
  // Are the points on the same side of the plane?
  if ((from_a[1] > 0) == (from_b[1] > 0)) {
    // Yes, so calculate the distance of the closest point.
    PN_stdfloat dist = min(cabs(from_a[1]), cabs(from_b[1]));
    if (dist * dist > from_radius_sq) {
      return nullptr;
    }
  }

  // Order from_a and from_b so that from_a has the deepest point.
  bool swapped = (from_a[1] < from_b[1]);
  if (swapped) {
    std::swap(from_a, from_b);
  }

  LPoint3 surface_point, interior_point;

  // Is the projection of from_a onto the plane inside the polygon?
  LPoint2 from_a_proj(from_a[0], from_a[2]);
  if (point_is_inside(from_a_proj, _points)) {
    // Yes, and we already checked the vertical separation earlier on, so we
    // know that the capsule is touching the polygon near from_a.
    LPoint3 deepest = (swapped ? capsule->get_point_b() : capsule->get_point_a()) * wrt_mat;
    PN_stdfloat from_radius = csqrt(from_radius_sq);
    surface_point = get_plane().project(deepest);
    interior_point = deepest - get_normal() * from_radius;
  }
  else {
    LVector3 from_direction = from_b - from_a;

    // Find the point in the capsule's inner segment with the closest distance
    // to the polygon's edges.  We effectively test a sphere around that point.
    PN_stdfloat min_dist_sq = make_inf((PN_stdfloat)0);
    LPoint3 poly_point;
    LPoint3 line_point;

    LPoint2 last_point = _points.back()._p;
    for (const PointDef &pd : _points) {
      LVector2 dir = last_point - pd._p;
      last_point = pd._p;

      double t1, t2;
      CollisionCapsule::calc_closest_segment_points(t1, t2,
          LPoint3(pd._p[0], 0, pd._p[1]), LVector3(dir[0], 0, dir[1]),
          from_a, from_direction);

      LPoint3 point1(pd._p[0] + dir[0] * t1, 0, pd._p[1] + dir[1] * t1);
      LPoint3 point2 = from_a + from_direction * t2;
      PN_stdfloat dist_sq = (point2 - point1).length_squared();
      if (dist_sq < min_dist_sq) {
        min_dist_sq = dist_sq;
        poly_point = point1;
        line_point = point2;
      }
    }

    // Project the closest point on the segment onto the polygon.  Is this point
    // inside the polygon?
    LPoint2 line_point_proj(line_point[0], line_point[2]);
    if (point_is_inside(line_point_proj, _points)) {
      // Yes, and we already checked the vertical separation earlier on, so we
      // know that the capsule is touching the polygon here.
      LMatrix4 to_3d_mat;
      rederive_to_3d_mat(to_3d_mat);

      surface_point = to_3d(line_point_proj, to_3d_mat);

      LPoint3 interior;
      if (IS_NEARLY_EQUAL(from_a[1], from_b[1])) {
        // It's parallel to the polygon; we can use any point on the segment we
        // want, so we might as well use the point we determined to be closest.
        interior = line_point;
      } else {
        // Use the deepest point.  FIXME: we need something better.  This
        // pushes the capsule out way too much.
        interior = from_a;
      }
      interior[1] += csqrt(from_radius_sq);
      interior_point = interior * to_3d_mat;
    }
    else if (min_dist_sq < from_radius_sq) {
      // No, but it is colliding with an edge.
      LMatrix4 to_3d_mat;
      rederive_to_3d_mat(to_3d_mat);

      surface_point = poly_point * to_3d_mat;

      // Make sure we calculate an interior point that lies below the polygon.
      LVector3 dir = line_point * to_3d_mat - surface_point;
      dir.normalize();
      interior_point = surface_point - dir * (csqrt(from_radius_sq) - csqrt(min_dist_sq));
    }
    else {
      // It is outside the polygon altogether.
      return nullptr;
    }
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);
  LVector3 normal = (has_effective_normal() && capsule->get_respect_effective_normal()) ? get_effective_normal() : get_normal();
  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(surface_point);
  new_entry->set_interior_point(interior_point);

  return new_entry;
}

/**
 * This is part of the double-dispatch implementation of test_intersection().
 * It is called when the "from" object is a box.
 */
PT(CollisionEntry) CollisionPolygon::
test_intersection_from_box(const CollisionEntry &entry) const {
  const CollisionBox *box;
  DCAST_INTO_R(box, entry.get_from(), nullptr);

  // To make things easier, transform the box into the coordinate space of the
  // plane.
  const LMatrix4 &wrt_mat = entry.get_wrt_mat();
  LMatrix4 plane_mat = wrt_mat * _to_2d_mat;

  LPoint3 from_center = box->get_center() * plane_mat;
  LVector3 from_extents = box->get_dimensions() * 0.5f;

  // Determine the basis vectors describing the box.
  LVecBase3 box_x = plane_mat.get_row3(0) * from_extents[0];
  LVecBase3 box_y = plane_mat.get_row3(1) * from_extents[1];
  LVecBase3 box_z = plane_mat.get_row3(2) * from_extents[2];

  // Is there a separating axis between the plane and the box?
  if (cabs(from_center[1]) > cabs(box_x[1]) + cabs(box_y[1]) + cabs(box_z[1])) {
    // There is one.  No collision.
    return nullptr;
  }

  // Now do the same for each of the box' primary axes.
  PN_stdfloat r1, center, r2;

  r1 = cabs(box_x.dot(box_x)) + cabs(box_y.dot(box_x)) + cabs(box_z.dot(box_x));
  project(box_x, center, r2);
  if (cabs(from_center.dot(box_x) - center) > r1 + r2) {
    return nullptr;
  }

  r1 = cabs(box_x.dot(box_y)) + cabs(box_y.dot(box_y)) + cabs(box_z.dot(box_y));
  project(box_y, center, r2);
  if (cabs(from_center.dot(box_y) - center) > r1 + r2) {
    return nullptr;
  }

  r1 = cabs(box_x.dot(box_z)) + cabs(box_y.dot(box_z)) + cabs(box_z.dot(box_z));
  project(box_z, center, r2);
  if (cabs(from_center.dot(box_z) - center) > r1 + r2) {
    return nullptr;
  }

  // Now do the same check for the cross products between the box axes and the
  // polygon edges.
  Points::const_iterator pi;
  for (pi = _points.begin(); pi != _points.end(); ++pi) {
    const PointDef &pd = *pi;
    LVector3 axis;

    axis.set(-box_x[1] * pd._v[1],
              box_x[0] * pd._v[1] - box_x[2] * pd._v[0],
              box_x[1] * pd._v[0]);
    r1 = cabs(box_x.dot(axis)) + cabs(box_y.dot(axis)) + cabs(box_z.dot(axis));
    project(axis, center, r2);
    if (cabs(from_center.dot(axis) - center) > r1 + r2) {
      return nullptr;
    }

    axis.set(-box_y[1] * pd._v[1],
              box_y[0] * pd._v[1] - box_y[2] * pd._v[0],
              box_y[1] * pd._v[0]);
    r1 = cabs(box_x.dot(axis)) + cabs(box_y.dot(axis)) + cabs(box_z.dot(axis));
    project(axis, center, r2);
    if (cabs(from_center.dot(axis) - center) > r1 + r2) {
      return nullptr;
    }

    axis.set(-box_z[1] * pd._v[1],
              box_z[0] * pd._v[1] - box_z[2] * pd._v[0],
              box_z[1] * pd._v[0]);
    r1 = cabs(box_x.dot(axis)) + cabs(box_y.dot(axis)) + cabs(box_z.dot(axis));
    project(axis, center, r2);
    if (cabs(from_center.dot(axis) - center) > r1 + r2) {
      return nullptr;
    }
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LVector3 normal = (has_effective_normal() && box->get_respect_effective_normal()) ? get_effective_normal() : get_normal();
  new_entry->set_surface_normal(normal);

  // Determine which point on the cube will be the interior point.  This is
  // the calculation that is also used for the plane, which is not perfectly
  // applicable, but I suppose it's better than nothing.
  LPoint3 interior_point = box->get_center() * wrt_mat +
    wrt_mat.get_row3(0) * from_extents[0] * ((box_x[1] > 0) - (box_x[1] < 0)) +
    wrt_mat.get_row3(1) * from_extents[1] * ((box_y[1] > 0) - (box_y[1] < 0)) +
    wrt_mat.get_row3(2) * from_extents[2] * ((box_z[1] > 0) - (box_z[1] < 0));

  // The surface point is the interior point projected onto the plane.
  new_entry->set_surface_point(get_plane().project(interior_point));
  new_entry->set_interior_point(interior_point);

  return new_entry;
}

/**
 * Fills the _viz_geom GeomNode up with Geoms suitable for rendering this
 * solid.
 */
void CollisionPolygon::
fill_viz_geom() {
  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "Recomputing viz for " << *this << "\n";
  }
  nassertv(_viz_geom != nullptr && _bounds_viz_geom != nullptr);
  draw_polygon(_viz_geom, _bounds_viz_geom, _points);
}

/**
 * Returns the linear distance of p to the line segment defined by f and t,
 * where v = (t - f).normalize(). The result is negative if p is left of the
 * line, positive if it is right of the line.  If the result is positive, it
 * is constrained by endpoints of the line segment (i.e.  the result might be
 * larger than it would be for a straight distance-to-line test).  If the
 * result is negative, we don't bother.
 */
PN_stdfloat CollisionPolygon::
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
 * Now that the _p members of the given points array have been computed, go
 * back and compute all of the _v members.
 */
void CollisionPolygon::
compute_vectors(Points &points) {
  size_t num_points = points.size();
  for (size_t i = 0; i < num_points; i++) {
    points[i]._v = points[(i + 1) % num_points]._p - points[i]._p;
    points[i]._v.normalize();
  }
}

/**
 * Fills up the indicated GeomNode with the Geoms to draw the polygon
 * indicated with the given set of 2-d points.
 */
void CollisionPolygon::
draw_polygon(GeomNode *viz_geom_node, GeomNode *bounds_viz_geom_node,
             const CollisionPolygon::Points &points) const {
  if (points.size() < 3) {
    if (collide_cat.is_debug()) {
      collide_cat.debug()
        << "(Degenerate poly, ignoring.)\n";
    }
    return;
  }

  LMatrix4 to_3d_mat;
  rederive_to_3d_mat(to_3d_mat);

  PT(GeomVertexData) vdata = new GeomVertexData
    ("collision", GeomVertexFormat::get_v3(),
     Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());

  Points::const_iterator pi;
  for (pi = points.begin(); pi != points.end(); ++pi) {
    vertex.add_data3(to_3d((*pi)._p, to_3d_mat));
  }

  PT(GeomTrifans) body = new GeomTrifans(Geom::UH_static);
  body->add_consecutive_vertices(0, points.size());
  body->close_primitive();

  PT(GeomLinestrips) border = new GeomLinestrips(Geom::UH_static);
  border->add_consecutive_vertices(0, points.size());
  border->add_vertex(0);
  border->close_primitive();

  PT(Geom) geom1 = new Geom(vdata);
  geom1->add_primitive(body);

  PT(Geom) geom2 = new Geom(vdata);
  geom2->add_primitive(border);

  viz_geom_node->add_geom(geom1, ((CollisionPolygon *)this)->get_solid_viz_state());
  viz_geom_node->add_geom(geom2, ((CollisionPolygon *)this)->get_wireframe_viz_state());

  bounds_viz_geom_node->add_geom(geom1, ((CollisionPolygon *)this)->get_solid_bounds_viz_state());
  bounds_viz_geom_node->add_geom(geom2, ((CollisionPolygon *)this)->get_wireframe_bounds_viz_state());
}


/**
 * Returns true if the indicated point is within the polygon's 2-d space,
 * false otherwise.
 */
bool CollisionPolygon::
point_is_inside(const LPoint2 &p, const CollisionPolygon::Points &points) const {
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
 * Returns the linear distance from the 2-d point to the nearest part of the
 * polygon defined by the points vector.  The result is negative if the point
 * is within the polygon.
 */
PN_stdfloat CollisionPolygon::
dist_to_polygon(const LPoint2 &p, const CollisionPolygon::Points &points) const {

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
 * Projects the polygon onto the given axis, returning the center on the line
 * and the half extent.
 */
void CollisionPolygon::
project(const LVector3 &axis, PN_stdfloat &center, PN_stdfloat &extent) const {
  PN_stdfloat begin, end;

  Points::const_iterator pi;
  pi = _points.begin();

  const LPoint2 &point = (*pi)._p;
  begin = point[0] * axis[0] + point[1] * axis[2];
  end = begin;

  for (; pi != _points.end(); ++pi) {
    const LPoint2 &point = (*pi)._p;

    PN_stdfloat t = point[0] * axis[0] + point[1] * axis[2];
    begin = min(begin, t);
    end = max(end, t);
  }

  center = (end + begin) * 0.5f;
  extent = cabs((end - begin) * 0.5f);
}

/**
 *
 */
void CollisionPolygon::
setup_points(const LPoint3 *begin, const LPoint3 *end) {
  int num_points = end - begin;
  nassertv(num_points >= 3);

  _points.clear();

  // Tell the base CollisionPlane class what its plane will be.  To do this,
  // we must first compute the polygon normal.
  LVector3 normal = LVector3::zero();

  // Project the polygon into each of the three major planes and calculate the
  // area of each 2-d projection.  This becomes the polygon normal.  This
  // works because the ratio between these different areas corresponds to the
  // angle at which the polygon is tilted toward each plane.
  for (int i = 0; i < num_points; i++) {
    const LPoint3 &p0 = begin[i];
    const LPoint3 &p1 = begin[(i + 1) % num_points];
    normal[0] += p0[1] * p1[2] - p0[2] * p1[1];
    normal[1] += p0[2] * p1[0] - p0[0] * p1[2];
    normal[2] += p0[0] * p1[1] - p0[1] * p1[0];
  }

  if (normal.length_squared() == 0.0f) {
    // The polygon has no area.
    return;
  }

#ifndef NDEBUG
  // Make sure all the source points are good.
  {
    if (!verify_points(begin, end)) {
      collide_cat.error() << "Invalid points in CollisionPolygon:\n";
      const LPoint3 *pi;
      for (pi = begin; pi != end; ++pi) {
        collide_cat.error(false) << "  " << (*pi) << "\n";
      }
      collide_cat.error(false)
        << "  normal " << normal << " with length " << normal.length() << "\n";

      return;
    }
  }

  if (collide_cat.is_spam()) {
    collide_cat.spam()
      << "CollisionPolygon defined with " << num_points << " vertices:\n";
    const LPoint3 *pi;
    for (pi = begin; pi != end; ++pi) {
      collide_cat.spam(false) << "  " << (*pi) << "\n";
    }
  }
#endif

  set_plane(LPlane(normal, begin[0]));

  // Construct a matrix that rotates the points from the (X,0,Z) plane into
  // the 3-d plane.
  LMatrix4 to_3d_mat;
  calc_to_3d_mat(to_3d_mat);

  // And the inverse matrix rotates points from 3-d space into the 2-d plane.
  _to_2d_mat.invert_from(to_3d_mat);

  // Now project all of the points onto the 2-d plane.

  const LPoint3 *pi;
  for (pi = begin; pi != end; ++pi) {
    LPoint3 point = (*pi) * _to_2d_mat;
    _points.push_back(PointDef(point[0], point[2]));
  }

  nassertv(_points.size() >= 3);

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

  compute_vectors(_points);
}

/**
 * Clips the source_points of the polygon by the indicated clipping plane, and
 * modifies new_points to reflect the new set of clipped points (but does not
 * compute the vectors in new_points).
 *
 * The return value is true if the set of points is unmodified (all points are
 * behind the clip plane), or false otherwise.
 */
bool CollisionPolygon::
clip_polygon(CollisionPolygon::Points &new_points,
             const CollisionPolygon::Points &source_points,
             const LPlane &plane) const {
  new_points.clear();
  if (source_points.empty()) {
    return true;
  }

  LPoint3 from3d;
  LVector3 delta3d;
  if (!plane.intersects_plane(from3d, delta3d, get_plane())) {
    // The clipping plane is parallel to the polygon.  The polygon is either
    // all in or all out.
    if (plane.dist_to_plane(get_plane().get_point()) < 0.0) {
      // A point within the polygon is behind the clipping plane: the polygon
      // is all in.
      new_points = source_points;
      return true;
    }
    return false;
  }

  // Project the line of intersection into the 2-d plane.  Now we have a 2-d
  // clipping line.
  LPoint2 from2d = to_2d(from3d);
  LVector2 delta2d = to_2d(delta3d);

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
 * Clips the polygon by all of the clip planes named in the clip plane
 * attribute and fills new_points up with the resulting points.
 *
 * The return value is true if the set of points is unmodified (all points are
 * behind all the clip planes), or false otherwise.
 */
bool CollisionPolygon::
apply_clip_plane(CollisionPolygon::Points &new_points,
                 const ClipPlaneAttrib *cpa,
                 const TransformState *net_transform) const {
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
        if (!clip_polygon(new_points, _points, plane)) {
          all_in = false;
        }
      } else {
        Points last_points;
        last_points.swap(new_points);
        if (!clip_polygon(new_points, last_points, plane)) {
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
 * Function to write the important information in the particular object to a
 * Datagram
 */
void CollisionPolygon::
write_datagram(BamWriter *manager, Datagram &me) {
  CollisionPlane::write_datagram(manager, me);
  me.add_uint16(_points.size());
  for (size_t i = 0; i < _points.size(); i++) {
    _points[i]._p.write_datagram(me);
    _points[i]._v.write_datagram(me);
  }
  _to_2d_mat.write_datagram(me);
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void CollisionPolygon::
fillin(DatagramIterator &scan, BamReader *manager) {
  CollisionPlane::fillin(scan, manager);

  size_t size = scan.get_uint16();
  for (size_t i = 0; i < size; i++) {
    LPoint2 p;
    LVector2 v;
    p.read_datagram(scan);
    v.read_datagram(scan);
    _points.push_back(PointDef(p, v));
  }
  _to_2d_mat.read_datagram(scan);

  if (manager->get_file_minor_ver() < 13) {
    // Before bam version 6.13, we were inadvertently storing CollisionPolygon
    // vertices clockwise, instead of counter-clockwise.  Correct that by re-
    // projecting.
    if (_points.size() >= 3) {
      LMatrix4 to_3d_mat;
      rederive_to_3d_mat(to_3d_mat);

      epvector<LPoint3> verts;
      verts.reserve(_points.size());
      Points::const_iterator pi;
      for (pi = _points.begin(); pi != _points.end(); ++pi) {
        verts.push_back(to_3d((*pi)._p, to_3d_mat));
      }

      const LPoint3 *verts_begin = &verts[0];
      const LPoint3 *verts_end = verts_begin + verts.size();
      setup_points(verts_begin, verts_end);
    }
  }
}


/**
 * Factory method to generate a CollisionPolygon object
 */
TypedWritable* CollisionPolygon::
make_CollisionPolygon(const FactoryParams &params) {
  CollisionPolygon *me = new CollisionPolygon;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Factory method to generate a CollisionPolygon object
 */
void CollisionPolygon::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_CollisionPolygon);
}
