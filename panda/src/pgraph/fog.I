// Filename: fog.I
// Created by:  drose (14Mar02)
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
//     Function: Fog::get_mode
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Fog::Mode Fog::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::set_mode
//       Access: Published
//  Description: Specifies the computation that is used to determine
//               the fog effect.  If this is M_linear, then the fog
//               will range from linearly from the onset point to the
//               opaque point (or for the distances specified in
//               set_linear_range), and the fog object should be
//               parented into the scene graph, or to the camera.
//
//               If this is anything else, the onset point and opaque
//               point are not used, and the fog effect is based on
//               the value specified to set_exp_density(), and it
//               doesn't matter to which node the fog object is
//               parented, or if it is parented anywhere at all.
////////////////////////////////////////////////////////////////////
INLINE void Fog::
set_mode(Mode mode) {
  _mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::get_color
//       Access: Published
//  Description: Returns the color of the fog.
////////////////////////////////////////////////////////////////////
INLINE const LColor &Fog::
get_color() const {
  return _color;
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::set_color
//       Access: Published
//  Description: Sets the color of the fog.
////////////////////////////////////////////////////////////////////
INLINE void Fog::
set_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b) {
  _color[0] = r;
  _color[1] = g;
  _color[2] = b;
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::set_color
//       Access: Published
//  Description: Sets the color of the fog.  The alpha component is
//               not used.
////////////////////////////////////////////////////////////////////
INLINE void Fog::
set_color(const LColor &color) {
  _color = color;
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::set_linear_range
//       Access: Published
//  Description: Specifies the effects of the fog in linear distance
//               units.  This is only used if the mode is M_linear.
//
//               This specifies a fog that begins at distance onset
//               units from the origin, and becomes totally opaque at
//               distance opaque units from the origin, along the
//               forward axis (usually Y).
//
//               This function also implicitly sets the mode the
//               M_linear, if it is not already set.
////////////////////////////////////////////////////////////////////
INLINE void Fog::
set_linear_range(PN_stdfloat onset, PN_stdfloat opaque) {
  LVector3 forward = LVector3::forward();
  _linear_onset_point = onset * forward;
  _linear_opaque_point = opaque * forward;
  _transformed_onset = onset;
  _transformed_opaque = opaque;
  _mode = M_linear;
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::get_linear_onset_point
//       Access: Published
//  Description: Returns the point in space at which the fog begins.
//               This is only used if the mode is M_linear.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3 &Fog::
get_linear_onset_point() const {
  return _linear_onset_point;
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::set_linear_onset_point
//       Access: Published
//  Description: Specifies the point in space at which the fog begins.
//               This is only used if the mode is M_linear.
////////////////////////////////////////////////////////////////////
INLINE void Fog::
set_linear_onset_point(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  _linear_onset_point.set(x, y, z);
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::set_linear_onset_point
//       Access: Published
//  Description: Specifies the point in space at which the fog begins.
//               This is only used if the mode is M_linear.
////////////////////////////////////////////////////////////////////
INLINE void Fog::
set_linear_onset_point(const LPoint3 &linear_onset_point) {
  _linear_onset_point = linear_onset_point;
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::get_linear_opaque_point
//       Access: Published
//  Description: Returns the point in space at which the fog
//               completely obscures geometry.  This is only used if
//               the mode is M_linear.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3 &Fog::
get_linear_opaque_point() const {
  return _linear_opaque_point;
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::set_linear_opaque_point
//       Access: Published
//  Description: Specifies the point in space at which the fog
//               completely obscures geometry.  This is only used if
//               the mode is M_linear.
////////////////////////////////////////////////////////////////////
INLINE void Fog::
set_linear_opaque_point(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  _linear_opaque_point.set(x, y, z);
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::set_linear_opaque_point
//       Access: Published
//  Description: Specifies the point in space at which the fog
//               completely obscures geometry.  This is only used if
//               the mode is M_linear.
////////////////////////////////////////////////////////////////////
INLINE void Fog::
set_linear_opaque_point(const LPoint3 &linear_opaque_point) {
  _linear_opaque_point = linear_opaque_point;
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::set_linear_fallback
//       Access: Published
//  Description: Fog effects are traditionally defined in
//               camera-relative space, but the Panda Fog node has a
//               special mode in which it can define a linear fog
//               effect in an arbitrary coordinate space.
//
//               This is done by specifying 3-d onset and opaque
//               points, and parenting the Fog object somewhere within
//               the scene graph.  In this mode, the fog will be
//               rendered as if it extended along the vector from the
//               onset point to the opaque point, in 3-d space.
//
//               However, the underlying fog effect supported by
//               hardware is generally only one-dimensional, and must
//               be rendered based on linear distance from the camera
//               plane.  Thus, this in-the-world effect is most
//               effective when the fog vector from onset point to
//               opaque point is most nearly parallel to the camera's
//               eye vector.
//
//               As the angle between the fog vector and the eye
//               vector increases, the accuracy of the effect
//               diminishes, up to a complete breakdown of the effect
//               at a 90 degree angle.
//
//               This function exists to define the workaround to this
//               problem.  The linear fallback parameters given here
//               specify how the fog should be rendered when the
//               parameters are exceeded in this way.
//
//               The angle parameter is the minimum angle, in degrees,
//               of the fog vector to the eye vector, at which the
//               fallback effect should be employed.  The onset and
//               opaque parameters specify the camera-relative onset
//               and opaque distances to pass to the rendering
//               hardware when employing the fallback effect.  This
//               supercedes the 3-d onset point and opaque points.
////////////////////////////////////////////////////////////////////
INLINE void Fog::
set_linear_fallback(PN_stdfloat angle, PN_stdfloat onset, PN_stdfloat opaque) {
  _linear_fallback_cosa = ccos(deg_2_rad(angle));
  _linear_fallback_onset = onset;
  _linear_fallback_opaque = opaque;
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::get_exp_density
//       Access: Published
//  Description: Returns the density of the fog for exponential
//               calculations.  This is only used if the mode is not
//               M_linear.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat Fog::
get_exp_density() const {
  return _exp_density;
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::set_exp_density
//       Access: Published
//  Description: Sets the density of the fog for exponential
//               calculations.  This is only used if the mode is not
//               M_linear.
//
//               If the mode is currently set to M_linear, this
//               function implicitly sets it to M_exponential.
////////////////////////////////////////////////////////////////////
INLINE void Fog::
set_exp_density(PN_stdfloat exp_density) {
  nassertv((exp_density >= 0.0) && (exp_density <= 1.0));
  _exp_density = exp_density;

  if (_mode == M_linear) {
    _mode = M_exponential;
  }
}
