// Filename: eggData.I
// Created by:  drose (11Feb99)
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


////////////////////////////////////////////////////////////////////
//     Function: EggData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggData::
EggData() {
  _auto_resolve_externals = false;
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
  _auto_resolve_externals(copy._auto_resolve_externals),
  _coordsys(copy._coordsys),
  _egg_filename(copy._egg_filename) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggData &EggData::
operator = (const EggData &copy) {
  EggGroupNode::operator = (copy);
  _auto_resolve_externals = copy._auto_resolve_externals;
  _coordsys = copy._coordsys;
  _egg_filename = copy._egg_filename;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::set_auto_resolve_externals
//       Access: Public
//  Description: Indicates whether the EggData object will
//               automatically resolve any external references when
//               read() is called.  The default is false.
////////////////////////////////////////////////////////////////////
INLINE void EggData::
set_auto_resolve_externals(bool resolve) {
  _auto_resolve_externals = resolve;
}

////////////////////////////////////////////////////////////////////
//     Function: EggData::get_auto_resolve_externals
//       Access: Public
//  Description: Indicates whether the EggData object will
//               automatically resolve any external references when
//               read() is called.  The default is false.
////////////////////////////////////////////////////////////////////
INLINE bool EggData::
get_auto_resolve_externals() const {
  return _auto_resolve_externals;
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
//  Description: Sets the filename--especially the directory part--in
//               which the egg file is considered to reside.  This is
//               also implicitly set by read().
////////////////////////////////////////////////////////////////////
INLINE void EggData::
set_egg_filename(const Filename &egg_filename) {
  _egg_filename = egg_filename;
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
