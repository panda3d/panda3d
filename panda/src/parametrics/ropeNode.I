// Filename: ropeNode.I
// Created by:  drose (04Dec02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: RopeNode::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RopeNode::CData::
CData() {
  _curve = new NurbsCurveEvaluator;
  _render_mode = RopeNode::RM_thread;
  _uv_mode = RopeNode::UV_none;
  _uv_scale.set(1.0f, 1.0f);
  _use_vertex_color = false;
  _num_subdiv = 10;
  _thickness = 1.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: RopeNode::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RopeNode::CData::
CData(const RopeNode::CData &copy) :
  _curve(copy._curve),
  _render_mode(copy._render_mode),
  _uv_mode(copy._uv_mode),
  _uv_scale(copy._uv_scale),
  _use_vertex_color(copy._use_vertex_color),
  _num_subdiv(copy._num_subdiv),
  _thickness(copy._thickness)
{
}

////////////////////////////////////////////////////////////////////
//     Function: set_curve
//       Access: Public
//  Description: Sets the particular curve represented by the
//               RopeNode.
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
set_curve(NurbsCurveEvaluator *curve) {
  CDWriter cdata(_cycler);
  cdata->_curve = curve;
}

////////////////////////////////////////////////////////////////////
//     Function: get_curve
//       Access: Public
//  Description: Returns the curve represented by the RopeNode.
////////////////////////////////////////////////////////////////////
INLINE NurbsCurveEvaluator *RopeNode::
get_curve() const {
  CDReader cdata(_cycler);
  return cdata->_curve;
}

////////////////////////////////////////////////////////////////////
//     Function: set_render_mode
//       Access: Public
//  Description: Specifies the method used to render the rope.  The
//               simplest is RM_thread, which just draws a one-pixel
//               line segment.
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
set_render_mode(RopeNode::RenderMode render_mode) {
  CDWriter cdata(_cycler);
  cdata->_render_mode = render_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: get_render_mode
//       Access: Public
//  Description: Returns the method used to render the rope.  See
//               set_render_mode().
////////////////////////////////////////////////////////////////////
INLINE RopeNode::RenderMode RopeNode::
get_render_mode() const {
  CDReader cdata(_cycler);
  return cdata->_render_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: set_uv_mode
//       Access: Public
//  Description: Specifies the algorithm to use to generate UV's for
//               the rope.
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
set_uv_mode(RopeNode::UVMode uv_mode) {
  CDWriter cdata(_cycler);
  cdata->_uv_mode = uv_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: get_uv_mode
//       Access: Public
//  Description: Returns the algorithm to use to generate UV's for the
//               rope.
////////////////////////////////////////////////////////////////////
INLINE RopeNode::UVMode RopeNode::
get_uv_mode() const {
  CDReader cdata(_cycler);
  return cdata->_uv_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: set_uv_scale
//       Access: Public
//  Description: Specifies an additional scaling factor to apply to
//               generated UV's for the rope.
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
set_uv_scale(const LVecBase2f &uv_scale) {
  CDWriter cdata(_cycler);
  cdata->_uv_scale = uv_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: get_uv_scale
//       Access: Public
//  Description: Returns the scaling factor to apply to generated UV's
//               for the rope.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase2f &RopeNode::
get_uv_scale() const {
  CDReader cdata(_cycler);
  return cdata->_uv_scale;
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
INLINE void RopeNode::
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
INLINE bool RopeNode::
get_use_vertex_color() const {
  CDReader cdata(_cycler);
  return cdata->_use_vertex_color;
}

////////////////////////////////////////////////////////////////////
//     Function: set_num_subdiv
//       Access: Public
//  Description: Specifies the number of subdivisions per cubic
//               segment (that is, per unique knot value) to draw in a
//               fixed uniform tesselation of the curve.
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
set_num_subdiv(int num_subdiv) {
  nassertv(num_subdiv >= 0);
  CDWriter cdata(_cycler);
  cdata->_num_subdiv = num_subdiv;
}

////////////////////////////////////////////////////////////////////
//     Function: get_num_subdiv
//       Access: Public
//  Description: Returns the number of subdivisions per cubic segment
//               to draw.  See set_num_subdiv().
////////////////////////////////////////////////////////////////////
INLINE int RopeNode::
get_num_subdiv() const {
  CDReader cdata(_cycler);
  return cdata->_num_subdiv;
}

////////////////////////////////////////////////////////////////////
//     Function: set_thickness
//       Access: Public
//  Description: Specifies the thickness of the rope, in pixels or in
//               spatial units, depending on the render mode.  See
//               set_render_mode().
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
set_thickness(float thickness) {
  nassertv(thickness >= 0);
  CDWriter cdata(_cycler);
  cdata->_thickness = thickness;
}

////////////////////////////////////////////////////////////////////
//     Function: get_thickness
//       Access: Public
//  Description: Returns the thickness of the rope.  See
//               set_thickness().
////////////////////////////////////////////////////////////////////
INLINE float RopeNode::
get_thickness() const {
  CDReader cdata(_cycler);
  return cdata->_thickness;
}
