// Filename: eggData.I
// Created by:  drose (11Feb99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: EggData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggData::
EggData() {
  _coordsys = CS_default;
}


////////////////////////////////////////////////////////////////////
//     Function: EggData::Copy constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggData::
EggData(const EggData &copy) : 
  EggGroupNode(copy), 
  _coordsys(copy._coordsys), 
  _egg_filename(copy._egg_filename) {
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::Copy assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggData &EggData::
operator = (const EggData &copy) {
  EggGroupNode::operator = (copy);
  _coordsys = copy._coordsys;
  _egg_filename = copy._egg_filename;
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: EggData::get_coordinate_system
//       Access: Public
//  Description: Returns the coordinate system in which the egg file
//               is defined.
////////////////////////////////////////////////////////////////////
INLINE CoordinateSystem EggData::
get_coordinate_system() const {
  return _coordsys;
}


////////////////////////////////////////////////////////////////////
//     Function: EggData::set_egg_filename
//       Access: Public
//  Description: Sets the directory in which the egg file is
//               considered to reside.  This is also implicitly set by
//               read().
////////////////////////////////////////////////////////////////////
INLINE void EggData::
set_egg_filename(const Filename &directory) {
  _egg_filename = directory;
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::get_egg_filename
//       Access: Public
//  Description: Returns the directory in which the egg file is
//               considered to reside.
////////////////////////////////////////////////////////////////////
INLINE const Filename &EggData::
get_egg_filename() const {
  return _egg_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::recompute_vertex_normals
//       Access: Public
//  Description: Recomputes all the vertex normals for polygon
//               geometry at this group node and below so that they
//               accurately reflect the vertex positions.  A shared
//               edge between two polygons (even in different groups)
//               is considered smooth if the angle between the two
//               edges is less than threshold degrees.
//
//               This function also removes degenerate polygons that
//               do not have enough vertices to define a normal.  It
//               does not affect normals for other kinds of primitives
//               like Nurbs or Points.
//
//               This function does not remove or adjust vertices in
//               the vertex pool; it only adds new vertices with the
//               correct normals.  Thus, it is a good idea to call
//               remove_unused_vertices() after calling this.
////////////////////////////////////////////////////////////////////
INLINE void EggData::
recompute_vertex_normals(double threshold) {
  EggGroupNode::recompute_vertex_normals(threshold, _coordsys);
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::recompute_polygon_normals
//       Access: Public
//  Description: Recomputes all the polygon normals for polygon
//               geometry at this group node and below so that they
//               accurately reflect the vertex positions.  Normals are
//               removed from the vertices and defined only on
//               polygons, giving the geometry a faceted appearance.
//
//               This function also removes degenerate polygons that
//               do not have enough vertices to define a normal.  It
//               does not affect normals for other kinds of primitives
//               like Nurbs or Points.
//
//               This function does not remove or adjust vertices in
//               the vertex pool; it only adds new vertices with the
//               normals removed.  Thus, it is a good idea to call
//               remove_unused_vertices() after calling this.
////////////////////////////////////////////////////////////////////
INLINE void EggData::
recompute_polygon_normals() {
  EggGroupNode::recompute_polygon_normals(_coordsys);
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::strip_normals
//       Access: Public
//  Description: Removes all normals from primitives, and the vertices
//               they reference, at this node and below.
//
//               This function does not remove or adjust vertices in
//               the vertex pool; it only adds new vertices with the
//               normal removed.  Thus, it is a good idea to call
//               remove_unused_vertices() after calling this.
////////////////////////////////////////////////////////////////////
INLINE void EggData::
strip_normals() {
  EggGroupNode::strip_normals();
}
