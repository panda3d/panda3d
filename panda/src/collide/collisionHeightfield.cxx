/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHeightfield.cxx
 * @author hecris
 * @date 2019-07-01
 */

#include "collisionBox.h"
#include "collisionRay.h"
#include "collisionSphere.h"
#include "collisionHeightfield.h"
#include "collisionHandler.h"
#include "collisionEntry.h"
#include "cmath.h"
#include "config_collide.h"
#include "boundingBox.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "geom.h"
#include "geomTriangles.h"
#include "geomVertexWriter.h"
#include <queue>
#include <algorithm>

using std::min;
using std::max;
using std::queue;
using std::vector;
using std::sort;

PStatCollector CollisionHeightfield::_volume_pcollector(
      "Collision Volumes:CollisionHeightfield");
PStatCollector CollisionHeightfield::_test_pcollector(
      "Collision Tests:CollisionHeightfield");
TypeHandle CollisionHeightfield::_type_handle;

/**
 *
 */
CollisionHeightfield::
CollisionHeightfield(PNMImage heightfield,
                     PN_stdfloat max_height, int num_subdivisions) {
  _heightfield = heightfield;
  _max_height = max_height;
  _nodes_count = 0;

  set_num_subdivisions(num_subdivisions);
}

/**
 * Sets the number of quadtree subdivisions and modifies
 * the quadtree accordingly. This should be called when a
 * user wants to modify the number of quadtree subdivisions
 * or from a constructor to initialize the quadtree.
 *
 * If the number of subdivisions is too high, it will
 * automatically be decremented.
 */
void CollisionHeightfield::
set_num_subdivisions(int num_subdivisions) {
  // The number of subdivisions should not be negative
  nassertv(num_subdivisions >= 0 && num_subdivisions <= 10);
  // Determine the number of quadtree nodes needed
  // for the corresponding number of subdivisions.
  int nodes_count = 0;
  for (int i = 0; i <= num_subdivisions; i++) {
    nodes_count += 1 << (i * 2);
  }
  if (nodes_count == _nodes_count) {
    // No changes to quadtree to be done
    return;
  }
  // Calculate the index of the first quad tree leaf node
  int leaf_first_index = nodes_count - pow(4, num_subdivisions);
  // Calculate the area of a leaf node in the quadtree
  PN_stdfloat heightfield_area = _heightfield.get_x_size() *
                                 _heightfield.get_y_size();
  nassertv(heightfield_area > 0);
  PN_stdfloat num_leafs = nodes_count - leaf_first_index;
  // If the area is too small (less than 1), then we
  // retry by decrementing the number of subdivisions.
  if (heightfield_area / num_leafs < 1) {
    set_num_subdivisions(num_subdivisions - 1);
    return;
  }
  if (nodes_count < _nodes_count) {
    // If the user is decreasing the number of subdivisions, then
    // we need only to update the index where the quadtree leaves start.
    _leaf_first_index = leaf_first_index;
  } else {
    // Otherwise we need to build a new quadtree
    if (_nodes_count != 0) {
      // There is an existing quadtree, delete it first
      delete[] _nodes;
      _nodes = nullptr;
    }
    QuadTreeNode *nodes = new QuadTreeNode[nodes_count];
    _nodes = nodes;
    _nodes_count = nodes_count;
    _leaf_first_index = leaf_first_index;
    fill_quadtree_areas();
    fill_quadtree_heights();
  }
  _num_subdivisions = num_subdivisions;
}

/**
 *
 */
void CollisionHeightfield::
fill_quadtree_areas() {
  nassertv(_heightfield.get_x_size() > 0 &&
           _heightfield.get_y_size() > 0);

  _nodes[0].area.min = {0, 0};
  _nodes[0].area.max = {(float)_heightfield.get_x_size(),
                        (float)_heightfield.get_y_size()};
  _nodes[0].index = 0;
  QuadTreeNode parent;
  for (int i = 1; i < _nodes_count; i += 4) {
    parent = _nodes[(i-1) / 4];
    LVector2 sub_area = (parent.area.max - parent.area.min) / 2;
    // SE Quadrant
    _nodes[i].area.min = parent.area.min;
    _nodes[i].area.max = parent.area.min + sub_area;
    _nodes[i].index = i;
    // SW Quadrant
    _nodes[i + 1].area.min = {parent.area.min[0] + sub_area[0],
                              parent.area.min[1]};
    _nodes[i + 1].area.max = _nodes[i + 1].area.min + sub_area;
    _nodes[i + 1].index = i + 1;
    // NE Quadrant
    _nodes[i + 2].area.min = {parent.area.min[0],
                              parent.area.min[1] + sub_area[1]};
    _nodes[i + 2].area.max = _nodes[i + 2].area.min + sub_area;
    _nodes[i + 2].index = i + 2;
    // NW Quadrant
    _nodes[i + 3].area.min = parent.area.min + sub_area;
    _nodes[i + 3].area.max = parent.area.max;
    _nodes[i + 3].index = i + 3;
  }
}

/**
 * Processes the heightfield image, setting the height values
 * of each quadtree node.
 *
 * @relates set_max_height
 * @relates set_heightfield
 */
void CollisionHeightfield::
fill_quadtree_heights() {
  QuadTreeNode node;
  QuadTreeNode child;
  for (int i = _nodes_count - 1; i >= 0; i--) {
    node = _nodes[i];
    PN_stdfloat height_min = INT_MAX;
    PN_stdfloat height_max = INT_MIN;
    if (i >= _leaf_first_index) {
      for (int x = node.area.min[0]; x < node.area.max[0]; x++) {
        for (int y = node.area.min[1]; y < node.area.max[1]; y++) {
          PN_stdfloat value = _heightfield.get_gray(x,
            _heightfield.get_y_size() - 1 - y) * _max_height;

          height_min = min(value, height_min);
          height_max = max(value, height_max);
        }
      }
    } else {
      for (int c = (i * 4) + 1, cmax = c + 4; c < cmax; c++) {
        child = _nodes[c];
        height_min = min(child.height_min, height_min);
        height_max = max(child.height_max, height_max);
      }
    }
    _nodes[i].height_min = height_min;
    _nodes[i].height_max = height_max;
  }
}

/**
 *
 */
PT(CollisionEntry) CollisionHeightfield::
test_intersection_from_ray(const CollisionEntry &entry) const {
  const CollisionRay *ray;
  DCAST_INTO_R(ray, entry.get_from(), nullptr);
  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_origin = ray->get_origin() * wrt_mat;
  LPoint3 from_direction = ray->get_direction() * wrt_mat;

  IntersectionParams params;
  params.from_origin = from_origin;
  params.from_direction = from_direction;
  vector<QuadTreeIntersection> intersections;
  intersections = find_intersections(line_intersects_box, params);

  if (intersections.empty()) {
    return nullptr;
  }
  // Sort intersections by their t1 values so we can return
  // the first intersection found.
  sort(intersections.begin(), intersections.end());

  double t1, t2;
  for (size_t i = 0; i < intersections.size(); i++) {
    t1 = intersections[i].tmin;
    t2 = intersections[i].tmax;

    if (t1 < 0.0 && t2 < 0.0) continue;
    t1 = max(t1, 0.0);

    LPoint3 p1 = from_origin + from_direction * t1;
    LPoint3 p2 = from_origin + from_direction * t2;
    // We use Bresenhaum's Line Algorithm to directly get the heightfield
    // coordinates that the line passes through.
    int err = cabs(2 * (p2[1] - p1[1]));
    int deltaerr = cabs(err - (p2[0] - p1[0]));

    int x = p1[0], y = p1[1];
    bool x_increasing = x < p2[0];
    bool y_increasing = y < p2[1];
    while ((x_increasing && x <= p2[0]) || (!x_increasing && x >= p2[0])) {
      bool intersected = false;
      Triangle int_tri;
      vector<Triangle> triangles = get_triangles(x, y);
      double min_t = DBL_MAX;
      for (Triangle tri : triangles) {
        double t;
        if (line_intersects_triangle(t, from_origin, from_direction, tri)) {
          if (t < 0) continue;
          if (t < min_t) {
            intersected = true;
            min_t = t;
            int_tri = tri;
          }
        }
      }
      // If the ray intersects this heightfield element, we can
      // stop here and return the intersection.
      if (intersected) {
        PT(CollisionEntry) new_entry = new CollisionEntry(entry);
        new_entry->set_surface_point(from_origin + from_direction * min_t);
        LPlane p = LPlane(int_tri.p1, int_tri.p2, int_tri.p3);
        LVector3 normal = p.get_normal();
        if (p.dist_to_plane(from_origin) < 0.0f) normal *= -1;
        new_entry->set_surface_normal(normal);
        return new_entry;
      }
      // Otherwise, go to the next heightfield element
      deltaerr += err;
      if (deltaerr >= 0) {
        if (y_increasing) y++;
        else y--;
        deltaerr -= 2 * (p2[0] - p1[0]);
      }
      if (x_increasing) x++;
      else x--;
    }
  }

  return nullptr;
}

/**
 *
 */
PT(CollisionEntry) CollisionHeightfield::
test_intersection_from_sphere(const CollisionEntry &entry) const {
  const CollisionSphere *sphere;
  DCAST_INTO_R(sphere, entry.get_from(), nullptr);

  CPT(TransformState) wrt_space = entry.get_wrt_space();
  const LMatrix4 &wrt_mat = wrt_space->get_mat();

  LPoint3 center = sphere->get_center() * wrt_mat;
  LVector3 radius_v = LVector3(sphere->get_radius(), 0.0f, 0.0f) * wrt_mat;
  PN_stdfloat radius_2 = radius_v.length_squared();
  PN_stdfloat radius = csqrt(radius_2);

  IntersectionParams params;
  params.center = center;
  params.radius = radius;

  vector<QuadTreeIntersection> intersections;
  intersections= find_intersections(sphere_intersects_box, params);

  if (intersections.empty()) {
    return nullptr;
  }

  LPoint3 point;
  LPoint3 closest_point;
  PN_stdfloat dist;
  PN_stdfloat dist_min = FLT_MAX;

  bool intersected = false;
  for (size_t i = 0; i < intersections.size(); i++) {
    QuadTreeNode node = _nodes[intersections[i].node_index];
    // Iterate through the circle's area and find triangle intersections,
    // find the one closest to the center of the sphere.
    for (int dx = -radius; dx <= radius; dx++) {
      for (int dy = -radius; dy <= radius; dy++) {
        int x = dx + center[0];
        int y = dy + center[1];
        if (x < node.area.min[0] || y < node.area.min[1] ||
            x > node.area.max[0] || y > node.area.max[1]) {
          // point not contained in rectangle
          continue;
        }
        if (dx * dx + dy * dy > radius_2) continue;

        vector<Triangle> triangles = get_triangles(x, y);
        for (Triangle tri : triangles) {
          point = closest_point_on_triangle(center, tri);
          dist = (point - center).length_squared();
          if (dist > radius_2) continue;
          if (dist < dist_min) {
            intersected = true;
            dist_min = dist;
            closest_point = point;
          }
        }
      }
    }
  }
  if (intersected) {
    PT(CollisionEntry) new_entry = new CollisionEntry(entry);
    LVector3 v(center - closest_point);
    PN_stdfloat v_length = v.length();
    if (IS_NEARLY_ZERO(v_length)) {
      v.set(1, 0, 0);
    } else {
      v /= v_length;
    }
    new_entry->set_surface_point(closest_point);
    new_entry->set_surface_normal(v);
    new_entry->set_interior_point(center - radius * v);
    return new_entry;
  } else {
    return nullptr;
  }
}

/**
 *
 */
PT(CollisionEntry) CollisionHeightfield::
test_intersection_from_box(const CollisionEntry &entry) const {
  const CollisionBox *box;
  DCAST_INTO_R(box, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();
  LPoint3 box_min = box->get_min() * wrt_mat;
  LPoint3 box_max = box->get_max() * wrt_mat;

  IntersectionParams params;
  params.box_min = box_min;
  params.box_max = box_max;

  vector<QuadTreeIntersection> intersections;
  intersections = find_intersections(box_intersects_box, params);

  if (intersections.empty()) {
    return nullptr;
  }

  bool intersected = false;
  Triangle intersected_tri;
  for (size_t i = 0; i < intersections.size(); i++) {
    QuadTreeNode node = _nodes[intersections[i].node_index];
    // Find the overlapping rectangle between the two boxes and
    // test the heightfield elements in that area.
    LVecBase2 overlap_min = LVecBase2(max(box_min[0], node.area.min[0]),
                                      max(box_min[1], node.area.min[1]));
    LVecBase2 overlap_max = LVecBase2(min(box_max[0], node.area.max[0]),
                                      min(box_max[1], node.area.max[1]));
    for (int x = overlap_min[0]; x < overlap_max[0]; x++) {
      for (int y = overlap_min[1]; y < overlap_max[1]; y++) {
        vector<Triangle> triangles = get_triangles(x, y);
        for (Triangle tri : triangles) {
          if (box_intersects_triangle(box_min, box_max, tri)) {
            intersected = true;
            intersected_tri = tri;
          }
        }
      }
    }
  }

  if (intersected) {
    PT(CollisionEntry) new_entry = new CollisionEntry(entry);
    LPlane p = LPlane(intersected_tri.p1, intersected_tri.p2, intersected_tri.p3);
    LVector3 normal = p.get_normal();
    if (p.dist_to_plane(box->get_center() * wrt_mat) < 0.0f) {
      normal *= -1;
      p.flip();
    }
    new_entry->set_surface_normal(normal);
    PN_stdfloat min_dist = 0;
    LPoint3 interior_point;
    // Find the deepest vertex and set it as the interior point
    for (int i = 0; i < 8; i++) {
      LPoint3 vertex = box->get_point(i) * wrt_mat;
      PN_stdfloat dist = p.dist_to_plane(vertex);
      if (dist <= min_dist) {
        min_dist = dist;
        interior_point = vertex;
      }
    }
    new_entry->set_interior_point(interior_point);
    new_entry->set_surface_point(
        closest_point_on_triangle(interior_point, intersected_tri));
    return new_entry;
  } else {
    return nullptr;
  }
}

/**
 *
 */
bool CollisionHeightfield::
line_intersects_box(const LPoint3 &box_min, const LPoint3 &box_max,
                    IntersectionParams &params) {
  LPoint3 from = params.from_origin;
  LPoint3 delta = params.from_direction;

  double tmin = -DBL_MAX;
  double tmax = DBL_MAX;
  for (int i = 0; i < 3; ++i) {
    PN_stdfloat d = delta[i];
    if (!IS_NEARLY_ZERO(d)) {
      double tmin2 = (box_min[i] - from[i]) / d;
      double tmax2 = (box_max[i] - from[i]) / d;
      if (tmin2 > tmax2) {
        std::swap(tmin2, tmax2);
      }
      tmin = max(tmin, tmin2);
      tmax = min(tmax, tmax2);
      if (tmin > tmax) {
        return false;
      }
    } else if (from[i] < box_min[i] || from[i] > box_max[i]) {
      // The line is parallel
      return false;
    }
  }
  params.t1 = tmin;
  params.t2 = tmax;
  return true;
}

/**
 *
 */
bool CollisionHeightfield::
line_intersects_triangle(double &t, const LPoint3 &from,
                         const LPoint3 &delta,
                         const Triangle &triangle) {
  // Implementation of Möller-Trumbore algorithm
  PN_stdfloat a,f,u,v;
  LVector3 edge1, edge2, h, s, q;
  edge1 = triangle.p2 - triangle.p1;
  edge2 = triangle.p3 - triangle.p1;
  h = delta.cross(edge2);
  a = dot(edge1, h);
  if (IS_NEARLY_ZERO(a)) {
    // line parallel to triangle
    return false;
  }
  f = 1.0 / a;
  s = from - triangle.p1;
  u = f * dot(s, h);
  if (u < 0.0 || u > 1.0) {
    return false;
  }
  q = s.cross(edge1);
  v = f * dot(delta, q);
  if (v < 0.0 || u + v > 1.0) {
    return false;
  }
  t = f * dot(edge2, q);
  return true;
}

/**
 *
 */
bool CollisionHeightfield::
sphere_intersects_box(const LPoint3 &box_min, const LPoint3 &box_max,
                      IntersectionParams &params) {
  LPoint3 center = params.center;
  double radius = params.radius;
  LPoint3 p = center.fmin(box_max).fmax(box_min);
  return (center - p).length_squared() <= radius * radius;
}

/**
 *
 */
bool CollisionHeightfield::
box_intersects_triangle(const LPoint3 &box_min, const LPoint3 &box_max,
                        const Triangle &triangle) {
  // Using the Seperating Axis Theorem, we have a maximum of 13 SAT tests
  // Refer to Christer Ericson's Collision Detection book for full explanation
  LPoint3 v0 = triangle.p1, v1 = triangle.p2, v2 = triangle.p3;

  PN_stdfloat p0, p1, p2, r;
  LVector3 c = (box_min + box_max) * 0.5f;
  PN_stdfloat e0 = (box_max[0] - box_min[0]) * 0.5f;
  PN_stdfloat e1 = (box_max[1] - box_min[1]) * 0.5f;
  PN_stdfloat e2 = (box_max[2] - box_min[2]) * 0.5f;

  v0 = v0 - c;
  v1 = v1 - c;
  v2 = v2 - c;

  LVector3 f0 = v1 - v0, f1 = v2 - v1, f2 = v0 - v2;

  r = e1 * cabs(f0[2]) + e2 * cabs(f0[1]);
  p0 =  v0[1]*(-v1[2] + v0[2]) + v0[2]*(v1[1] - v0[1]);
  p2 =  v2[1]*(-v1[2] + v0[2]) + v2[2]*(v1[1] - v0[1]);
  if (max(-1 * max(p0, p2), min(p0, p2)) > r) return false;

  r = e1 * cabs(f1[2]) + e2 * cabs(f1[1]);
  p0 =  v0[1]*(-v2[2] + v1[2]) + v0[2]*(v2[1] - v1[1]);
  p2 =  v2[1]*(-v2[2] + v1[2]) + v2[2]*(v2[1] - v1[1]);
  if (max(-1 * max(p0, p2), min(p0, p2)) > r) return false;

  r = e1 * cabs(f2[2]) + e2 * cabs(f2[1]);
  p0 =  v0[1]*(-v0[2] + v2[2]) + v0[2]*(v0[1] - v2[1]);
  p1 =  v1[1]*(-v0[2] + v2[2]) + v1[2]*(v0[1] - v2[1]);
  if (max(-1 * max(p0, p1), min(p0, p1)) > r) return false;

  r = e0 * cabs(f0[2]) + e2 * cabs(f0[0]);
  p0 =  v0[0]*(v1[2] - v0[2]) + v0[2]*(-v1[0] + v0[0]);
  p2 =  v2[0]*(v1[2] - v0[2]) + v2[2]*(-v1[0] + v0[0]);
  if (max(-1 * max(p0, p2), min(p0, p2)) > r) return false;

  r = e0 * cabs(f1[2]) + e2 * cabs(f1[0]);
  p0 =  v0[0]*(v2[2] - v1[2]) + v0[2]*(-v2[0] + v1[0]);
  p2 =  v2[0]*(v2[2] - v1[2]) + v2[2]*(-v2[0] + v1[0]);
  if (max(-1 * max(p0, p2), min(p0, p2)) > r) return false;

  r = e0 * cabs(f2[2]) + e2 * cabs(f2[0]);
  p0 =  v0[0]*(v0[2] - v2[2]) + v0[2]*(-v0[0] + v2[0]);
  p1 =  v1[0]*(v0[2] - v2[2]) + v1[2]*(-v0[0] + v2[0]);
  if (max(-1 * max(p0, p1), min(p0, p1)) > r) return false;

  r = e0 * cabs(f0[1]) + e1 * cabs(f0[0]);
  p0 =  v0[0]*(-v1[1] + v0[1]) + v0[1]*(v1[0] - v0[0]);
  p2 =  v2[0]*(-v1[1] + v0[1]) + v2[1]*(v1[0] - v0[0]);
  if (max(-1 * max(p0, p2), min(p0, p2)) > r) return false;

  r = e0 * cabs(f1[1]) + e1 * cabs(f1[0]);
  p0 =  v0[0]*(-v2[1] + v1[1]) + v0[1]*(v2[0] - v1[0]);
  p2 =  v2[0]*(-v2[1] + v1[1]) + v2[1]*(v2[0] - v1[0]);
  if (max(-1 * max(p0, p2), min(p0, p2)) > r) return false;

  r = e0 * cabs(f2[1]) + e1 * cabs(f2[0]);
  p0 =  v0[0]*(-v0[1] + v2[1]) + v0[1]*(v0[0] - v2[0]);
  p1 =  v1[0]*(-v0[1] + v2[1]) + v1[1]*(v0[0] - v2[0]);
  if (max(-1 * max(p0, p1), min(p0, p1)) > r) return false;

  if (max(max(v0[0], v1[0]), v2[0]) < -e0 ||
      min(min(v0[0], v1[0]), v2[0]) > e0) return false;
  if (max(max(v0[1], v1[1]), v2[1]) < -e1 ||
      min(min(v0[1], v1[1]), v2[1]) > e1) return false;
  if (max(max(v0[2], v1[2]), v2[2]) < -e2 ||
      min(min(v0[2], v1[2]), v2[2]) > e2) return false;

  LVector3 n = f0.cross(f1);
  PN_stdfloat d = dot(n, triangle.p1);

  LPoint3 e = box_max - c;

  PN_stdfloat r2 = e[0] * cabs(n[0]) + e[1] * cabs(n[1]) + e[2] * cabs(n[2]);
  PN_stdfloat s = dot(n, c) - d;
  return cabs(s) <= r2;
}

/**
 *
 */
bool CollisionHeightfield::
box_intersects_box(const LPoint3 &box_min, const LPoint3 &box_max,
                   IntersectionParams &params) {

  return (box_min[0] <= params.box_max[0] && box_max[0] >= params.box_min[0]) &&
         (box_min[1] <= params.box_max[1] && box_max[1] >= params.box_min[1]) &&
         (box_min[2] <= params.box_max[2] && box_max[2] >= params.box_min[2]);
}

/**
 * Returns the closest point on a triangle to another given point.
 */
LPoint3 CollisionHeightfield::
closest_point_on_triangle(const LPoint3 &p, const Triangle &triangle) {
  LVector3 ab = triangle.p2 - triangle.p1;
  LVector3 ac = triangle.p3 - triangle.p1;
  LVector3 ap = p - triangle.p1;
  PN_stdfloat d1 = dot(ab, ap);
  PN_stdfloat d2 = dot(ac, ap);
  if (d1 <= 0.0f && d2 <= 0.0f) return triangle.p1;

  LVector3 bp = p - triangle.p2;
  PN_stdfloat d3 = dot(ab, bp);
  PN_stdfloat d4 = dot(ac, bp);
  if (d3 >= 0.0f && d4 <= d3) return triangle.p2;

  PN_stdfloat vc = d1 * d4 - d3 * d2;
  if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
    PN_stdfloat v = d1 / (d1 - d3);
    return triangle.p1 + v * ab;
  }

  LVector3 cp = p - triangle.p3;
  PN_stdfloat d5 = dot(ab, cp);
  PN_stdfloat d6 = dot(ac, cp);
  if (d6 >= 0.0f && d5 <= d6) return triangle.p3;

  PN_stdfloat vb = d5 * d2 - d1 * d6;
  if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
    PN_stdfloat w = d2 / (d2 - d6);
    return triangle.p1 + w * ac;
  }

  PN_stdfloat va = d3 * d6 - d5 * d4;
  if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
    PN_stdfloat w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
    return triangle.p2 + w * (triangle.p3 - triangle.p2);
  }

  PN_stdfloat denom = 1.0f / (va + vb + vc);
  PN_stdfloat v = vb * denom;
  PN_stdfloat w = vc * denom;
  return triangle.p1 + ab * v + ac * w;
}

/**
 * Given a pointer to a function that tests for intersection between a solid
 * and a box, return the quad tree nodes that are intersected by that solid.
 */
vector<CollisionHeightfield::QuadTreeIntersection> CollisionHeightfield::
find_intersections(BoxIntersection intersects_box, IntersectionParams params) const {
  queue<QuadTreeNode> q;
  QuadTreeNode node = _nodes[0];
  q.push(node);

  LPoint3 box_min, box_max;
  vector<QuadTreeIntersection> intersections;

  while (!q.empty()) {
    node = q.front();
    q.pop();
    box_min = {node.area.min[0], node.area.min[1], node.height_min};
    box_max = {node.area.max[0], node.area.max[1], node.height_max};
    if (!intersects_box(box_min, box_max, params)) {
      continue;
    }

    if (node.index >= _leaf_first_index) {
      QuadTreeIntersection intersection = {node.index, params.t1, params.t2};
      intersections.push_back(intersection);
    } else {
      int child_first_index = 4 * node.index + 1;
      q.push(_nodes[child_first_index]);
      q.push(_nodes[child_first_index + 1]);
      q.push(_nodes[child_first_index + 2]);
      q.push(_nodes[child_first_index + 3]);
    }
  }

  return intersections;
}

/**
 * Return the triangles that are defined at the
 * given 2D coordinate.
 */
vector<CollisionHeightfield::Triangle> CollisionHeightfield::
get_triangles(int x, int y) const {
  int rows = _heightfield.get_x_size();
  int cols = _heightfield.get_y_size();
  vector<Triangle> triangles;
  if (x < 0 || y < 0 || x >= rows || y >= cols)
    return triangles;

  Triangle t;
  int heightfield_y = cols - 1 - y;
  bool odd = (x + heightfield_y) & 1;
  t.p1 = LPoint3(x, y, get_height(x, heightfield_y));
  for (int dx = -1; dx <= 1; dx += 2) {
    for (int dy = -1; dy <= 1; dy += 2) {
      int x2 = x + dx;
      int y2 = y - dy;
      int heightfield_y2 = heightfield_y + dy;
      if (x2 < 0 || heightfield_y2 < 0 || x2 >= rows || heightfield_y2 >= cols) continue;
      if (odd) {
        t.p2 = LPoint3(x2, y, get_height(x2, heightfield_y));
        t.p3 = LPoint3(x, y2, get_height(x, heightfield_y2));
      } else {
        t.p2 = LPoint3(x2, y2, get_height(x2, heightfield_y2));
        t.p3 = LPoint3(x2, y, get_height(x2, heightfield_y));
        triangles.push_back(t);
        t.p3 = LPoint3(x, y2, get_height(x, heightfield_y2));
        triangles.push_back(t);
      }
    }
  }

  return triangles;
}

/**
 * Generic CollisionSolid member functions
 */
void CollisionHeightfield::
fill_viz_geom() {
  PT(GeomVertexData) vdata = new GeomVertexData
    ("collision", GeomVertexFormat::get_v3(),
     Geom::UH_static);

  GeomVertexWriter vertex(vdata, InternalName::get_vertex());

  PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);

  int cols = _heightfield.get_x_size();
  int rows = _heightfield.get_y_size();

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      // a point (x, y) in the heightfield image
      // maps to (x, y2, height_at(x, y)) in 3D space
      int y2 = rows - y - 1;
      vertex.add_data3(x, y2, get_height(x, y));
    }
  }

  for (int i = 0; i < rows - 1; i++) {
    for (int j = 0; j < cols - 1; j++) {
      int pos = i * cols + j;
      if (pos & 1) { // odd
        tris->add_vertices(pos, pos + cols, pos + 1);
        tris->add_vertices(pos + 1, pos + cols, pos + cols + 1);
      }
      else { // even
        tris->add_vertices(pos, pos + cols, pos + cols + 1);
        tris->add_vertices(pos, pos + cols + 1, pos + 1);
      }
    }
  }

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(tris);

  _viz_geom->add_geom(geom, get_solid_viz_state());
  _bounds_viz_geom->add_geom(geom, get_solid_bounds_viz_state());
}

CollisionSolid *CollisionHeightfield::
make_copy() {
  return new CollisionHeightfield(*this);
}

LPoint3 CollisionHeightfield::
get_collision_origin() const {
  return LPoint3(0, 0, 0);
}

PT(BoundingVolume) CollisionHeightfield::
compute_internal_bounds() const {
  QuadTreeNode node = _nodes[0];
  LPoint3 box_min = {node.area.min[0], node.area.min[1],
                     node.height_min};
  LPoint3 box_max = {node.area.max[0], node.area.max[1],
                     node.height_max};
  return new BoundingBox(box_min, box_max);
}

PStatCollector &CollisionHeightfield::
get_volume_pcollector() {
  return _volume_pcollector;
}

PStatCollector &CollisionHeightfield::
get_test_pcollector() {
  return _test_pcollector;
}

TypedWritable *CollisionHeightfield::
make_from_bam(const FactoryParams &params) {
  CollisionHeightfield *me = new CollisionHeightfield;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

void CollisionHeightfield::
fillin(DatagramIterator &scan, BamReader *manager) {
}

void CollisionHeightfield::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}
