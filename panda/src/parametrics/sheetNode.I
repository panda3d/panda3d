// Filename: sheetNode.I
// Created by:  drose (11Oct03)
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
//     Function: SheetNode::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SheetNode::CData::
CData() {
  _surface = new NurbsSurfaceEvaluator;
  _use_vertex_color = false;
  _num_u_subdiv = 2;
  _num_v_subdiv = 2;
}

////////////////////////////////////////////////////////////////////
//     Function: SheetNode::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SheetNode::CData::
CData(const SheetNode::CData &copy) :
  _surface(copy._surface),
  _use_vertex_color(copy._use_vertex_color),
  _num_u_subdiv(copy._num_u_subdiv),
  _num_v_subdiv(copy._num_v_subdiv)
{
}

////////////////////////////////////////////////////////////////////
//     Function: set_surface
//       Access: Public
//  Description: Sets the particular surface represented by the
//               SheetNode.
////////////////////////////////////////////////////////////////////
INLINE void SheetNode::
set_surface(NurbsSurfaceEvaluator *surface) {
  CDWriter cdata(_cycler);
  cdata->_surface = surface;
}

////////////////////////////////////////////////////////////////////
//     Function: get_surface
//       Access: Public
//  Description: Returns the surface represented by the SheetNode.
////////////////////////////////////////////////////////////////////
INLINE NurbsSurfaceEvaluator *SheetNode::
get_surface() const {
  CDReader cdata(_cycler);
  return cdata->_surface;
}

////////////////////////////////////////////////////////////////////
//     Function: set_use_vertex_color
//       Access: Public
//  Description: Sets the "use vertex color" flag.  When this is true,
//               the R, G, B, A vertex color is assumed to be stored
//               as the dimensions 0, 1, 2, 3, respectively, of the
//               extended vertex values.  Use
//               NurbsCurveEvaluator::set_extended_vertex() to set
//               these values.
////////////////////////////////////////////////////////////////////
INLINE void SheetNode::
set_use_vertex_color(bool flag) {
  CDWriter cdata(_cycler);
  cdata->_use_vertex_color = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: get_use_vertex_color
//       Access: Public
//  Description: Returns the "use vertex color" flag.  See
//               set_use_vertex_color().
////////////////////////////////////////////////////////////////////
INLINE bool SheetNode::
get_use_vertex_color() const {
  CDReader cdata(_cycler);
  return cdata->_use_vertex_color;
}

////////////////////////////////////////////////////////////////////
//     Function: set_num_u_subdiv
//       Access: Public
//  Description: Specifies the number of subdivisions per cubic
//               segment (that is, per unique knot value) to draw in a
//               fixed uniform tesselation of the surface in the U
//               direction.
////////////////////////////////////////////////////////////////////
INLINE void SheetNode::
set_num_u_subdiv(int num_u_subdiv) {
  nassertv(num_u_subdiv >= 0);
  CDWriter cdata(_cycler);
  cdata->_num_u_subdiv = num_u_subdiv;
}

////////////////////////////////////////////////////////////////////
//     Function: get_num_u_subdiv
//       Access: Public
//  Description: Returns the number of subdivisions per cubic segment
//               to draw in the U direction.  See set_num_u_subdiv().
////////////////////////////////////////////////////////////////////
INLINE int SheetNode::
get_num_u_subdiv() const {
  CDReader cdata(_cycler);
  return cdata->_num_u_subdiv;
}

////////////////////////////////////////////////////////////////////
//     Function: set_num_v_subdiv
//       Access: Public
//  Description: Specifies the number of subdivisions per cubic
//               segment (that is, per unique knot value) to draw in a
//               fixed uniform tesselation of the surface in the V
//               direction.
////////////////////////////////////////////////////////////////////
INLINE void SheetNode::
set_num_v_subdiv(int num_v_subdiv) {
  nassertv(num_v_subdiv >= 0);
  CDWriter cdata(_cycler);
  cdata->_num_v_subdiv = num_v_subdiv;
}

////////////////////////////////////////////////////////////////////
//     Function: get_num_v_subdiv
//       Access: Public
//  Description: Returns the number of subdivisions per cubic segment
//               to draw in the V direction.  See set_num_v_subdiv().
////////////////////////////////////////////////////////////////////
INLINE int SheetNode::
get_num_v_subdiv() const {
  CDReader cdata(_cycler);
  return cdata->_num_v_subdiv;
}
