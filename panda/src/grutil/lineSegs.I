// Filename: lineSegs.I
// Created by:  drose (16Mar02)
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

////////////////////////////////////////////////////////////////////
//     Function: LineSegs::Point::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LineSegs::Point::
Point() {
}

////////////////////////////////////////////////////////////////////
//     Function: LineSegs::Point::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LineSegs::Point::
Point(const LVecBase3 &point, const LColor &color) :
  _point(point[0], point[1], point[2]),
  _color(color)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LineSegs::Point::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LineSegs::Point::
Point(const LineSegs::Point &copy) :
  _point(copy._point),
  _color(copy._color)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LineSegs::Point::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LineSegs::Point::
operator = (const LineSegs::Point &copy) {
  _point = copy._point;
  _color = copy._color;
}


////////////////////////////////////////////////////////////////////
//     Function: LineSegs::set_color
//       Access: Public
//  Description: Establishes the color that will be assigned to all
//               vertices created by future calls to move_to() and
//               draw_to().
////////////////////////////////////////////////////////////////////
INLINE void LineSegs::
set_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a) {
  _color.set(r, g, b, a);
}

////////////////////////////////////////////////////////////////////
//     Function: LineSegs::set_color
//       Access: Public
//  Description: Establishes the color that will be assigned to all
//               vertices created by future calls to move_to() and
//               draw_to().
////////////////////////////////////////////////////////////////////
INLINE void LineSegs::
set_color(const LColor &color) {
  _color = color;
}

////////////////////////////////////////////////////////////////////
//     Function: LineSegs::set_thickness
//       Access: Public
//  Description: Establishes the line thickness or point size in
//               pixels that will be assigned to all lines and points
//               created by future calls to create().
////////////////////////////////////////////////////////////////////
INLINE void LineSegs::
set_thickness(PN_stdfloat thick) {
  _thick = thick;
}

////////////////////////////////////////////////////////////////////
//     Function: LineSegs::move_to
//       Access: Public
//  Description: Moves the pen to the given point without drawing a
//               line.  When followed by draw_to(), this marks the
//               first point of a line segment; when followed by
//               move_to() or create(), this creates a single point.
////////////////////////////////////////////////////////////////////
INLINE void LineSegs::
move_to(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  move_to(LVertex(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: LineSegs::draw_to
//       Access: Public
//  Description: Draws a line segment from the pen's last position
//               (the last call to move_to or draw_to) to the
//               indicated point.  move_to() and draw_to() only update
//               tables; the actual drawing is performed when create()
//               is called.
////////////////////////////////////////////////////////////////////
INLINE void LineSegs::
draw_to(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  draw_to(LVertex(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: LineSegs::create
//       Access: Public
//  Description: Creates a new GeomNode that will render the series of
//               line segments and points described via calls to
//               move_to() and draw_to().  The lines and points are
//               created with the color and thickness established by
//               calls to set_color() and set_thick().
//
//               If dynamic is true, the line segments will be created
//               with the dynamic Geom setting, optimizing them for
//               runtime vertex animation.
////////////////////////////////////////////////////////////////////
INLINE GeomNode *LineSegs::
create(bool dynamic) {
  GeomNode *gnode = new GeomNode(get_name());
  return create(gnode, dynamic);
}

////////////////////////////////////////////////////////////////////
//     Function: LineSegs::get_num_vertices
//       Access: Public
//  Description: Returns the total number of line segment and point
//               vertices generated by the last call to create().  The
//               positions of these vertices may be read and adjusted
//               through get_vertex() and set_vertex().
////////////////////////////////////////////////////////////////////
INLINE int LineSegs::
get_num_vertices() const {
  if (_created_data == (GeomVertexData *)NULL) {
    return 0;
  }
  return _created_data->get_num_rows();
}

////////////////////////////////////////////////////////////////////
//     Function: LineSegs::set_vertex
//       Access: Public
//  Description: Moves the nth point or vertex of the line segment
//               sequence generated by the last call to create().  The
//               first move_to() generates vertex 0; subsequent
//               move_to() and draw_to() calls generate consecutively
//               higher vertex numbers.
////////////////////////////////////////////////////////////////////
INLINE void LineSegs::
set_vertex(int n, PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  set_vertex(n, LVertex(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: LineSegs::set_vertex_color
//       Access: Public
//  Description: Changes the vertex color of the nth point or vertex.
//               See set_vertex().
////////////////////////////////////////////////////////////////////
INLINE void LineSegs::
set_vertex_color(int n, PN_stdfloat r, PN_stdfloat g, PN_stdfloat b, PN_stdfloat a) {
  set_vertex_color(n, LColor(r, g, b, a));
}
