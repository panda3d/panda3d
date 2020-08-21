/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggPolygon.cxx
 * @author drose
 * @date 1999-01-16
 */

#include "eggPolygon.h"
#include "eggGroupNode.h"
#include "plane.h"

#include "indent.h"

#include <algorithm>

TypeHandle EggPolygon::_type_handle;

/**
 * Makes a copy of this object.
 */
EggPolygon *EggPolygon::
make_copy() const {
  return new EggPolygon(*this);
}

/**
 * Cleans up modeling errors in whatever context this makes sense.  For
 * instance, for a polygon, this calls remove_doubled_verts(true).  For a
 * point, it calls remove_nonunique_verts().  Returns true if the primitive is
 * valid, or false if it is degenerate.
 */
bool EggPolygon::
cleanup() {
  remove_doubled_verts(true);

  // Use calculate_normal() to determine if the polygon is degenerate.
  LNormald normal;
  return calculate_normal(normal);
}

/**
 * Calculates the true polygon normal--the vector pointing out of the front of
 * the polygon--based on the vertices.  This does not return or change the
 * polygon's normal as set via set_normal().
 *
 * The return value is true if the normal is computed correctly, or false if
 * the polygon is degenerate and does not have at least three noncollinear
 * vertices.
 */
bool EggPolygon::
calculate_normal(LNormald &result, CoordinateSystem cs) const {
  result = LNormald::zero();

  // Project the polygon into each of the three major planes and calculate the
  // area of each 2-d projection.  This becomes the polygon normal.  This
  // works because the ratio between these different areas corresponds to the
  // angle at which the polygon is tilted toward each plane.
  size_t num_verts = size();
  for (size_t i = 0; i < num_verts; i++) {
    LVertexd p0 = get_vertex(i)->get_pos3();
    LVertexd p1 = get_vertex((i + 1) % num_verts)->get_pos3();
    result[0] += p0[1] * p1[2] - p0[2] * p1[1];
    result[1] += p0[2] * p1[0] - p0[0] * p1[2];
    result[2] += p0[0] * p1[1] - p0[1] * p1[0];
  }

  if (!result.normalize()) {
    // The polygon is degenerate: it has zero area in each plane.
    return false;
  }

  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }
  if (cs == CS_zup_left || cs == CS_yup_left) {
    // In a left-handed coordinate system, we must flip the result.
    result = -result;
  }
  return true;
}

/**
 * Returns true if all of the polygon's vertices lie within the same plane,
 * false otherwise.
 */
bool EggPolygon::
is_planar() const {
  if (size() <= 3) {
    // If we don't have more than three vertices, we can't be non-planar.
    return true;
  }

  LNormald normal;
  if (!calculate_normal(normal)) {
    // A degenerate polygon--all of the vertices are within one line, or all
    // in the same point--is technically planar.  Not sure if this is a useful
    // return value or not.
    return true;
  }

  // There should be at least one vertex (actually, at least three) since we
  // have already shown that the polygon is nondegenerate.
  nassertr(!empty(), false);

  // Create a plane perpendicular to the polygon's normal, containing the
  // first vertex.
  const_iterator vi = begin();
  LVecBase3d first_point = (*vi)->get_pos3();
  LPlaned plane(normal, first_point);

  // And check that all of the remaining vertices are sufficiently close to
  // the plane.
  ++vi;
  while (vi != end()) {
    LVecBase3d this_point = (*vi)->get_pos3();
    if (!this_point.almost_equal(first_point)) {
      double dist = plane.dist_to_plane(this_point);
      double tol = dist / length(this_point - first_point);
      if (!IS_THRESHOLD_ZERO(tol, 0.0001)) {
        // Nope, too far away--the polygon is nonplanar.
        return false;
      }
    }
    ++vi;
  }

  // All vertices are close enough to pass.
  return true;
}

/**
 * Subdivides the polygon into triangles and adds those triangles to the
 * parent group node in place of the original polygon.  Returns a pointer to
 * the original polygon, which is likely about to be destructed.
 *
 * If convex_also is true, both concave and convex polygons will be subdivided
 * into triangles; otherwise, only concave polygons will be subdivided, and
 * convex polygons will be copied unchanged into the container.
 */
PT(EggPolygon) EggPolygon::
triangulate_in_place(bool convex_also) {
  EggGroupNode *parent = get_parent();
  nassertr(parent != nullptr, this);

  PT(EggPolygon) save_me = this;
  parent->remove_child(this);
  triangulate_poly(parent, convex_also);

  return save_me;
}

/**
 * Writes the polygon to the indicated output stream in Egg format.
 */
void EggPolygon::
write(std::ostream &out, int indent_level) const {
  write_header(out, indent_level, "<Polygon>");
  write_body(out, indent_level+2);
  indent(out, indent_level) << "}\n";
}

/**
 * Decomposes a concave polygon into triangles.  Returns true if successful,
 * false if the polygon is self-intersecting.
 */
bool EggPolygon::
decomp_concave(EggGroupNode *container, int asum, int x, int y) const {
#define VX(p, c)    p->coord[c]

  struct DecompVtx {
    int index;
    LPoint3d coord;
    struct DecompVtx *next;
  };

  DecompVtx *p0, *p1, *p2, *t0, *vert;
  DecompVtx *m[3];
  double xmin, xmax, ymin, ymax;
  int i, init, csum, chek;
  double a[3], b[3], c[3], s[3];

  int num_verts = size();
  nassertr(num_verts >= 3, false);

  /* Make linked list of verts */
  vert = (DecompVtx *) alloca(sizeof(DecompVtx));
  vert->index = 0;
  vert->coord = get_vertex(0)->get_pos3();
  p1 = vert;

  for (i = 1; i < num_verts; i++) {
    p0 = (DecompVtx *) alloca(sizeof(DecompVtx));
    p0->index = i;
    p0->coord = get_vertex(i)->get_pos3();
    // There shouldn't be two consecutive identical vertices.  If there are,
    // skip one.
    if (!(p0->coord == p1->coord)) {
      p1->next = p0;
      p1 = p0;
    }
  }
  p1->next = vert;

  p0 = vert;
  p1 = p0->next;
  p2 = p1->next;
  m[0] = p0;
  m[1] = p1;
  m[2] = p2;
  chek = 0;

  while (p0 != p2->next) {
    /* Polygon is self-intersecting so punt */
    if (chek &&
        m[0] == p0 &&
        m[1] == p1 &&
        m[2] == p2) {

      return false;
    }

    chek = 1;

    a[0] = VX(p1, y) - VX(p2, y);
    b[0] = VX(p2, x) - VX(p1, x);
    a[2] = VX(p0, y) - VX(p1, y);
    b[2] = VX(p1, x) - VX(p0, x);

    csum = ((b[0] * a[2] - b[2] * a[0] >= 0.0) ? 1 : 0);

    if (csum ^ asum) {
      /* current angle is concave */
      p0 = p1;
      p1 = p2;
      p2 = p2->next;

    } else {
      /* current angle is convex */
      xmin = (VX(p0, x) < VX(p1, x)) ? VX(p0, x) : VX(p1, x);
      if (xmin > VX(p2, x))
        xmin = VX(p2, x);

      xmax = (VX(p0, x) > VX(p1, x)) ? VX(p0, x) : VX(p1, x);
      if (xmax < VX(p2, x))
        xmax = VX(p2, x);

      ymin = (VX(p0, y) < VX(p1, y)) ? VX(p0, y) : VX(p1, y);
      if (ymin > VX(p2, y))
        ymin = VX(p2, y);

      ymax = (VX(p0, y) > VX(p1, y)) ? VX(p0, y) : VX(p1, y);
      if (ymax < VX(p2, y))
        ymax = VX(p2, y);

      for (init = 1, t0 = p2->next; t0 != p0; t0 = t0->next) {
        if (VX(t0, x) >= xmin && VX(t0, x) <= xmax &&
            VX(t0, y) >= ymin && VX(t0, y) <= ymax) {
          if (init) {
            a[1] = VX(p2, y) - VX(p0, y);
            b[1] = VX(p0, x) - VX(p2, x);
            init = 0;
            c[0] = VX(p1, x) * VX(p2, y) - VX(p2, x) * VX(p1, y);
            c[1] = VX(p2, x) * VX(p0, y) - VX(p0, x) * VX(p2, y);
            c[2] = VX(p0, x) * VX(p1, y) - VX(p1, x) * VX(p0, y);
          }

          s[0] = a[0] * VX(t0, x) + b[0] * VX(t0, y) + c[0];
          s[1] = a[1] * VX(t0, x) + b[1] * VX(t0, y) + c[1];
          s[2] = a[2] * VX(t0, x) + b[2] * VX(t0, y) + c[2];

          if (asum) {
            if (s[0] >= 0.0 && s[1] >= 0.0 && s[2] >= 0.0)
              break;
          } else {
            if (s[0] <= 0.0 && s[1] <= 0.0 && s[2] <= 0.0)
              break;
          }
        }
      }

      if (t0 != p0) {
        p0 = p1;
        p1 = p2;
        p2 = p2->next;
      } else {

        // Here's a triangle to output.
        PT(EggPolygon) triangle = new EggPolygon(*this);
        triangle->clear();
        triangle->add_vertex(get_vertex(p0->index));
        triangle->add_vertex(get_vertex(p1->index));
        triangle->add_vertex(get_vertex(p2->index));

        if (triangle->cleanup()) {
          container->add_child(triangle.p());
        }

        p0->next = p1->next;
        p1 = p2;
        p2 = p2->next;

        m[0] = p0;
        m[1] = p1;
        m[2] = p2;
        chek = 0;
      }
    }
  }

  // One more triangle to output.
  PT(EggPolygon) triangle = new EggPolygon(*this);
  triangle->clear();
  triangle->add_vertex(get_vertex(p0->index));
  triangle->add_vertex(get_vertex(p1->index));
  triangle->add_vertex(get_vertex(p2->index));

  if (triangle->cleanup()) {
    container->add_child(triangle.p());
  }

  return true;
}


/**
 * Breaks a (possibly concave) higher-order polygon into a series of
 * constituent triangles.  Fills the container up with EggPolygons that
 * represent the triangles.  Returns true if successful, false on failure.
 *
 * If convex_also is true, both concave and convex polygons will be subdivided
 * into triangles; otherwise, only concave polygons will be subdivided, and
 * convex polygons will be copied unchanged into the container.
 *
 * It is assumed that the EggPolygon is not already a child of any other group
 * when this function is called.
 */
bool EggPolygon::
triangulate_poly(EggGroupNode *container, bool convex_also) {
  LPoint3d p0, p1, as;
  double dx1, dy1, dx2, dy2, max;
  int i, flag, asum, csum, index, x, y, v0, v1, v, even;

  if (!cleanup()) {
    return false;
  }

  // First see if the polygon is already a triangle
  int num_verts = size();
  if (num_verts == 3) {
    container->add_child(this);

    return true;

  } else if (num_verts < 3) {
    // Or if it's a degenerate polygon.
    return false;
  }

  // calculate signed areas
  as[0] = 0.0;
  as[1] = 0.0;
  as[2] = 0.0;

  for (i = 0; i < num_verts; i++) {
    p0 = get_vertex(i)->get_pos3();
    p1 = get_vertex((i + 1) % num_verts)->get_pos3();
    as[0] += p0[0] * p1[1] - p0[1] * p1[0];
    as[1] += p0[0] * p1[2] - p0[2] * p1[0];
    as[2] += p0[1] * p1[2] - p0[2] * p1[1];
  }

  /* select largest signed area */
  max = 0.0;
  index = 0;
  flag = 0;
  for (i = 0; i < 3; i++) {
    if (as[i] >= 0.0) {
      if (as[i] > max) {
        max = as[i];
        index = i;
        flag = 1;
      }
    } else {
      as[i] = -as[i];
      if (as[i] > max) {
        max = as[i];
        index = i;
        flag = 0;
      }
    }
  }

  /* pointer offsets */
  switch (index) {
  case 0:
    x = 0;
    y = 1;
    break;

  case 1:
    x = 0;
    y = 2;
    break;

  default: // case 2
    x = 1;
    y = 2;
    break;
  }

  /* concave check */
  p0 = get_vertex(0)->get_pos3();
  p1 = get_vertex(1)->get_pos3();
  dx1 = p1[x] - p0[x];
  dy1 = p1[y] - p0[y];
  p0 = p1;
  p1 = get_vertex(2)->get_pos3();

  dx2 = p1[x] - p0[x];
  dy2 = p1[y] - p0[y];
  asum = ((dx1 * dy2 - dx2 * dy1 >= 0.0) ? 1 : 0);

  for (i = 0; i < num_verts - 1; i++) {
    p0 = p1;
    p1 = get_vertex((i+3) % num_verts)->get_pos3();

    dx1 = dx2;
    dy1 = dy2;
    dx2 = p1[x] - p0[x];
    dy2 = p1[y] - p0[y];
    csum = ((dx1 * dy2 - dx2 * dy1 >= 0.0) ? 1 : 0);

    if (csum ^ asum) {
      // It's a concave polygon.  This is a little harder.
      return decomp_concave(container, flag, x, y);
    }
  }

  // It's a convex polygon.
  if (!convex_also) {
    // Make sure that it's also coplanar.  If it's not, we should triangulate
    // it anyway.
    if (is_planar()) {
      container->add_child(this);
      return true;
    }
  }

  v0 = 0;
  v1 = 1;
  v = num_verts - 1;

  even = 1;

  /*
   * Convert to triangles only. Do not fan out from a single vertex
   * but zigzag into triangle strip.
   */
  for (i = 0; i < num_verts - 2; i++) {
    if (even) {
      PT(EggPolygon) triangle = new EggPolygon(*this);
      triangle->clear();
      triangle->add_vertex(get_vertex(v0));
      triangle->add_vertex(get_vertex(v1));
      triangle->add_vertex(get_vertex(v));

      if (triangle->cleanup()) {
        container->add_child(triangle.p());
      }

      v0 = v1;
      v1 = v;
      v = v0 + 1;
    } else {
      PT(EggPolygon) triangle = new EggPolygon(*this);
      triangle->clear();
      triangle->add_vertex(get_vertex(v1));
      triangle->add_vertex(get_vertex(v0));
      triangle->add_vertex(get_vertex(v));

      if (triangle->cleanup()) {
        container->add_child(triangle.p());
      }

      v0 = v1;
      v1 = v;
      v = v0 - 1;
    }

    even = !even;
  }

  return true;
}
