// Filename: eggPolygon.I
// Created by:  drose (10Feb99)
// 
////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////
//     Function: EggPolygon::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggPolygon::
EggPolygon(const string &name) : EggPrimitive(name) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggPolygon::Copy constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggPolygon::
EggPolygon(const EggPolygon &copy) : EggPrimitive(copy) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggPolygon::Copy assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggPolygon &EggPolygon::
operator = (const EggPolygon &copy) {
  EggPrimitive::operator = (copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPolygon::recompute_polygon_normal
//       Access: Public
//  Description: Recalculates the normal according to the order of the
//               vertices, and sets it.  Returns true if the normal is
//               computed correctly, or false if the polygon is
//               degenerate and does not have a normal.
////////////////////////////////////////////////////////////////////
INLINE bool EggPolygon::
recompute_polygon_normal() {
  Normald normal;
  if (calculate_normal(normal)) {
    set_normal(normal);
    return true;
  }
  clear_normal();
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPolygon::triangulate_into
//       Access: Public
//  Description: Subdivides the polygon into triangles and adds each
//               one to the indicated container.  If the polygon is
//               already a triangle, adds an exact copy of the polygon
//               to the container.  Does not remove the polygon from
//               its existing parent or modify it in any way.
//
//               Returns true if the triangulation is successful, or
//               false if there was some error (in which case the
//               container may contain some partial triangulation).
////////////////////////////////////////////////////////////////////
INLINE bool EggPolygon::
triangulate_into(EggGroupNode *container) const {
  return triangulate_poly(container);
}
