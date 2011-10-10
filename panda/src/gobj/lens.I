// Filename: lens.I
// Created by:  drose (29Nov01)
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
//     Function: Lens::extrude
//       Access: Published
//  Description: Given a 2-d point in the range (-1,1) in both
//               dimensions, where (0,0) is the center of the
//               lens and (-1,-1) is the lower-left corner,
//               compute the corresponding vector in space that maps
//               to this point, if such a vector can be determined.
//               The vector is returned by indicating the points on
//               the near plane and far plane that both map to the
//               indicated 2-d point.
//
//               Returns true if the vector is defined, or false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Lens::
extrude(const LPoint2 &point2d, LPoint3 &near_point, LPoint3 &far_point) const {
  return extrude_impl(LPoint3(point2d[0], point2d[1], 0.0f),
                      near_point, far_point);
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::extrude
//       Access: Published
//  Description: Given a 2-d point in the range (-1,1) in both
//               dimensions, where (0,0) is the center of the
//               lens and (-1,-1) is the lower-left corner,
//               compute the corresponding vector in space that maps
//               to this point, if such a vector can be determined.
//               The vector is returned by indicating the points on
//               the near plane and far plane that both map to the
//               indicated 2-d point.
//
//               The z coordinate of the 2-d point is ignored.
//
//               Returns true if the vector is defined, or false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Lens::
extrude(const LPoint3 &point2d, LPoint3 &near_point, LPoint3 &far_point) const {
  return extrude_impl(point2d, near_point, far_point);
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::extrude_vec
//       Access: Published
//  Description: Given a 2-d point in the range (-1,1) in both
//               dimensions, where (0,0) is the center of the
//               lens and (-1,-1) is the lower-left corner,
//               compute the vector that corresponds to the view
//               direction.  This will be parallel to the normal on
//               the surface (the far plane) corresponding to the lens
//               shape at this point.
//
//               See the comment block on Lens::extrude_vec_impl() for
//               a more in-depth comment on the meaning of this
//               vector.
//
//               Returns true if the vector is defined, or false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Lens::
extrude_vec(const LPoint2 &point2d, LVector3 &vec) const {
  return extrude_vec_impl(LPoint3(point2d[0], point2d[1], 0.0f), vec);
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::extrude_vec
//       Access: Published
//  Description: Given a 2-d point in the range (-1,1) in both
//               dimensions, where (0,0) is the center of the
//               lens and (-1,-1) is the lower-left corner,
//               compute the vector that corresponds to the view
//               direction.  This will be parallel to the normal on
//               the surface (the far plane) corresponding to the lens
//               shape at this point.
//
//               See the comment block on Lens::extrude_vec_impl() for
//               a more in-depth comment on the meaning of this
//               vector.
//
//               The z coordinate of the 2-d point is ignored.
//
//               Returns true if the vector is defined, or false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool Lens::
extrude_vec(const LPoint3 &point2d, LVector3 &vec) const {
  return extrude_vec_impl(point2d, vec);
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::project
//       Access: Published
//  Description: Given a 3-d point in space, determine the 2-d point
//               this maps to, in the range (-1,1) in both dimensions,
//               where (0,0) is the center of the lens and
//               (-1,-1) is the lower-left corner.
//
//               Returns true if the 3-d point is in front of the lens
//               and within the viewing frustum (in which case point2d
//               is filled in), or false otherwise (in which case
//               point2d will be filled in with something, which may
//               or may not be meaningful).
////////////////////////////////////////////////////////////////////
INLINE bool Lens::
project(const LPoint3 &point3d, LPoint2 &point2d) const {
  LPoint3 result;
  bool okflag = project_impl(point3d, result);
  point2d.set(result[0], result[1]);
  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::project
//       Access: Published
//  Description: Given a 3-d point in space, determine the 2-d point
//               this maps to, in the range (-1,1) in both dimensions,
//               where (0,0) is the center of the lens and
//               (-1,-1) is the lower-left corner.
//
//               The z coordinate will also be set to a value in the
//               range (-1, 1), where 1 represents a point on the near
//               plane, and -1 represents a point on the far plane.
//
//               Returns true if the 3-d point is in front of the lens
//               and within the viewing frustum (in which case point2d
//               is filled in), or false otherwise (in which case
//               point2d will be filled in with something, which may
//               or may not be meaningful).
////////////////////////////////////////////////////////////////////
INLINE bool Lens::
project(const LPoint3 &point3d, LPoint3 &point2d) const {
  return project_impl(point3d, point2d);
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::set_change_event
//       Access: Published
//  Description: Sets the name of the event that will be generated
//               whenever any properties of the Lens have
//               changed.  If this is not set for a particular lens,
//               no event will be generated.
//
//               The event is thrown with one parameter, the lens
//               itself.  This can be used to automatically track
//               changes to camera fov, etc. in the application.
////////////////////////////////////////////////////////////////////
INLINE void Lens::
set_change_event(const string &event) {
  _change_event = event;
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::get_change_event
//       Access: Published
//  Description: Returns the name of the event that will be generated
//               whenever any properties of this particular Lens have
//               changed.
////////////////////////////////////////////////////////////////////
INLINE const string &Lens::
get_change_event() const {
  return _change_event;
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::get_coordinate_system
//       Access: Published
//  Description: Returns the coordinate system that all 3-d
//               computations are performed within for this
//               Lens.  Normally, this is CS_default.
////////////////////////////////////////////////////////////////////
INLINE CoordinateSystem Lens::
get_coordinate_system() const {
  return _cs;
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::set_film_size
//       Access: Published
//  Description: Sets the size and shape of the "film" within the
//               lens.  This both establishes the units used by
//               calls like set_focal_length(), and establishes the
//               aspect ratio of the frame.
//
//               In a physical camera, the field of view of a lens is
//               determined by the lens' focal length and by the size
//               of the film area exposed by the lens.  For instance,
//               a 35mm camera exposes a rectangle on the film about
//               24mm x 36mm, which means a 50mm lens gives about a
//               40-degree horizontal field of view.
//
//               In the virtual camera, you may set the film size to
//               any units here, and specify a focal length in the
//               same units to simulate the same effect.  Or, you may
//               ignore this parameter, and specify the field of view
//               and aspect ratio of the lens directly.
////////////////////////////////////////////////////////////////////
INLINE void Lens::
set_film_size(PN_stdfloat width, PN_stdfloat height) {
  set_film_size(LVecBase2(width, height));
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::set_film_offset
//       Access: Published
//  Description: Sets the horizontal and vertical offset amounts of
//               this Lens.  These are both in the same units
//               specified in set_film_size().
//
//               This can be used to establish an off-axis lens.
////////////////////////////////////////////////////////////////////
INLINE void Lens::
set_film_offset(PN_stdfloat x, PN_stdfloat y) {
  set_film_offset(LVecBase2(x, y));
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::set_film_offset
//       Access: Published
//  Description: Sets the horizontal and vertical offset amounts of
//               this Lens.  These are both in the same units
//               specified in set_film_size().
//
//               This can be used to establish an off-axis lens.
////////////////////////////////////////////////////////////////////
INLINE void Lens::
set_film_offset(const LVecBase2 &film_offset) {
  _film_offset = film_offset;
  adjust_comp_flags(CF_mat, 0);
  throw_change_event();
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::get_film_offset
//       Access: Published
//  Description: Returns the horizontal and vertical offset amounts of
//               this Lens.  See set_film_offset().
////////////////////////////////////////////////////////////////////
INLINE const LVector2 &Lens::
get_film_offset() const {
  return _film_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::set_fov
//       Access: Published
//  Description: Sets the field of view of the lens in both
//               dimensions.  This establishes both the field of view
//               and the aspect ratio of the lens.  This is one way to
//               specify the field of view of a lens;
//               set_focal_length() is another way.
//
//               For certain kinds of lenses (like OrthoLens),
//               the field of view has no meaning.
////////////////////////////////////////////////////////////////////
INLINE void Lens::
set_fov(PN_stdfloat hfov, PN_stdfloat vfov) {
  set_fov(LVecBase2(hfov, vfov));
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::get_hfov
//       Access: Published
//  Description: Returns the horizontal component of fov only.  See
//               get_fov().
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat Lens::
get_hfov() const {
  return get_fov()[0];
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::get_vfov
//       Access: Published
//  Description: Returns the vertical component of fov only.  See
//               get_fov().
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat Lens::
get_vfov() const {
  return get_fov()[1];
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::set_near
//       Access: Published
//  Description: Defines the position of the near plane (or cylinder,
//               sphere, whatever).  Points closer to the lens than
//               this may not be rendered.
////////////////////////////////////////////////////////////////////
INLINE void Lens::
set_near(PN_stdfloat near_distance) {
  _near_distance = near_distance;
  adjust_comp_flags(CF_projection_mat | CF_projection_mat_inv, 0);
  throw_change_event();
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::get_near
//       Access: Published
//  Description: Returns the position of the near plane (or cylinder,
//               sphere, whatever).
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat Lens::
get_near() const {
  return _near_distance;
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::set_far
//       Access: Published
//  Description: Defines the position of the far plane (or cylinder,
//               sphere, whatever).  Points farther from the lens than
//               this may not be rendered.
////////////////////////////////////////////////////////////////////
INLINE void Lens::
set_far(PN_stdfloat far_distance) {
  _far_distance = far_distance;
  adjust_comp_flags(CF_projection_mat | CF_projection_mat_inv, 0);
  throw_change_event();
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::get_far
//       Access: Published
//  Description: Returns the position of the far plane (or cylinder,
//               sphere, whatever).
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat Lens::
get_far() const {
  return _far_distance;
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::set_near_far
//       Access: Published
//  Description: Simultaneously changes the near and far planes.
////////////////////////////////////////////////////////////////////
INLINE void Lens::
set_near_far(PN_stdfloat near_distance, PN_stdfloat far_distance) {
  _near_distance = near_distance;
  _far_distance = far_distance;
  adjust_comp_flags(CF_projection_mat | CF_projection_mat_inv, 0);
  throw_change_event();
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::set_view_hpr
//       Access: Published
//  Description: Sets the direction in which the lens is facing.
//               Normally, this is down the forward axis (usually the
//               Y axis), but it may be rotated.  This is only one way
//               of specifying the rotation; you may also specify an
//               explicit vector in which to look, or you may give a
//               complete transformation matrix.
////////////////////////////////////////////////////////////////////
INLINE void Lens::
set_view_hpr(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) {
  set_view_hpr(LVecBase3(h, p, r));
}
////////////////////////////////////////////////////////////////////
//     Function: Lens::set_view_vector
//       Access: Published
//  Description: Specifies the direction in which the lens is facing
//               by giving an axis to look along, and a perpendicular
//               (or at least non-parallel) up axis.
//
//               See also set_view_hpr().
////////////////////////////////////////////////////////////////////
INLINE void Lens::
set_view_vector(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z, PN_stdfloat i, PN_stdfloat j, PN_stdfloat k) {
  set_view_vector(LVector3(x, y, z), LVector3(i, j, k));
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::get_keystone
//       Access: Published
//  Description: Returns the keystone correction specified for the
//               lens.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase2 &Lens::
get_keystone() const {
  return _keystone;
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::get_last_change
//       Access: Public
//  Description: Returns the UpdateSeq that is incremented whenever
//               the lens properties are changed.  As long as this
//               number remains the same, you may assume the lens
//               properties are unchanged.
////////////////////////////////////////////////////////////////////
INLINE const UpdateSeq &Lens::
get_last_change() const {
  return _last_change;
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::adjust_user_flags
//       Access: Protected
//  Description: Clears from _user_flags the bits in the first
//               parameter, and sets the bits in the second parameter.
////////////////////////////////////////////////////////////////////
INLINE void Lens::
adjust_user_flags(int clear_flags, int set_flags) {
  _user_flags = (_user_flags & ~clear_flags) | set_flags;
}

////////////////////////////////////////////////////////////////////
//     Function: Lens::adjust_comp_flags
//       Access: Protected
//  Description: Clears from _comp_flags the bits in the first
//               parameter, and sets the bits in the second parameter.
////////////////////////////////////////////////////////////////////
INLINE void Lens::
adjust_comp_flags(int clear_flags, int set_flags) {
  _comp_flags = (_comp_flags & ~clear_flags) | set_flags;
}

INLINE ostream &
operator << (ostream &out, const Lens &lens) {
  lens.output(out);
  return out;
}

