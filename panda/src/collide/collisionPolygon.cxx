// Filename: collisionPolygon.cxx
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

#include "collisionPolygon.h"
#include "collisionHandler.h"
#include "collisionEntry.h"
#include "collisionSphere.h"
#include "collisionLine.h"
#include "collisionRay.h"
#include "collisionSegment.h"
#include "config_collide.h"
#include "cullTraverserData.h"
#include "boundingSphere.h"
#include "pointerToArray.h"
#include "geomNode.h"
#include "geom.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "geomPolygon.h"
#include "transformState.h"
#include "clipPlaneAttrib.h"
#include "nearly_zero.h"
#include "qpgeom.h"
#include "qpgeomTrifans.h"
#include "qpgeomLinestrips.h"
#include "qpgeomVertexWriter.h"

#include <algorithm>

TypeHandle CollisionPolygon::_type_handle;



////////////////////////////////////////////////////////////////////
//     Function: is_right
//  Description: Returns true if the 2-d v1 is to the right of v2.
////////////////////////////////////////////////////////////////////
INLINE bool
is_right(const LVector2f &v1, const LVector2f &v2) {
  return (-v1[0] * v2[1] + v1[1] * v2[0]) > 0;
}

////////////////////////////////////////////////////////////////////
//     Function: dist_to_line
//  Description: Returns the linear distance of p to the line defined
//               by f and f+v, where v is a normalized vector.  The
//               result is negative if p is left of the line, positive
//               if it is right of the line.
////////////////////////////////////////////////////////////////////
INLINE float
dist_to_line(const LPoint2f &p,
             const LPoint2f &f, const LVector2f &v) {
  LVector2f v1 = (p - f);
  return (-v1[0] * v[1] + v1[1] * v[0]);
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CollisionPolygon::
CollisionPolygon(const CollisionPolygon &copy) :
  CollisionPlane(copy),
  _points(copy._points),
  _to_2d_mat(copy._to_2d_mat)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid *CollisionPolygon::
make_copy() {
  return new CollisionPolygon(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::verify_points
//       Access: Public, Static
//  Description: Verifies that the indicated set of points will define
//               a valid CollisionPolygon: that is, at least three
//               non-collinear points, with no points repeated.
//
//               This does not check that the polygon defined is
//               convex; that check is made later, once we have
//               projected the points to 2-d space where the decision
//               is easier.
////////////////////////////////////////////////////////////////////
bool CollisionPolygon::
verify_points(const LPoint3f *begin, const LPoint3f *end) {
  int num_points = end - begin;
  if (num_points < 3) {
    return false;
  }

  bool all_ok = true;

  // First, check for repeated or invalid points.
  const LPoint3f *pi;
  for (pi = begin; pi != end && all_ok; ++pi) {
    if ((*pi).is_nan()) {
      all_ok = false;
    } else {
      // Make sure no points are repeated.
      const LPoint3f *pj;
      for (pj = begin; pj != pi && all_ok; ++pj) {
        if ((*pj).almost_equal(*pi)) {
          all_ok = false;
        }
      }
    }
  }

  if (all_ok) {
    // Create a plane to determine the planarity of the first three
    // points (or the first two points and the nth point thereafter, in
    // case the first three points happen to be collinear).
    bool got_normal = false;
    for (int i = 2; i < num_points && !got_normal; i++) {
      Planef plane(begin[0], begin[1], begin[i]);
      LVector3f normal = plane.get_normal();
      float normal_length = normal.length();
      got_normal = IS_THRESHOLD_EQUAL(normal_length, 1.0f, 0.001f);
    }

    if (!got_normal) {
      all_ok = false;
    }
  }

  return all_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::is_valid
//       Access: Public
//  Description: Returns true if the CollisionPolygon is valid
//               (that is, it has at least three vertices), or false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool CollisionPolygon::
is_valid() const {
  return (_points.size() >= 3);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::is_concave
//       Access: Public
//  Description: Returns true if the CollisionPolygon appears to be
//               concave, or false if it is safely convex.
////////////////////////////////////////////////////////////////////
bool CollisionPolygon::
is_concave() const {
  if (_points.size() < 3) {
    // It's not even a valid polygon.
    return true;
  }

  LPoint2f p0 = _points[0]._p;
  LPoint2f p1 = _points[1]._p;
  float dx1 = p1[0] - p0[0];
  float dy1 = p1[1] - p0[1];
  p0 = p1;
  p1 = _points[2]._p;

  float dx2 = p1[0] - p0[0];
  float dy2 = p1[1] - p0[1];
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::xform
//       Access: Public, Virtual
//  Description: Transforms the solid by the indicated matrix.
////////////////////////////////////////////////////////////////////
void CollisionPolygon::
xform(const LMatrix4f &mat) {
  // We need to convert all the vertices to 3-d for this operation,
  // and then convert them back.  Hopefully we won't lose too much
  // precision during all of this.

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
    LMatrix4f to_3d_mat;
    rederive_to_3d_mat(to_3d_mat);

    pvector<LPoint3f> verts;
    Points::const_iterator pi;
    for (pi = _points.begin(); pi != _points.end(); ++pi) {
      verts.push_back(to_3d((*pi)._p, to_3d_mat) * mat);
    }

    const LPoint3f *verts_begin = &verts[0];
    const LPoint3f *verts_end = verts_begin + verts.size();
    setup_points(verts_begin, verts_end);
  }

  CollisionSolid::xform(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::get_collision_origin
//       Access: Public, Virtual
//  Description: Returns the point in space deemed to be the "origin"
//               of the solid for collision purposes.  The closest
//               intersection point to this origin point is considered
//               to be the most significant.
////////////////////////////////////////////////////////////////////
LPoint3f CollisionPolygon::
get_collision_origin() const {
  LMatrix4f to_3d_mat;
  rederive_to_3d_mat(to_3d_mat);

  LPoint2f median = _points[0]._p;
  for (int n = 1; n < (int)_points.size(); n++) {
    median += _points[n]._p;
  }
  median /= _points.size();

  return to_3d(median, to_3d_mat);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::get_viz
//       Access: Public, Virtual
//  Description: Returns a GeomNode that may be rendered to visualize
//               the CollisionSolid.  This is used during the cull
//               traversal to render the CollisionNodes that have been
//               made visible.
////////////////////////////////////////////////////////////////////
PT(PandaNode) CollisionPolygon::
get_viz(const CullTraverserData &data, bool bounds_only) const {
  const RenderAttrib *cpa_attrib =
    data._state->get_attrib(ClipPlaneAttrib::get_class_type());
  if (cpa_attrib == (const RenderAttrib *)NULL) {
    // Fortunately, the polygon is not clipped.  This is the normal,
    // easy case.
    return CollisionSolid::get_viz(data, bounds_only);
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "drawing polygon with clip plane " << *cpa_attrib << "\n";
  }

  // The polygon is clipped.  We need to render it clipped.  We could
  // just turn on the ClipPlaneAttrib state and render the full
  // polygon, letting the hardware do the clipping, but we get fancy
  // and clip it by hand instead, just to prove that our clipping
  // algorithm works properly.  This does require some more dynamic
  // work.
  const ClipPlaneAttrib *cpa = DCAST(ClipPlaneAttrib, cpa_attrib);
  Points new_points;
  if (apply_clip_plane(new_points, cpa, data._net_transform)) {
    // All points are behind the clip plane; just draw the original
    // polygon.
    return CollisionSolid::get_viz(data, bounds_only);
  }

  if (new_points.empty()) {
    // All points are in front of the clip plane; draw nothing.
    return NULL;
  }

  // Draw the clipped polygon.
  PT(GeomNode) viz_geom_node = new GeomNode("viz");
  PT(GeomNode) bounds_viz_geom_node = new GeomNode("bounds_viz");
  draw_polygon(viz_geom_node, bounds_viz_geom_node, new_points);

  if (bounds_only) {
    return bounds_viz_geom_node.p();
  } else {
    return viz_geom_node.p();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionPolygon::
output(ostream &out) const {
  out << "cpolygon, (" << get_plane()
      << "), " << _points.size() << " vertices";
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionPolygon::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << (*this) << "\n";
  Points::const_iterator pi;
  for (pi = _points.begin(); pi != _points.end(); ++pi) {
    indent(out, indent_level + 2) << (*pi)._p << "\n";
  }

  LMatrix4f to_3d_mat;
  rederive_to_3d_mat(to_3d_mat);
  out << "In 3-d space:\n";
  PTA_Vertexf verts;
  for (pi = _points.begin(); pi != _points.end(); ++pi) {
    verts.push_back(to_3d((*pi)._p, to_3d_mat));
  }

  PTA_Vertexf::const_iterator vi;
  for (vi = verts.begin(); vi != verts.end(); ++vi) {
    indent(out, indent_level + 2) << (*vi) << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::recompute_bound
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
BoundingVolume *CollisionPolygon::
recompute_bound() {
  // First, get ourselves a fresh, empty bounding volume.
  BoundingVolume *bound = BoundedObject::recompute_bound();
  nassertr(bound != (BoundingVolume*)0L, bound);

  GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, bound);

  // Now actually compute the bounding volume by putting it around all
  // of our vertices.
  LMatrix4f to_3d_mat;
  rederive_to_3d_mat(to_3d_mat);
  pvector<LPoint3f> vertices;
  Points::const_iterator pi;
  for (pi = _points.begin(); pi != _points.end(); ++pi) {
    vertices.push_back(to_3d((*pi)._p, to_3d_mat));
  }

  const LPoint3f *vertices_begin = &vertices[0];
  const LPoint3f *vertices_end = vertices_begin + vertices.size();
  gbv->around(vertices_begin, vertices_end);

  return bound;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::test_intersection_from_sphere
//       Access: Protected, Virtual
//  Description: This is part of the double-dispatch implementation of
//               test_intersection().  It is called when the "from"
//               object is a sphere.
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionPolygon::
test_intersection_from_sphere(const CollisionEntry &entry) const {
  if (_points.size() < 3) {
    return NULL;
  }

  const CollisionSphere *sphere;
  DCAST_INTO_R(sphere, entry.get_from(), 0);

  CPT(TransformState) wrt_space = entry.get_wrt_space();
  CPT(TransformState) wrt_prev_space = entry.get_wrt_prev_space();

  const LMatrix4f &wrt_mat = wrt_space->get_mat();

  LPoint3f orig_center = sphere->get_center() * wrt_mat;
  LPoint3f from_center = orig_center;
  bool moved_from_center = false;

  if (wrt_prev_space != wrt_space) {
    // If we have a delta between the previous position and the
    // current position, we use that to determine some more properties
    // of the collision.
    LPoint3f b = from_center;
    LPoint3f a = sphere->get_center() * wrt_prev_space->get_mat();
    LVector3f delta = b - a;

    // First, there is no collision if the "from" object is definitely
    // moving in the same direction as the plane's normal.
    float dot = delta.dot(get_normal());
    if (dot > 0.1f) {
      return NULL;
    }

    if (IS_NEARLY_ZERO(dot)) {
      // If we're moving parallel to the plane, the sphere is tested
      // at its final point.  Leave it as it is.

    } else {
      // Otherwise, we're moving into the plane; the sphere is tested
      // at the point along its path that is closest to intersecting
      // the plane.  This may be the actual intersection point, or it
      // may be the starting point or the final point.
      float t = -(dist_to_plane(a) / dot);
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

  LVector3f from_radius_v =
    LVector3f(sphere->get_radius(), 0.0f, 0.0f) * wrt_mat;
  float from_radius_2 = from_radius_v.length_squared();
  float from_radius = csqrt(from_radius_2);

  LVector3f normal = (has_effective_normal() && sphere->get_respect_effective_normal()) ? get_effective_normal() : get_normal();
#ifndef NDEBUG
  if (!IS_THRESHOLD_EQUAL(normal.length_squared(), 1.0f, 0.001), NULL) {
    collide_cat.info()
      << "polygon within " << entry.get_into_node_path()
      << " has normal " << normal << " of length " << normal.length()
      << "\n";
    normal.normalize();
  }
#endif

  // The nearest point within the plane to our center is the
  // intersection of the line (center, center - normal) with the plane.
  float dist;
  if (!get_plane().intersects_line(dist, from_center, -get_normal())) {
    // No intersection with plane?  This means the plane's effective
    // normal was within the plane itself.  A useless polygon.
    return NULL;
  }

  if (dist > from_radius || dist < -from_radius) {
    // No intersection with the plane.
    return NULL;
  }

  LPoint2f p = to_2d(from_center - dist * get_normal());
  float edge_dist;

  const ClipPlaneAttrib *cpa = entry.get_into_clip_planes();
  if (cpa != (ClipPlaneAttrib *)NULL) {
    // We have a clip plane; apply it.
    Points new_points;
    if (apply_clip_plane(new_points, cpa, entry.get_into_node_path().get_net_transform())) {
      // All points are behind the clip plane; just do the default
      // test.
      edge_dist = dist_to_polygon(p, _points);

    } else if (new_points.empty()) {
      // The polygon is completely clipped.
      return NULL;

    } else {
      // Test against the clipped polygon.
      edge_dist = dist_to_polygon(p, new_points);
    }

  } else {
    // No clip plane is in effect.  Do the default test. 
    edge_dist = dist_to_polygon(p, _points);
  }

  // Now we have edge_dist, which is the distance from the sphere
  // center to the nearest edge of the polygon, within the polygon's
  // plane.

  if (edge_dist > from_radius) {
    // No intersection; the circle is outside the polygon.
    return NULL;
  }

  // The sphere appears to intersect the polygon.  If the edge is less
  // than from_radius away, the sphere may be resting on an edge of
  // the polygon.  Determine how far the center of the sphere must
  // remain from the plane, based on its distance from the nearest
  // edge.

  float max_dist = from_radius;
  if (edge_dist >= 0.0f) {
    float max_dist_2 = max(from_radius_2 - edge_dist * edge_dist, 0.0f);
    max_dist = csqrt(max_dist_2);
  }

  if (dist > max_dist) {
    // There's no intersection: the sphere is hanging off the edge.
    return NULL;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  float into_depth = max_dist - dist;
  if (moved_from_center) {
    // We have to base the depth of intersection on the sphere's final
    // resting point, not the point from which we tested the
    // intersection.
    float orig_dist;
    get_plane().intersects_line(orig_dist, orig_center, -normal);
    into_depth = max_dist - orig_dist;
  }

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(from_center - normal * dist);
  new_entry->set_interior_point(from_center - normal * (dist + into_depth));

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::test_intersection_from_line
//       Access: Protected, Virtual
//  Description: This is part of the double-dispatch implementation of
//               test_intersection().  It is called when the "from"
//               object is a line.
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionPolygon::
test_intersection_from_line(const CollisionEntry &entry) const {
  if (_points.size() < 3) {
    return NULL;
  }

  const CollisionLine *line;
  DCAST_INTO_R(line, entry.get_from(), 0);

  const LMatrix4f &wrt_mat = entry.get_wrt_mat();

  LPoint3f from_origin = line->get_origin() * wrt_mat;
  LVector3f from_direction = line->get_direction() * wrt_mat;

  float t;
  if (!get_plane().intersects_line(t, from_origin, from_direction)) {
    // No intersection.
    return NULL;
  }

  LPoint3f plane_point = from_origin + t * from_direction;
  LPoint2f p = to_2d(plane_point);

  const ClipPlaneAttrib *cpa = entry.get_into_clip_planes();
  if (cpa != (ClipPlaneAttrib *)NULL) {
    // We have a clip plane; apply it.
    Points new_points;
    apply_clip_plane(new_points, cpa, entry.get_into_node_path().get_net_transform());
    if (new_points.size() < 3) {
      return NULL;
    }
    if (!point_is_inside(p, new_points)) {
      return NULL;
    }

  } else {
    // No clip plane is in effect.  Do the default test.
    if (!point_is_inside(p, _points)) {
      return NULL;
    }
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LVector3f normal = (has_effective_normal() && line->get_respect_effective_normal()) ? get_effective_normal() : get_normal();

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(plane_point);

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::test_intersection_from_ray
//       Access: Protected, Virtual
//  Description: This is part of the double-dispatch implementation of
//               test_intersection().  It is called when the "from"
//               object is a ray.
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionPolygon::
test_intersection_from_ray(const CollisionEntry &entry) const {
  if (_points.size() < 3) {
    return NULL;
  }

  const CollisionRay *ray;
  DCAST_INTO_R(ray, entry.get_from(), 0);

  const LMatrix4f &wrt_mat = entry.get_wrt_mat();

  LPoint3f from_origin = ray->get_origin() * wrt_mat;
  LVector3f from_direction = ray->get_direction() * wrt_mat;

  float t;
  if (!get_plane().intersects_line(t, from_origin, from_direction)) {
    // No intersection.
    return NULL;
  }

  if (t < 0.0f) {
    // The intersection point is before the start of the ray.
    return NULL;
  }

  LPoint3f plane_point = from_origin + t * from_direction;
  LPoint2f p = to_2d(plane_point);

  const ClipPlaneAttrib *cpa = entry.get_into_clip_planes();
  if (cpa != (ClipPlaneAttrib *)NULL) {
    // We have a clip plane; apply it.
    Points new_points;
    apply_clip_plane(new_points, cpa, entry.get_into_node_path().get_net_transform());
    if (new_points.size() < 3) {
      return NULL;
    }
    if (!point_is_inside(p, new_points)) {
      return NULL;
    }

  } else {
    // No clip plane is in effect.  Do the default test.
    if (!point_is_inside(p, _points)) {
      return NULL;
    }
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LVector3f normal = (has_effective_normal() && ray->get_respect_effective_normal()) ? get_effective_normal() : get_normal();

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(plane_point);

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::test_intersection_from_segment
//       Access: Public, Virtual
//  Description: This is part of the double-dispatch implementation of
//               test_intersection().  It is called when the "from"
//               object is a segment.
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionPolygon::
test_intersection_from_segment(const CollisionEntry &entry) const {
  if (_points.size() < 3) {
    return NULL;
  }

  const CollisionSegment *segment;
  DCAST_INTO_R(segment, entry.get_from(), 0);

  const LMatrix4f &wrt_mat = entry.get_wrt_mat();

  LPoint3f from_a = segment->get_point_a() * wrt_mat;
  LPoint3f from_b = segment->get_point_b() * wrt_mat;
  LPoint3f from_direction = from_b - from_a;

  float t;
  if (!get_plane().intersects_line(t, from_a, from_direction)) {
    // No intersection.
    return NULL;
  }

  if (t < 0.0f || t > 1.0f) {
    // The intersection point is before the start of the segment or
    // after the end of the segment.
    return NULL;
  }

  LPoint3f plane_point = from_a + t * from_direction;
  LPoint2f p = to_2d(plane_point);

  const ClipPlaneAttrib *cpa = entry.get_into_clip_planes();
  if (cpa != (ClipPlaneAttrib *)NULL) {
    // We have a clip plane; apply it.
    Points new_points;
    apply_clip_plane(new_points, cpa, entry.get_into_node_path().get_net_transform());
    if (new_points.size() < 3) {
      return NULL;
    }
    if (!point_is_inside(p, new_points)) {
      return NULL;
    }

  } else {
    // No clip plane is in effect.  Do the default test.
    if (!point_is_inside(p, _points)) {
      return NULL;
    }
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LVector3f normal = (has_effective_normal() && segment->get_respect_effective_normal()) ? get_effective_normal() : get_normal();

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(plane_point);

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::fill_viz_geom
//       Access: Protected, Virtual
//  Description: Fills the _viz_geom GeomNode up with Geoms suitable
//               for rendering this solid.
////////////////////////////////////////////////////////////////////
void CollisionPolygon::
fill_viz_geom() {
  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "Recomputing viz for " << *this << "\n";
  }
  draw_polygon(_viz_geom, _bounds_viz_geom, _points);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::compute_vectors
//       Access: Private, Static
//  Description: Now that the _p members of the given points array
//               have been computed, go back and compute all of the _v
//               members.
////////////////////////////////////////////////////////////////////
void CollisionPolygon::
compute_vectors(Points &points) {
  size_t num_points = points.size();
  for (size_t i = 0; i < num_points; i++) {
    points[i]._v = points[(i + 1) % num_points]._p - points[i]._p;
    points[i]._v.normalize();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::draw_polygon
//       Access: Private
//  Description: Fills up the indicated GeomNode with the Geoms to
//               draw the polygon indicated with the given set of 2-d
//               points.
////////////////////////////////////////////////////////////////////
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

  LMatrix4f to_3d_mat;
  rederive_to_3d_mat(to_3d_mat);

  if (use_qpgeom) {
    PT(qpGeomVertexData) vdata = new qpGeomVertexData
      ("collision", qpGeomVertexFormat::get_v3(),
       qpGeom::UH_static);
    qpGeomVertexWriter vertex(vdata, InternalName::get_vertex());

    Points::const_iterator pi;
    for (pi = points.begin(); pi != points.end(); ++pi) {
      vertex.add_data3f(to_3d((*pi)._p, to_3d_mat));
    }
    
    PT(qpGeomTrifans) body = new qpGeomTrifans(qpGeom::UH_static);
    body->add_consecutive_vertices(0, points.size());
    body->close_primitive();

    PT(qpGeomLinestrips) border = new qpGeomLinestrips(qpGeom::UH_static);
    border->add_consecutive_vertices(0, points.size());
    border->add_vertex(0);
    border->close_primitive();

    PT(qpGeom) geom1 = new qpGeom;
    geom1->set_vertex_data(vdata);
    geom1->add_primitive(body);

    PT(qpGeom) geom2 = new qpGeom;
    geom2->set_vertex_data(vdata);
    geom2->add_primitive(border);

    _viz_geom->add_geom(geom1, ((CollisionPolygon *)this)->get_solid_viz_state());
    _viz_geom->add_geom(geom2, ((CollisionPolygon *)this)->get_wireframe_viz_state());

    _bounds_viz_geom->add_geom(geom1, ((CollisionPolygon *)this)->get_solid_bounds_viz_state());
    _bounds_viz_geom->add_geom(geom2, ((CollisionPolygon *)this)->get_wireframe_bounds_viz_state());

  } else {
    PTA_Vertexf verts;
    Points::const_iterator pi;
    for (pi = points.begin(); pi != points.end(); ++pi) {
      verts.push_back(to_3d((*pi)._p, to_3d_mat));
    }
    
    PTA_int lengths;
    lengths.push_back(points.size());
    
    GeomPolygon *polygon = new GeomPolygon;
    polygon->set_coords(verts);
    polygon->set_num_prims(1);
    polygon->set_lengths(lengths);
    
    viz_geom_node->add_geom(polygon, ((CollisionPolygon *)this)->get_solid_viz_state());
    viz_geom_node->add_geom(polygon, ((CollisionPolygon *)this)->get_wireframe_viz_state());
    
    bounds_viz_geom_node->add_geom(polygon, ((CollisionPolygon *)this)->get_solid_bounds_viz_state());
    bounds_viz_geom_node->add_geom(polygon, ((CollisionPolygon *)this)->get_wireframe_bounds_viz_state());
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::point_is_inside
//       Access: Private
//  Description: Returns true if the indicated point is within the
//               polygon's 2-d space, false otherwise.
////////////////////////////////////////////////////////////////////
bool CollisionPolygon::
point_is_inside(const LPoint2f &p, const CollisionPolygon::Points &points) const {
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::dist_to_polygon
//       Access: Private
//  Description: Returns the linear distance from the 2-d point to the
//               nearest part of the polygon defined by the points
//               vector.  The result is negative if the point is
//               within the polygon.
////////////////////////////////////////////////////////////////////
float CollisionPolygon::
dist_to_polygon(const LPoint2f &p, const CollisionPolygon::Points &points) const {

  // We know that that the polygon is convex and is defined with the
  // points in counterclockwise order.  Therefore, we simply compare
  // the signed distance to each line; the answer is the maximum of
  // these.  (This doesn't quite get the right answer if the closest
  // part of the polygon is one of the vertices, but it's close enough
  // for these purposes.)

  float max_dist = dist_to_line(p, points.front()._p, points.front()._v);

  for (size_t i = 1; i < points.size(); i++) {
    max_dist = max(max_dist, dist_to_line(p, points[i]._p, points[i]._v));
  }

  return max_dist;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::setup_points
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionPolygon::
setup_points(const LPoint3f *begin, const LPoint3f *end) {
  int num_points = end - begin;
  nassertv(num_points >= 3);

  _points.clear();

  // Tell the base CollisionPlane class what its plane will be.  To do
  // this, we must first compute the polygon normal.
  LVector3f normal = Normalf::zero();

  // Project the polygon into each of the three major planes and
  // calculate the area of each 2-d projection.  This becomes the
  // polygon normal.  This works because the ratio between these
  // different areas corresponds to the angle at which the polygon is
  // tilted toward each plane.
  for (int i = 0; i < num_points; i++) {
    const LPoint3f &p0 = begin[i];
    const LPoint3f &p1 = begin[(i + 1) % num_points];
    normal[0] += p0[1] * p1[2] - p0[2] * p1[1];
    normal[1] += p0[2] * p1[0] - p0[0] * p1[2];
    normal[2] += p0[0] * p1[1] - p0[1] * p1[0];
  }

  if (IS_NEARLY_ZERO(normal.length_squared())) {
    // The polygon has no area.
    return;
  }

#ifndef NDEBUG
  // Make sure all the source points are good.
  {
    if (!verify_points(begin, end)) {
      collide_cat.error() << "Invalid points in CollisionPolygon:\n";
      const LPoint3f *pi;
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
    const LPoint3f *pi;
    for (pi = begin; pi != end; ++pi) {
      collide_cat.spam(false) << "  " << (*pi) << "\n";
    }
  }
#endif

  set_plane(Planef(normal, begin[0]));

  // Construct a matrix that rotates the points from the (X,0,Z) plane
  // into the 3-d plane.
  LMatrix4f to_3d_mat;
  calc_to_3d_mat(to_3d_mat);

  // And the inverse matrix rotates points from 3-d space into the 2-d
  // plane.
  _to_2d_mat.invert_from(to_3d_mat);

  // Now project all of the points onto the 2-d plane.

  const LPoint3f *pi;
  for (pi = begin; pi != end; ++pi) {
    LPoint3f point = (*pi) * _to_2d_mat;
    _points.push_back(PointDef(point[0], point[2]));
  }

  nassertv(_points.size() >= 3);

#ifndef NDEBUG
  /*
  // Now make sure the points define a convex polygon.
  if (is_concave()) {
    collide_cat.error() << "Invalid concave CollisionPolygon defined:\n";
    const LPoint3f *pi;
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::legacy_to_3d
//       Access: Private
//  Description: Converts the indicated point to 3-d space according
//               to the way CollisionPolygons used to be stored in bam
//               files prior to 4.9.
////////////////////////////////////////////////////////////////////
LPoint3f CollisionPolygon::
legacy_to_3d(const LVecBase2f &point2d, int axis) const {
  nassertr(!point2d.is_nan(), LPoint3f(0.0f, 0.0f, 0.0f));

  LVector3f normal = get_normal();
  float D = get_plane()[3];

  nassertr(!normal.is_nan(), LPoint3f(0.0f, 0.0f, 0.0f));
  nassertr(!cnan(D), LPoint3f(0.0f, 0.0f, 0.0f));

  switch (axis) {
  case 0:  // AT_x:
    return LPoint3f(-(normal[1]*point2d[0] + normal[2]*point2d[1] + D)/normal[0],                    point2d[0], point2d[1]);

  case 1:  // AT_y:
    return LPoint3f(point2d[0],
                    -(normal[0]*point2d[0] + normal[2]*point2d[1] + D)/normal[1],                    point2d[1]);

  case 2:  // AT_z:
    return LPoint3f(point2d[0], point2d[1],
                    -(normal[0]*point2d[0] + normal[1]*point2d[1] + D)/normal[2]);
  }

  nassertr(false, LPoint3f(0.0f, 0.0f, 0.0f));
  return LPoint3f(0.0f, 0.0f, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::clip_polygon
//       Access: Private
//  Description: Clips the source_points of the polygon by the
//               indicated clipping plane, and modifies new_points to
//               reflect the new set of clipped points (but does not
//               compute the vectors in new_points).
//
//               The return value is true if the set of points is
//               unmodified (all points are behind the clip plane), or
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool CollisionPolygon::
clip_polygon(CollisionPolygon::Points &new_points, 
             const CollisionPolygon::Points &source_points,
             const Planef &plane) const {
  new_points.clear();
  if (source_points.empty()) {
    return true;
  }

  LPoint3f from3d;
  LVector3f delta3d;
  if (!plane.intersects_plane(from3d, delta3d, get_plane())) {
    // The clipping plane is parallel to the polygon.  The polygon is
    // either all in or all out.
    if (plane.dist_to_plane(get_plane().get_point()) < 0.0) {
      // A point within the polygon is behind the clipping plane: the
      // polygon is all in.
      new_points = source_points;
      return true;
    }
    return false;
  }

  // Project the line of intersection into the 2-d plane.  Now we have
  // a 2-d clipping line.
  LPoint2f from2d = to_2d(from3d);
  LVector2f delta2d = to_2d(delta3d);

  float a = -delta2d[1];
  float b = delta2d[0];
  float c = from2d[0] * delta2d[1] - from2d[1] * delta2d[0];

  // Now walk through the points.  Any point on the left of our line
  // gets removed, and the line segment clipped at the point of
  // intersection.

  // We might increase the number of vertices by as many as 1, if the
  // plane clips off exactly one corner.  (We might also decrease the
  // number of vertices, or keep them the same number.)
  new_points.reserve(source_points.size() + 1);

  LPoint2f last_point = source_points.back()._p;
  bool last_is_in = !is_right(last_point - from2d, delta2d);
  bool all_in = last_is_in;
  Points::const_iterator pi;
  for (pi = source_points.begin(); pi != source_points.end(); ++pi) {
    const LPoint2f &this_point = (*pi)._p;
    bool this_is_in = !is_right(this_point - from2d, delta2d);

    if (this_is_in != last_is_in) {
      // We have just crossed over the clipping line.  Find the point
      // of intersection.
      LVector2f d = this_point - last_point;
      float t = -(a * last_point[0] + b * last_point[1] + c) / (a * d[0] + b * d[1]);
      LPoint2f p = last_point + t * d;

      new_points.push_back(PointDef(p[0], p[1]));
      last_is_in = this_is_in;
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::apply_clip_plane
//       Access: Private
//  Description: Clips the polygon by all of the clip planes named in
//               the clip plane attribute and fills new_points up with
//               the resulting points.
//
//               The return value is true if the set of points is
//               unmodified (all points are behind all the clip
//               planes), or false otherwise.
////////////////////////////////////////////////////////////////////
bool CollisionPolygon::
apply_clip_plane(CollisionPolygon::Points &new_points, 
                 const ClipPlaneAttrib *cpa,
                 const TransformState *net_transform) const {
  bool all_in = true;

  int num_planes = cpa->get_num_planes();
  if (num_planes > 0) {
    PlaneNode *plane_node = cpa->get_plane(0);
    NodePath plane_path(plane_node);
    CPT(TransformState) new_transform = 
      net_transform->invert_compose(plane_path.get_net_transform());
    
    Planef plane = plane_node->get_plane() * new_transform->get_mat();
    if (!clip_polygon(new_points, _points, plane)) {
      all_in = false;
    }

    for (int i = 1; i < num_planes; i++) {
      PlaneNode *plane_node = cpa->get_plane(i);
      NodePath plane_path(plane_node);
      CPT(TransformState) new_transform = 
        net_transform->invert_compose(plane_path.get_net_transform());
      
      Planef plane = plane_node->get_plane() * new_transform->get_mat();
      Points last_points;
      last_points.swap(new_points);
      if (!clip_polygon(new_points, last_points, plane)) {
        all_in = false;
      }
    }
  }

  if (!all_in) {
    compute_vectors(new_points);
  }

  return all_in;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void CollisionPolygon::
fillin(DatagramIterator &scan, BamReader *manager) {
  CollisionPlane::fillin(scan, manager);
  if (manager->get_file_minor_ver() < 9) {
    // Decode old-style collision polygon.

    Points points;
    size_t size = scan.get_uint16();
    for (size_t i = 0; i < size; i++) {
      LPoint2f p;
      p.read_datagram(scan);
      
      points.push_back(PointDef(p[0], p[1]));
    }
    LPoint2f median;
    median.read_datagram(scan);

    int axis = scan.get_uint8();
    bool reversed = (scan.get_uint8() != 0);

    // Now convert the above points into 3-d by the old-style rules.
    pvector<LPoint3f> verts;
    Points::const_iterator pi;
    for (pi = points.begin(); pi != points.end(); ++pi) {
      verts.push_back(legacy_to_3d((*pi)._p, axis));
    }
    if (reversed) {
      reverse(verts.begin(), verts.end());
    }

    const LPoint3f *verts_begin = &verts[0];
    const LPoint3f *verts_end = verts_begin + verts.size();
    setup_points(verts_begin, verts_end);

  } else {
    // Load new-style collision polygon.
    size_t size = scan.get_uint16();
    for (size_t i = 0; i < size; i++) {
      LPoint2f p;
      LVector2f v;
      p.read_datagram(scan);
      v.read_datagram(scan);
      _points.push_back(PointDef(p, v));
    }
    _to_2d_mat.read_datagram(scan);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::make_CollisionPolygon
//       Access: Protected
//  Description: Factory method to generate a CollisionPolygon object
////////////////////////////////////////////////////////////////////
TypedWritable* CollisionPolygon::
make_CollisionPolygon(const FactoryParams &params) {
  CollisionPolygon *me = new CollisionPolygon;
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
void CollisionPolygon::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_CollisionPolygon);
}


