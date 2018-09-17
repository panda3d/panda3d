/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggPolygon.I
 * @author drose
 * @date 1999-02-10
 */

/**
 *
 */
INLINE EggPolygon::
EggPolygon(const std::string &name) : EggPrimitive(name) {
}

/**
 *
 */
INLINE EggPolygon::
EggPolygon(const EggPolygon &copy) : EggPrimitive(copy) {
}

/**
 *
 */
INLINE EggPolygon &EggPolygon::
operator = (const EggPolygon &copy) {
  EggPrimitive::operator = (copy);
  return *this;
}

/**
 * Recalculates the normal according to the order of the vertices, and sets
 * it.  Returns true if the normal is computed correctly, or false if the
 * polygon is degenerate and does not have a normal.
 */
INLINE bool EggPolygon::
recompute_polygon_normal(CoordinateSystem cs) {
  LNormald normal;
  if (calculate_normal(normal, cs)) {
    set_normal(normal);
    return true;
  }
  clear_normal();
  return false;
}

/**
 * Subdivides the polygon into triangles and adds each one to the indicated
 * container.  If the polygon is already a triangle, adds an exact copy of the
 * polygon to the container.  Does not remove the polygon from its existing
 * parent or modify it in any way.
 *
 * Returns true if the triangulation is successful, or false if there was some
 * error (in which case the container may contain some partial triangulation).
 *
 * If convex_also is true, both concave and convex polygons will be subdivided
 * into triangles; otherwise, only concave polygons will be subdivided, and
 * convex polygons will be copied unchanged into the container.
 */
INLINE bool EggPolygon::
triangulate_into(EggGroupNode *container, bool convex_also) const {
  PT(EggPolygon) copy = new EggPolygon(*this);
  return copy->triangulate_poly(container, convex_also);
}
