// Filename: collisionBox.cxx
// Created by:  amith tudur (31Jul09)
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

#include "collisionBox.h"
#include "collisionLine.h"
#include "collisionRay.h"
#include "collisionSphere.h"
#include "collisionSegment.h"
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid *CollisionBox::
make_copy() {
  return new CollisionBox(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::setup_box
//       Access: Public, Virtual
//  Description: Compute parameters for each of the box's sides
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::setup_points
//       Access: Private
//  Description: Computes the plane and 2d projection of points that
//               make up this side.
////////////////////////////////////////////////////////////////////
void CollisionBox::
setup_points(const LPoint3 *begin, const LPoint3 *end, int plane) {
  int num_points = end - begin;
  nassertv(num_points >= 3);

  _points[plane].clear();

  // Construct a matrix that rotates the points from the (X,0,Z) plane
  // into the 3-d plane.
  LMatrix4 to_3d_mat;
  calc_to_3d_mat(to_3d_mat, plane);

  // And the inverse matrix rotates points from 3-d space into the 2-d
  // plane.
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::test_intersection
//       Access: Public, Virtual
//  Description: First Dispatch point for box as a FROM object
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionBox::
test_intersection(const CollisionEntry &entry) const {
  return entry.get_into()->test_intersection_from_box(entry);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::xform
//       Access: Public, Virtual
//  Description: Transforms the solid by the indicated matrix.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::get_collision_origin
//       Access: Public, Virtual
//  Description: Returns the point in space deemed to be the "origin"
//               of the solid for collision purposes.  The closest
//               intersection point to this origin point is considered
//               to be the most significant.
////////////////////////////////////////////////////////////////////
LPoint3 CollisionBox::
get_collision_origin() const {
  return _center;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::get_min
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
LPoint3 CollisionBox::
get_min() const {
  return _min;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::get_max
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
LPoint3 CollisionBox::
get_max() const {
  return _max;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::get_approx_center
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
LPoint3 CollisionBox::
get_approx_center() const {
  return (_min + _max) * 0.5f;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::get_volume_pcollector
//       Access: Public, Virtual
//  Description: Returns a PStatCollector that is used to count the
//               number of bounding volume tests made against a solid
//               of this type in a given frame.
////////////////////////////////////////////////////////////////////
PStatCollector &CollisionBox::
get_volume_pcollector() {
  return _volume_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::get_test_pcollector
//       Access: Public, Virtual
//  Description: Returns a PStatCollector that is used to count the
//               number of intersection tests made against a solid
//               of this type in a given frame.
////////////////////////////////////////////////////////////////////
PStatCollector &CollisionBox::
get_test_pcollector() {
  return _test_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionBox::
output(ostream &out) const {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::compute_internal_bounds
//       Access: Protected, Virtual
//  Description: Sphere is chosen as the Bounding Volume type for 
//               speed and efficiency
////////////////////////////////////////////////////////////////////
PT(BoundingVolume) CollisionBox::
compute_internal_bounds() const {
  return new BoundingSphere(_center, _radius);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::test_intersection_from_sphere
//       Access: Public, Virtual
//  Description: Double dispatch point for sphere as FROM object
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionBox::
test_intersection_from_sphere(const CollisionEntry &entry) const {

  const CollisionSphere *sphere;
  DCAST_INTO_R(sphere, entry.get_from(), 0);

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
      // If we have a delta between the previous position and the
      // current position, we use that to determine some more properties
      // of the collision.
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
        // If we're moving parallel to the plane, the sphere is tested
        // at its final point.  Leave it as it is.

      } else {
        // Otherwise, we're moving into the plane; the sphere is tested
        // at the point along its path that is closest to intersecting
        // the plane.  This may be the actual intersection point, or it
        // may be the starting point or the final point.
        // dot is equal to the (negative) magnitude of 'delta' along the
        // direction of the plane normal
        // t = ratio of (distance from start pos to plane) to (distance
        // from start pos to end pos), along axis of plane normal
        PN_stdfloat dist_to_p = plane.dist_to_plane(a);
        t = (dist_to_p / -dot);
            
        // also compute the actual contact point and time of contact
        // for handlers that need it
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

    // The nearest point within the plane to our center is the
    // intersection of the line (center, center - normal) with the plane.
    
    if (!plane.intersects_line(dist, from_center, -(plane.get_normal()))) {
      // No intersection with plane?  This means the plane's effective
      // normal was within the plane itself.  A useless polygon.
      continue;
    }

    if (dist > from_radius || dist < -from_radius) {
      // No intersection with the plane.
      continue;
    }

    LPoint2 p = to_2d(from_center - dist * plane.get_normal(), ip);
    PN_stdfloat edge_dist = 0.0f;

    const ClipPlaneAttrib *cpa = entry.get_into_clip_planes();
    if (cpa != (ClipPlaneAttrib *)NULL) {
      // We have a clip plane; apply it.
      Points new_points;
      if (apply_clip_plane(new_points, cpa, entry.get_into_node_path().get_net_transform(),ip)) {
        // All points are behind the clip plane; just do the default
        // test.
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

    // Now we have edge_dist, which is the distance from the sphere
    // center to the nearest edge of the polygon, within the polygon's
    // plane. edge_dist<0 means the point is within the polygon.
    if(edge_dist < 0) {
      intersect = true;
      continue;
    }

    if((edge_dist > 0) && 
       ((edge_dist * edge_dist + dist * dist) > from_radius_2)) {
      // No intersection; the circle is outside the polygon.
      continue;
    }

    // The sphere appears to intersect the polygon.  If the edge is less
    // than from_radius away, the sphere may be resting on an edge of
    // the polygon.  Determine how far the center of the sphere must
    // remain from the plane, based on its distance from the nearest
    // edge.

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
    return NULL;

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  PN_stdfloat into_depth = max_dist - dist;
  if (moved_from_center) {
    // We have to base the depth of intersection on the sphere's final
    // resting point, not the point from which we tested the
    // intersection.
    PN_stdfloat orig_dist;
    plane.intersects_line(orig_dist, orig_center, -normal);
    into_depth = max_dist - orig_dist;
  }

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(from_center - normal * dist);
  new_entry->set_interior_point(from_center - normal * (dist + into_depth));
  new_entry->set_contact_pos(contact_point);
  new_entry->set_contact_normal(plane.get_normal());
  new_entry->set_t(actual_t);

  return new_entry;
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::test_intersection_from_ray
//       Access: Public, Virtual
//  Description: Double dispatch point for ray as a FROM object
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionBox::
test_intersection_from_ray(const CollisionEntry &entry) const {
  const CollisionRay *ray;
  DCAST_INTO_R(ray, entry.get_from(), 0);
  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_origin = ray->get_origin() * wrt_mat;
  LVector3 from_direction = ray->get_direction() * wrt_mat;

  int i, j;
  PN_stdfloat t;
  PN_stdfloat near_t = 0.0;
  bool intersect;
  LPlane plane;
  LPlane near_plane; 

  //Returns the details about the first plane of the box that the ray
  //intersects.
  for (i = 0, intersect = false, t = 0, j = 0; i < 6 && j < 2; i++) {
    plane = get_plane(i);

    if (!plane.intersects_line(t, from_origin, from_direction)) {
      // No intersection.  The ray is parallel to the plane.
      continue;
    }

    if (t < 0.0f) {
      // The intersection point is before the start of the ray, and so
      // the ray is entirely in front of the plane.
      continue;
    }
    LPoint3 plane_point = from_origin + t * from_direction;
    LPoint2 p = to_2d(plane_point, i);

    if (!point_is_inside(p, _points[i])){
      continue;
    }
    intersect = true;
    if (j) {
      if(t < near_t) {
        near_plane = plane;
        near_t = t;
      }
    }
    else {
      near_plane = plane;
      near_t = t;
    }
    ++j;
  }
   

  if(!intersect) {
    //No intersection with ANY of the box's planes has been detected
    return NULL;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }

  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3 into_intersection_point = from_origin + near_t * from_direction;

  LVector3 normal =
    (has_effective_normal() && ray->get_respect_effective_normal()) 
    ? get_effective_normal() : near_plane.get_normal();

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(into_intersection_point);

  return new_entry;
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::test_intersection_from_segment
//       Access: Public, Virtual
//  Description: Double dispatch point for segment as a FROM object
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionBox::
test_intersection_from_segment(const CollisionEntry &entry) const {
  const CollisionSegment *seg;
  DCAST_INTO_R(seg, entry.get_from(), 0);
  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_origin = seg->get_point_a() * wrt_mat;
  LPoint3 from_extent = seg->get_point_b() * wrt_mat;
  LVector3 from_direction = from_extent - from_origin;

  int i, j;
  PN_stdfloat t;
  PN_stdfloat near_t = 0.0;
  bool intersect;
  LPlane plane;
  LPlane near_plane;

  //Returns the details about the first plane of the box that the
  //segment intersects.
  for(i = 0, intersect = false, t = 0, j = 0; i < 6 && j < 2; i++) {
    plane = get_plane(i);

    if (!plane.intersects_line(t, from_origin, from_direction)) {
      // No intersection.  The segment is parallel to the plane.
      continue;
    }

    if (t < 0.0f || t > 1.0f) {
      // The intersection point is before the start of the segment,
      // or after the end of the segment, so the segment is either
      // entirely in front of or behind the plane.
      continue;
    }
    LPoint3 plane_point = from_origin + t * from_direction;
    LPoint2 p = to_2d(plane_point, i);

    if (!point_is_inside(p, _points[i])){
      continue;
    }
    intersect = true;
    if(j) {
      if(t < near_t) {
        near_plane = plane;
        near_t = t;
      }
    }
    else {
      near_plane = plane;
      near_t = t;
    }
    ++j;
  }

  if(!intersect) {
    //No intersection with ANY of the box's planes has been detected
    return NULL;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }

  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3 into_intersection_point = from_origin + near_t * from_direction;

  LVector3 normal =
    (has_effective_normal() && seg->get_respect_effective_normal()) 
    ? get_effective_normal() : near_plane.get_normal();

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(into_intersection_point);

  return new_entry;
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::test_intersection_from_box
//       Access: Public, Virtual
//  Description: Double dispatch point for box as a FROM object
//               NOT Implemented.
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionBox::
test_intersection_from_box(const CollisionEntry &entry) const {
  return NULL;
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::fill_viz_geom
//       Access: Protected, Virtual
//  Description: Fills the _viz_geom GeomNode up with Geoms suitable
//               for rendering this solid.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::apply_clip_plane
//       Access: Private
//  Description: Clips the polygon by all of the clip planes named in
//               the clip plane attribute and fills new_points up with
//               the resulting points.
//
//               The return value is true if the set of points is
//               unmodified (all points are behind all the clip
//               planes), or false otherwise.
////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::clip_polygon
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
    // The clipping plane is parallel to the polygon.  The polygon is
    // either all in or all out.
    if (plane.dist_to_plane(get_plane(plane_no).get_point()) < 0.0) {
      // A point within the polygon is behind the clipping plane: the
      // polygon is all in.
      new_points = source_points;
      return true;
    }
    return false;
  }

  // Project the line of intersection into the 2-d plane.  Now we have
  // a 2-d clipping line.
  LPoint2 from2d = to_2d(from3d,plane_no);
  LVector2 delta2d = to_2d(delta3d,plane_no);

  PN_stdfloat a = -delta2d[1];
  PN_stdfloat b = delta2d[0];
  PN_stdfloat c = from2d[0] * delta2d[1] - from2d[1] * delta2d[0];

  // Now walk through the points.  Any point on the left of our line
  // gets removed, and the line segment clipped at the point of
  // intersection.

  // We might increase the number of vertices by as many as 1, if the
  // plane clips off exactly one corner.  (We might also decrease the
  // number of vertices, or keep them the same number.)
  new_points.reserve(source_points.size() + 1);

  LPoint2 last_point = source_points.back()._p;
  bool last_is_in = !is_right(last_point - from2d, delta2d);
  bool all_in = last_is_in;
  Points::const_iterator pi;
  for (pi = source_points.begin(); pi != source_points.end(); ++pi) {
    const LPoint2 &this_point = (*pi)._p;
    bool this_is_in = !is_right(this_point - from2d, delta2d);

    // There appears to be a compiler bug in gcc 4.0: we need to
    // extract this comparison outside of the if statement.
    bool crossed_over = (this_is_in != last_is_in);
    if (crossed_over) {
      // We have just crossed over the clipping line.  Find the point
      // of intersection.
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


////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::
//       Access: Private
//  Description: Returns the linear distance from the 2-d point to the
//               nearest part of the polygon defined by the points
//               vector.  The result is negative if the point is
//               within the polygon.
////////////////////////////////////////////////////////////////////
PN_stdfloat CollisionBox::
dist_to_polygon(const LPoint2 &p, const CollisionBox::Points &points) const {

  // We know that that the polygon is convex and is defined with the
  // points in counterclockwise order.  Therefore, we simply compare
  // the signed distance to each line segment; we ignore any negative
  // values, and take the minimum of all the positive values.  

  // If all values are negative, the point is within the polygon; we
  // therefore return an arbitrary negative result.
  
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::dist_to_line_segment
//       Access: Private, Static
//  Description: Returns the linear distance of p to the line segment
//               defined by f and t, where v = (t - f).normalize().
//               The result is negative if p is left of the line,
//               positive if it is right of the line.  If the result
//               is positive, it is constrained by endpoints of the
//               line segment (i.e. the result might be larger than it
//               would be for a straight distance-to-line test).  If
//               the result is negative, we don't bother.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::point_is_inside
//       Access: Private
//  Description: Returns true if the indicated point is within the
//               polygon's 2-d space, false otherwise.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::compute_vectors
//       Access: Private, Static
//  Description: Now that the _p members of the given points array
//               have been computed, go back and compute all of the _v
//               members.
////////////////////////////////////////////////////////////////////
void CollisionBox::
compute_vectors(Points &points) {
  size_t num_points = points.size();
  for (size_t i = 0; i < num_points; i++) {
    points[i]._v = points[(i + 1) % num_points]._p - points[i]._p;
    points[i]._v.normalize();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a CollisionBox object
////////////////////////////////////////////////////////////////////
void CollisionBox::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_CollisionBox);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::make_CollisionBox
//       Access: Protected
//  Description: Factory method to generate a CollisionBox object
////////////////////////////////////////////////////////////////////
TypedWritable *CollisionBox::
make_CollisionBox(const FactoryParams &params) {
  CollisionBox *me = new CollisionBox;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
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
