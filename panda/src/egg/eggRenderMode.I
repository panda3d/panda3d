// Filename: eggRenderMode.I
// Created by:  drose (20Jan99)
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
//     Function: EggRenderMode::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggRenderMode::
EggRenderMode(const EggRenderMode &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::set_depth_write_mode
//       Access: Public
//  Description: Specifies whether writes should be made to the depth
//               buffer (assuming the rendering backend provides a
//               depth buffer) when rendering this geometry.
////////////////////////////////////////////////////////////////////
INLINE void EggRenderMode::
set_depth_write_mode(DepthWriteMode mode) {
  _depth_write_mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::get_depth_write_mode
//       Access: Public
//  Description: Returns the depth_write mode that was set, or
//               DWM_unspecified if nothing was set.  See
//               set_depth_write_mode().
////////////////////////////////////////////////////////////////////
INLINE EggRenderMode::DepthWriteMode EggRenderMode::
get_depth_write_mode() const {
  return _depth_write_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::set_depth_test_mode
//       Access: Public
//  Description: Specifies whether this geometry should be tested
//               against the depth buffer when it is drawn (assuming
//               the rendering backend provides a depth buffer).  Note
//               that this is different, and independent from, the
//               depth_write mode.
////////////////////////////////////////////////////////////////////
INLINE void EggRenderMode::
set_depth_test_mode(DepthTestMode mode) {
  _depth_test_mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::get_depth_test_mode
//       Access: Public
//  Description: Returns the depth_test mode that was set, or
//               DTM_unspecified if nothing was set.  See
//               set_depth_test_mode().
////////////////////////////////////////////////////////////////////
INLINE EggRenderMode::DepthTestMode EggRenderMode::
get_depth_test_mode() const {
  return _depth_test_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::set_visibility_mode
//       Access: Public
//  Description: Specifies whether this geometry is to be considered
//               normally visible, or hidden.  If it is hidden, it is
//               either not loaded into the scene graph at all, or
//               loaded as a "stashed" node, according to the setting
//               of egg-suppress-hidden.
////////////////////////////////////////////////////////////////////
INLINE void EggRenderMode::
set_visibility_mode(VisibilityMode mode) {
  _visibility_mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::get_visibility_mode
//       Access: Public
//  Description: Returns the visibility mode that was set, or
//               VM_unspecified if nothing was set.  See
//               set_visibility_mode().
////////////////////////////////////////////////////////////////////
INLINE EggRenderMode::VisibilityMode EggRenderMode::
get_visibility_mode() const {
  return _visibility_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::set_alpha_mode
//       Access: Public
//  Description: Specifies precisely how the transparency for this
//               geometry should be achieved, or if it should be used.
//               The default, AM_unspecified, is to use transparency
//               if the geometry has a color whose alpha value is
//               non-1, or if it has a four-channel texture applied;
//               otherwise, AM_on forces transparency on, and AM_off
//               forces it off.  The other flavors of transparency are
//               specific ways to turn on transparency, which may or
//               may not be supported by a particular rendering
//               backend.
////////////////////////////////////////////////////////////////////
INLINE void EggRenderMode::
set_alpha_mode(AlphaMode mode) {
  _alpha_mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::get_alpha_mode
//       Access: Public
//  Description: Returns the alpha mode that was set, or
//               AM_unspecified if nothing was set.  See
//               set_alpha_mode().
////////////////////////////////////////////////////////////////////
INLINE EggRenderMode::AlphaMode EggRenderMode::
get_alpha_mode() const {
  return _alpha_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::set_depth_offset
//       Access: Public
//  Description: Sets the "depth-offset" flag associated with this
//               object.  This adds or subtracts an offset bias
//               into the depth buffer. See also DepthOffsetAttrib
//               and NodePath::set_depth_offset().
////////////////////////////////////////////////////////////////////
INLINE void EggRenderMode::
set_depth_offset(int order) {
  _depth_offset = order;
  _has_depth_offset = true;
}


////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::get_depth_offset
//       Access: Public
//  Description: Returns the "depth-offset" flag as set for this
//               particular object.  See set_depth_offset().
////////////////////////////////////////////////////////////////////
INLINE int EggRenderMode::
get_depth_offset() const {
  return _depth_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::has_depth_offset
//       Access: Public
//  Description: Returns true if the depth-offset flag has been set for
//               this particular object.  See set_depth_offset().
////////////////////////////////////////////////////////////////////
INLINE bool EggRenderMode::
has_depth_offset() const {
  return _has_depth_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::clear_depth_offset
//       Access: Public
//  Description: Removes the depth-offset flag from this particular
//               object.  See set_depth_offset().
////////////////////////////////////////////////////////////////////
INLINE void EggRenderMode::
clear_depth_offset() {
  _has_depth_offset = false;
}


////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::set_draw_order
//       Access: Public
//  Description: Sets the "draw-order" flag associated with this
//               object.  This specifies a particular order in which
//               objects of this type should be drawn, within the
//               specified bin.  If a bin is not explicitly specified,
//               "fixed" is used.  See also set_bin().
////////////////////////////////////////////////////////////////////
INLINE void EggRenderMode::
set_draw_order(int order) {
  _draw_order = order;
  _has_draw_order = true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::get_draw_order
//       Access: Public
//  Description: Returns the "draw-order" flag as set for this
//               particular object.  See set_draw_order().
////////////////////////////////////////////////////////////////////
INLINE int EggRenderMode::
get_draw_order() const {
  return _draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::has_draw_order
//       Access: Public
//  Description: Returns true if the draw-order flag has been set for
//               this particular object.  See set_draw_order().
////////////////////////////////////////////////////////////////////
INLINE bool EggRenderMode::
has_draw_order() const {
  return _has_draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::clear_draw_order
//       Access: Public
//  Description: Removes the draw-order flag from this particular
//               object.  See set_draw_order().
////////////////////////////////////////////////////////////////////
INLINE void EggRenderMode::
clear_draw_order() {
  _has_draw_order = false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::set_bin
//       Access: Public
//  Description: Sets the "bin" string for this particular object.
//               This names a particular bin in which the object
//               should be rendered.  The exact meaning of a bin is
//               implementation defined, but generally a GeomBin
//               matching each bin name must also be specifically
//               added to the rendering engine (e.g. the
//               CullTraverser) in use for this to work.  See also
//               set_draw_order().
////////////////////////////////////////////////////////////////////
INLINE void EggRenderMode::
set_bin(const string &bin) {
  _bin = bin;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::get_bin
//       Access: Public
//  Description: Returns the bin name that has been set for this
//               particular object, if any.  See set_bin().
////////////////////////////////////////////////////////////////////
INLINE string EggRenderMode::
get_bin() const {
  return _bin;
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::has_bin
//       Access: Public
//  Description: Returns true if a bin name has been set for this
//               particular object.  See set_bin().
////////////////////////////////////////////////////////////////////
INLINE bool EggRenderMode::
has_bin() const {
  return !_bin.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::clear_bin
//       Access: Public
//  Description: Removes the bin name that was set for this particular
//               object.  See set_bin().
////////////////////////////////////////////////////////////////////
INLINE void EggRenderMode::
clear_bin() {
  _bin = string();
}

////////////////////////////////////////////////////////////////////
//     Function: EggRenderMode::Inequality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggRenderMode::
operator != (const EggRenderMode &other) const {
  return !(*this == other);
}
