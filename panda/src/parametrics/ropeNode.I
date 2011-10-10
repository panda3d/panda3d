// Filename: ropeNode.I
// Created by:  drose (04Dec02)
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
//     Function: RopeNode::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RopeNode::CData::
CData() {
  _curve = new NurbsCurveEvaluator;
  _render_mode = RopeNode::RM_thread;
  _uv_mode = RopeNode::UV_none;
  _u_dominant = true;
  _uv_scale = 1.0f;
  _normal_mode = RopeNode::NM_none;
  _tube_up = LVector3::up();
  _matrix = LMatrix4::ident_mat();
  _has_matrix = false;
  _use_vertex_color = false;
  _num_subdiv = 10;
  _num_slices = 5;
  _use_vertex_thickness = false;
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
  _u_dominant(copy._u_dominant),
  _uv_scale(copy._uv_scale),
  _normal_mode(copy._normal_mode),
  _tube_up(copy._tube_up),
  _matrix(copy._matrix),
  _has_matrix(copy._has_matrix),
  _use_vertex_color(copy._use_vertex_color),
  _num_subdiv(copy._num_subdiv),
  _num_slices(copy._num_slices),
  _use_vertex_thickness(copy._use_vertex_thickness),
  _thickness(copy._thickness)
{
}

////////////////////////////////////////////////////////////////////
//     Function: set_curve
//       Access: Published
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
//       Access: Published
//  Description: Returns the curve represented by the RopeNode.
////////////////////////////////////////////////////////////////////
INLINE NurbsCurveEvaluator *RopeNode::
get_curve() const {
  CDReader cdata(_cycler);
  return cdata->_curve;
}

////////////////////////////////////////////////////////////////////
//     Function: set_render_mode
//       Access: Published
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
//       Access: Published
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
//       Access: Published
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
//       Access: Published
//  Description: Returns the algorithm to use to generate UV's for the
//               rope.
////////////////////////////////////////////////////////////////////
INLINE RopeNode::UVMode RopeNode::
get_uv_mode() const {
  CDReader cdata(_cycler);
  return cdata->_uv_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: set_uv_direction
//       Access: Published
//  Description: Specify true to vary the U coordinate down the length
//               of the rope, or false to vary the V coordinate.
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
set_uv_direction(bool u_dominant) {
  CDWriter cdata(_cycler);
  cdata->_u_dominant = u_dominant;
}

////////////////////////////////////////////////////////////////////
//     Function: get_uv_direction
//       Access: Published
//  Description: Returns true if the rope runs down the U coordinate
//               of the texture, or false if it runs down the V
//               coordinate.
////////////////////////////////////////////////////////////////////
INLINE bool RopeNode::
get_uv_direction() const {
  CDReader cdata(_cycler);
  return cdata->_u_dominant;
}

////////////////////////////////////////////////////////////////////
//     Function: set_uv_scale
//       Access: Published
//  Description: Specifies an additional scaling factor to apply to
//               generated UV's along the rope.  This scale factor is
//               applied in whichever direction is along the rope, as
//               specified by set_uv_direction().
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
set_uv_scale(PN_stdfloat uv_scale) {
  CDWriter cdata(_cycler);
  cdata->_uv_scale = uv_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: get_uv_scale
//       Access: Published
//  Description: Returns the scaling factor to apply to generated UV's
//               for the rope.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat RopeNode::
get_uv_scale() const {
  CDReader cdata(_cycler);
  return cdata->_uv_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: set_normal_mode
//       Access: Published
//  Description: Specifies the kind of normals to generate for the
//               rope.  This is only applicable when the RenderMode is
//               set to RM_tube; in the other render modes, normals
//               are never generated.
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
set_normal_mode(RopeNode::NormalMode normal_mode) {
  CDWriter cdata(_cycler);
  cdata->_normal_mode = normal_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: get_normal_mode
//       Access: Published
//  Description: Returns the kind of normals to generate for the rope.
//               This is only applicable when the RenderMode is set to
//               RM_tube.
////////////////////////////////////////////////////////////////////
INLINE RopeNode::NormalMode RopeNode::
get_normal_mode() const {
  CDReader cdata(_cycler);
  return cdata->_normal_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: set_tube_up
//       Access: Published
//  Description: Specifies a normal vector, generally perpendicular to
//               the main axis of the starting point of the curve,
//               that controls the "top" of the curve, when RenderMode
//               is RM_tube.  This is used to orient the vertices that
//               make up the tube.  If this vector is too nearly
//               parallel with the starting direction of the curve,
//               there may be a tendency for the whole tube to
//               gimble-lock around its primary axis.
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
set_tube_up(const LVector3 &tube_up) {
  CDWriter cdata(_cycler);
  cdata->_tube_up = tube_up;
}

////////////////////////////////////////////////////////////////////
//     Function: get_tube_up
//       Access: Published
//  Description: Returns the normal vector used to control the "top"
//               of the curve, when RenderMode is RM_tube.  See
//               set_tube_up().
////////////////////////////////////////////////////////////////////
INLINE const LVector3 &RopeNode::
get_tube_up() const {
  CDReader cdata(_cycler);
  return cdata->_tube_up;
}

////////////////////////////////////////////////////////////////////
//     Function: set_use_vertex_color
//       Access: Published
//  Description: Sets the "use vertex color" flag.  When this is true,
//               the R, G, B, A vertex color is assumed to be stored
//               as the dimensions n + 0, n + 1, n + 2, n + 3,
//               respectively, of the extended vertex values, where n
//               is the value returned by
//               get_vertex_color_dimension().  Use
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
//       Access: Published
//  Description: Returns the "use vertex color" flag.  See
//               set_use_vertex_color().
////////////////////////////////////////////////////////////////////
INLINE bool RopeNode::
get_use_vertex_color() const {
  CDReader cdata(_cycler);
  return cdata->_use_vertex_color;
}

////////////////////////////////////////////////////////////////////
//     Function: get_vertex_color_dimension
//       Access: Published, Static
//  Description: Returns the numeric extended dimension in which the
//               color components should be found.  See
//               NurbsCurveEvaluator::set_extended_vertex().
//
//               The color components will be expected at (n, n + 1, n
//               + 2, n + 3).
////////////////////////////////////////////////////////////////////
INLINE int RopeNode::
get_vertex_color_dimension() {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: set_num_subdiv
//       Access: Published
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
//       Access: Published
//  Description: Returns the number of subdivisions per cubic segment
//               to draw.  See set_num_subdiv().
////////////////////////////////////////////////////////////////////
INLINE int RopeNode::
get_num_subdiv() const {
  CDReader cdata(_cycler);
  return cdata->_num_subdiv;
}

////////////////////////////////////////////////////////////////////
//     Function: set_num_slices
//       Access: Published
//  Description: Specifies the number of radial subdivisions to make
//               if RenderMode is RM_tube.  It is ignored in the other
//               render modes.
//
//               Increasing this number increases the roundness of a
//               cross-section of the tube.  The minimum value for a
//               dimensional tube is 3; setting it to 2 will get you a
//               thin piece of tape (which is similar to RM_billboard,
//               except it won't rotate to face the camera).
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
set_num_slices(int num_slices) {
  nassertv(num_slices >= 0);
  CDWriter cdata(_cycler);
  cdata->_num_slices = num_slices;
}

////////////////////////////////////////////////////////////////////
//     Function: get_num_slices
//       Access: Published
//  Description: Returns the number of radial subdivisions to make if
//               RenderMode is RM_tube.  It is ignored in the other
//               render modes.  See set_num_slices().
////////////////////////////////////////////////////////////////////
INLINE int RopeNode::
get_num_slices() const {
  CDReader cdata(_cycler);
  return cdata->_num_slices;
}

////////////////////////////////////////////////////////////////////
//     Function: set_use_vertex_thickness
//       Access: Published
//  Description: Sets the "use vertex thickness" flag.  When this is true,
//               the vertex thickness is assumed to be stored as the
//               dimension get_vertex_thickness_dimension(), of the
//               extended vertex values.  Use
//               NurbsCurveEvaluator::set_extended_vertex() to set
//               these values.
//
//               In this mode, the overall thickness is also applied
//               as a scale to the vertex thickness.  Not all render
//               modes support vertex thickness.
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
set_use_vertex_thickness(bool flag) {
  CDWriter cdata(_cycler);
  cdata->_use_vertex_thickness = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: get_use_vertex_thickness
//       Access: Published
//  Description: Returns the "use vertex thickness" flag.  See
//               set_use_vertex_thickness().
////////////////////////////////////////////////////////////////////
INLINE bool RopeNode::
get_use_vertex_thickness() const {
  CDReader cdata(_cycler);
  return cdata->_use_vertex_thickness;
}

////////////////////////////////////////////////////////////////////
//     Function: get_vertex_thickness_dimension
//       Access: Published, Static
//  Description: Returns the numeric extended dimension in which the
//               thickness component should be found.  See
//               NurbsCurveEvaluator::set_extended_vertex().
////////////////////////////////////////////////////////////////////
INLINE int RopeNode::
get_vertex_thickness_dimension() {
  return 4;
}

////////////////////////////////////////////////////////////////////
//     Function: set_thickness
//       Access: Published
//  Description: Specifies the thickness of the rope, in pixels or in
//               spatial units, depending on the render mode.  See
//               set_render_mode().
//
//               The thickness may also be specified on a per-vertex
//               basis.  See set_use_vertex_thickness().
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
set_thickness(PN_stdfloat thickness) {
  nassertv(thickness >= 0);
  CDWriter cdata(_cycler);
  cdata->_thickness = thickness;
}

////////////////////////////////////////////////////////////////////
//     Function: get_thickness
//       Access: Published
//  Description: Returns the thickness of the rope.  See
//               set_thickness().
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat RopeNode::
get_thickness() const {
  CDReader cdata(_cycler);
  return cdata->_thickness;
}

////////////////////////////////////////////////////////////////////
//     Function: set_matrix
//       Access: Published
//  Description: Specifies an optional matrix which is used to
//               transform each control vertex after it has been
//               transformed into the RopeNode's coordinate space, but
//               before the polygon vertices are generated.
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
set_matrix(const LMatrix4 &matrix) {
  CDWriter cdata(_cycler);
  cdata->_matrix = matrix;
  cdata->_has_matrix = true;
}

////////////////////////////////////////////////////////////////////
//     Function: clear_matrix
//       Access: Published
//  Description: Resets the node's matrix to identity.  See
//               set_matrix().
////////////////////////////////////////////////////////////////////
INLINE void RopeNode::
clear_matrix() {
  CDWriter cdata(_cycler);
  cdata->_matrix = LMatrix4::ident_mat();
  cdata->_has_matrix = false;
}

////////////////////////////////////////////////////////////////////
//     Function: has_matrix
//       Access: Published
//  Description: Returns true if the node has a matrix set, false
//               otherwise.  See set_matrix().
////////////////////////////////////////////////////////////////////
INLINE bool RopeNode::
has_matrix() const {
  CDReader cdata(_cycler);
  return cdata->_has_matrix;
}

////////////////////////////////////////////////////////////////////
//     Function: get_matrix
//       Access: Published
//  Description: Returns the optional matrix which is used to
//               transform each control vertex after it has been
//               transformed into the RopeNode's coordinate space, but
//               before the polygon vertices are generated.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4 &RopeNode::
get_matrix() const {
  CDReader cdata(_cycler);
  return cdata->_matrix;
}
