// Filename: lineSegs.I
// Created by:  drose (16Mar02)
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
Point(const LVecBase3f &point, const Colorf &color) :
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
set_color(float r, float g, float b, float a) {
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
set_color(const Colorf &color) {
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
set_thickness(float thick) {
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
move_to(float x, float y, float z) {
  move_to(Vertexf(x, y, z));
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
draw_to(float x, float y, float z) {
  draw_to(Vertexf(x, y, z));
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
//               If frame_accurate is true, the line segments will be
//               created as a frame-accurate index, so that later
//               calls to set_vertex or set_vertex_color will be
//               visually correct.
////////////////////////////////////////////////////////////////////
INLINE GeomNode *LineSegs::
create(bool frame_accurate) {
  GeomNode *gnode = new GeomNode(get_name());
  return create(gnode, frame_accurate);
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
  return _created_verts.size();
}

////////////////////////////////////////////////////////////////////
//     Function: LineSegs::get_vertex
//       Access: Public
//  Description: Returns the nth point or vertex of the line segment
//               sequence generated by the last call to create().  The
//               first move_to() generates vertex 0; subsequent
//               move_to() and draw_to() calls generate consecutively
//               higher vertex numbers.
////////////////////////////////////////////////////////////////////
INLINE Vertexf LineSegs::
get_vertex(int vertex) const {
  nassertr(vertex >= 0 && vertex < (int)_created_verts.size(),
           Vertexf(0.0f, 0.0f, 0.0f));
  return _created_verts[vertex];
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
set_vertex(int vertex, const Vertexf &vert) {
  nassertv(vertex >= 0 && vertex < (int)_created_verts.size());
  _created_verts[vertex] = vert;
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
set_vertex(int vertex, float x, float y, float z) {
  set_vertex(vertex, Vertexf(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: LineSegs::get_vertex_color
//       Access: Public
//  Description: Returns the color of the nth point or vertex/
////////////////////////////////////////////////////////////////////
INLINE Colorf LineSegs::
get_vertex_color(int vertex) const {
  nassertr(vertex >= 0 && vertex < (int)_created_colors.size(),
           Colorf(0.0f, 0.0f, 0.0f, 0.0f));
  return _created_colors[vertex];
}

////////////////////////////////////////////////////////////////////
//     Function: LineSegs::set_vertex_color
//       Access: Public
//  Description: Changes the vertex color of the nth point or vertex.
//               See set_vertex().
////////////////////////////////////////////////////////////////////
INLINE void LineSegs::
set_vertex_color(int vertex, const Colorf &color) {
  nassertv(vertex >= 0 && vertex < (int)_created_verts.size());
  _created_colors[vertex] = color;
}

////////////////////////////////////////////////////////////////////
//     Function: LineSegs::set_vertex_color
//       Access: Public
//  Description: Changes the vertex color of the nth point or vertex.
//               See set_vertex().
////////////////////////////////////////////////////////////////////
INLINE void LineSegs::
set_vertex_color(int vertex, float r, float g, float b, float a) {
  set_vertex_color(vertex, Colorf(r, g, b, a));
}
