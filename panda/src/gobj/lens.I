/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lens.I
 * @author drose
 * @date 2001-11-29
 */

/**
 * Given a 2-d point in the range (-1,1) in both dimensions, where (0,0) is
 * the center of the lens and (-1,-1) is the lower-left corner, compute the
 * corresponding vector in space that maps to this point, if such a vector can
 * be determined.  The vector is returned by indicating the points on the near
 * plane and far plane that both map to the indicated 2-d point.
 *
 * Returns true if the vector is defined, or false otherwise.
 */
INLINE bool Lens::
extrude(const LPoint2 &point2d, LPoint3 &near_point, LPoint3 &far_point) const {
  CDReader cdata(_cycler);
  return do_extrude(cdata, LPoint3(point2d[0], point2d[1], 0.0f),
                    near_point, far_point);
}

/**
 * Given a 2-d point in the range (-1,1) in both dimensions, where (0,0) is
 * the center of the lens and (-1,-1) is the lower-left corner, compute the
 * corresponding vector in space that maps to this point, if such a vector can
 * be determined.  The vector is returned by indicating the points on the near
 * plane and far plane that both map to the indicated 2-d point.
 *
 * The z coordinate of the 2-d point is ignored.
 *
 * Returns true if the vector is defined, or false otherwise.
 */
INLINE bool Lens::
extrude(const LPoint3 &point2d, LPoint3 &near_point, LPoint3 &far_point) const {
  CDReader cdata(_cycler);
  return do_extrude(cdata, point2d, near_point, far_point);
}

/**
 * Uses the depth component of the 3-d result from project() to compute the
 * original point in 3-d space corresponding to a particular point on the
 * lens.  This exactly reverses project(), assuming the point does fall
 * legitimately within the lens.
 */
INLINE bool Lens::
extrude_depth(const LPoint3 &point2d, LPoint3 &point3d) const {
  CDReader cdata(_cycler);
  return do_extrude_depth(cdata, point2d, point3d);
}

/**
 * Given a 2-d point in the range (-1,1) in both dimensions, where (0,0) is
 * the center of the lens and (-1,-1) is the lower-left corner, compute the
 * vector that corresponds to the view direction.  This will be parallel to
 * the normal on the surface (the far plane) corresponding to the lens shape
 * at this point.
 *
 * See the comment block on Lens::extrude_vec_impl() for a more in-depth
 * comment on the meaning of this vector.
 *
 * Returns true if the vector is defined, or false otherwise.
 */
INLINE bool Lens::
extrude_vec(const LPoint2 &point2d, LVector3 &vec) const {
  CDReader cdata(_cycler);
  return do_extrude_vec(cdata, LPoint3(point2d[0], point2d[1], 0.0f), vec);
}

/**
 * Given a 2-d point in the range (-1,1) in both dimensions, where (0,0) is
 * the center of the lens and (-1,-1) is the lower-left corner, compute the
 * vector that corresponds to the view direction.  This will be parallel to
 * the normal on the surface (the far plane) corresponding to the lens shape
 * at this point.
 *
 * See the comment block on Lens::extrude_vec_impl() for a more in-depth
 * comment on the meaning of this vector.
 *
 * The z coordinate of the 2-d point is ignored.
 *
 * Returns true if the vector is defined, or false otherwise.
 */
INLINE bool Lens::
extrude_vec(const LPoint3 &point2d, LVector3 &vec) const {
  CDReader cdata(_cycler);
  return do_extrude_vec(cdata, point2d, vec);
}

/**
 * Given a 3-d point in space, determine the 2-d point this maps to, in the
 * range (-1,1) in both dimensions, where (0,0) is the center of the lens and
 * (-1,-1) is the lower-left corner.
 *
 * Returns true if the 3-d point is in front of the lens and within the
 * viewing frustum (in which case point2d is filled in), or false otherwise
 * (in which case point2d will be filled in with something, which may or may
 * not be meaningful).
 */
INLINE bool Lens::
project(const LPoint3 &point3d, LPoint2 &point2d) const {
  CDReader cdata(_cycler);
  LPoint3 result;
  bool okflag = do_project(cdata, point3d, result);
  point2d.set(result[0], result[1]);
  return okflag;
}

/**
 * Given a 3-d point in space, determine the 2-d point this maps to, in the
 * range (-1,1) in both dimensions, where (0,0) is the center of the lens and
 * (-1,-1) is the lower-left corner.
 *
 * The z coordinate will also be set to a value in the range (-1, 1), where 1
 * represents a point on the near plane, and -1 represents a point on the far
 * plane.
 *
 * Returns true if the 3-d point is in front of the lens and within the
 * viewing frustum (in which case point2d is filled in), or false otherwise
 * (in which case point2d will be filled in with something, which may or may
 * not be meaningful).
 */
INLINE bool Lens::
project(const LPoint3 &point3d, LPoint3 &point2d) const {
  CDReader cdata(_cycler);
  return do_project(cdata, point3d, point2d);
}

/**
 * Sets the name of the event that will be generated whenever any properties
 * of the Lens have changed.  If this is not set for a particular lens, no
 * event will be generated.
 *
 * The event is thrown with one parameter, the lens itself.  This can be used
 * to automatically track changes to camera fov, etc.  in the application.
 */
INLINE void Lens::
set_change_event(const std::string &event) {
  CDWriter cdata(_cycler, true);
  cdata->_change_event = event;
}

/**
 * Returns the name of the event that will be generated whenever any
 * properties of this particular Lens have changed.
 */
INLINE const std::string &Lens::
get_change_event() const {
  CDReader cdata(_cycler);
  return cdata->_change_event;
}

/**
 * Returns the coordinate system that all 3-d computations are performed
 * within for this Lens.  Normally, this is CS_default.
 */
INLINE CoordinateSystem Lens::
get_coordinate_system() const {
  CDReader cdata(_cycler);
  return cdata->_cs;
}

/**
 * Sets the horizontal size of the film without changing its shape.  The
 * aspect ratio remains unchanged; this computes the vertical size of the film
 * to automatically maintain the aspect ratio.
 */
INLINE void Lens::
set_film_size(PN_stdfloat width) {
  CDWriter cdata(_cycler, true);
  do_set_film_size(cdata, width);
}

/**
 * Sets the size and shape of the "film" within the lens.  This both
 * establishes the units used by calls like set_focal_length(), and
 * establishes the aspect ratio of the frame.
 *
 * In a physical camera, the field of view of a lens is determined by the
 * lens' focal length and by the size of the film area exposed by the lens.
 * For instance, a 35mm camera exposes a rectangle on the film about 24mm x
 * 36mm, which means a 50mm lens gives about a 40-degree horizontal field of
 * view.
 *
 * In the virtual camera, you may set the film size to any units here, and
 * specify a focal length in the same units to simulate the same effect.  Or,
 * you may ignore this parameter, and specify the field of view and aspect
 * ratio of the lens directly.
 */
INLINE void Lens::
set_film_size(PN_stdfloat width, PN_stdfloat height) {
  set_film_size(LVecBase2(width, height));
}

/**
 * Sets the size and shape of the "film" within the lens.  This both
 * establishes the units used by calls like set_focal_length(), and
 * establishes the aspect ratio of the frame.
 *
 * In a physical camera, the field of view of a lens is determined by the
 * lens' focal length and by the size of the film area exposed by the lens.
 * For instance, a 35mm camera exposes a rectangle on the film about 24mm x
 * 36mm, which means a 50mm lens gives about a 40-degree horizontal field of
 * view.
 *
 * In the virtual camera, you may set the film size to any units here, and
 * specify a focal length in the same units to simulate the same effect.  Or,
 * you may ignore this parameter, and specify the field of view and aspect
 * ratio of the lens directly.
 */
INLINE void Lens::
set_film_size(const LVecBase2 &film_size) {
  CDWriter cdata(_cycler, true);
  do_set_film_size(cdata, film_size);
}

/**
 * Returns the horizontal and vertical film size of the virtual film.  See
 * set_film_size().
 */
INLINE const LVecBase2 &Lens::
get_film_size() const {
  CDReader cdata(_cycler);
  return do_get_film_size(cdata);
}

/**
 * Sets the horizontal and vertical offset amounts of this Lens.  These are
 * both in the same units specified in set_film_size().
 *
 * This can be used to establish an off-axis lens.
 */
INLINE void Lens::
set_film_offset(PN_stdfloat x, PN_stdfloat y) {
  set_film_offset(LVecBase2(x, y));
}

/**
 * Sets the horizontal and vertical offset amounts of this Lens.  These are
 * both in the same units specified in set_film_size().
 *
 * This can be used to establish an off-axis lens.
 */
INLINE void Lens::
set_film_offset(const LVecBase2 &film_offset) {
  CDWriter cdata(_cycler, true);
  do_set_film_offset(cdata, film_offset);
}

/**
 * Returns the horizontal and vertical offset amounts of this Lens.  See
 * set_film_offset().
 */
INLINE const LVector2 &Lens::
get_film_offset() const {
  CDReader cdata(_cycler);
  return do_get_film_offset(cdata);
}

/**
 * Sets the focal length of the lens.  This may adjust the field-of-view
 * correspondingly, and is an alternate way to specify field of view.
 *
 * For certain kinds of lenses (e.g.  OrthographicLens), the focal length has
 * no meaning.
 */
INLINE void Lens::
set_focal_length(PN_stdfloat focal_length) {
  CDWriter cdata(_cycler, true);
  do_set_focal_length(cdata, focal_length);
}

/**
 * Returns the focal length of the lens.  This may have been set explicitly by
 * a previous call to set_focal_length(), or it may be computed based on the
 * lens' fov and film_size.  For certain kinds of lenses, the focal length has
 * no meaning.
 */
INLINE PN_stdfloat Lens::
get_focal_length() const {
  CDReader cdata(_cycler);
  return do_get_focal_length(cdata);
}

/**
 * Sets the horizontal field of view of the lens without changing the aspect
 * ratio.  The vertical field of view is adjusted to maintain the same aspect
 * ratio.
 */
INLINE void Lens::
set_fov(PN_stdfloat hfov) {
  CDWriter cdata(_cycler, true);
  do_set_fov(cdata, hfov);
}

/**
 * Sets the field of view of the lens in both dimensions.  This establishes
 * both the field of view and the aspect ratio of the lens.  This is one way
 * to specify the field of view of a lens; set_focal_length() is another way.
 *
 * For certain kinds of lenses (like OrthoLens), the field of view has no
 * meaning.
 */
INLINE void Lens::
set_fov(PN_stdfloat hfov, PN_stdfloat vfov) {
  set_fov(LVecBase2(hfov, vfov));
}

/**
 * Sets the field of view of the lens in both dimensions.  This establishes
 * both the field of view and the aspect ratio of the lens.  This is one way
 * to specify the field of view of a lens; set_focal_length() is another way.
 *
 * For certain kinds of lenses (like OrthographicLens), the field of view has
 * no meaning.
 */
INLINE void Lens::
set_fov(const LVecBase2 &fov) {
  CDWriter cdata(_cycler, true);
  do_set_fov(cdata, fov);
}

/**
 * Returns the horizontal and vertical film size of the virtual film.  See
 * set_fov().
 */
INLINE const LVecBase2 &Lens::
get_fov() const {
  CDReader cdata(_cycler);
  return do_get_fov(cdata);
}

/**
 * Returns the horizontal component of fov only.  See get_fov().
 */
INLINE PN_stdfloat Lens::
get_hfov() const {
  return get_fov()[0];
}

/**
 * Returns the vertical component of fov only.  See get_fov().
 */
INLINE PN_stdfloat Lens::
get_vfov() const {
  return get_fov()[1];
}

/**
 * Sets the aspect ratio of the lens.  This is the ratio of the height to the
 * width of the generated image.  Setting this overrides the two-parameter fov
 * or film size setting.
 */
INLINE void Lens::
set_aspect_ratio(PN_stdfloat aspect_ratio) {
  CDWriter cdata(_cycler, true);
  do_set_aspect_ratio(cdata, aspect_ratio);
}

/**
 * Returns the aspect ratio of the Lens.  This is determined based on the
 * indicated film size; see set_film_size().
 */
INLINE PN_stdfloat Lens::
get_aspect_ratio() const {
  CDReader cdata(_cycler);
  return do_get_aspect_ratio(cdata);
}

/**
 * Defines the position of the near plane (or cylinder, sphere, whatever).
 * Points closer to the lens than this may not be rendered.
 */
INLINE void Lens::
set_near(PN_stdfloat near_distance) {
  CDWriter cdata(_cycler, true);
  do_set_near(cdata, near_distance);
}

/**
 * Returns the position of the near plane (or cylinder, sphere, whatever).
 */
INLINE PN_stdfloat Lens::
get_near() const {
  CDReader cdata(_cycler);
  return do_get_near(cdata);
}

/**
 * Defines the position of the far plane (or cylinder, sphere, whatever).
 * Points farther from the lens than this may not be rendered.
 */
INLINE void Lens::
set_far(PN_stdfloat far_distance) {
  CDWriter cdata(_cycler, true);
  do_set_far(cdata, far_distance);
}

/**
 * Returns the position of the far plane (or cylinder, sphere, whatever).
 */
INLINE PN_stdfloat Lens::
get_far() const {
  CDReader cdata(_cycler);
  return do_get_far(cdata);
}

/**
 * Simultaneously changes the near and far planes.
 */
INLINE void Lens::
set_near_far(PN_stdfloat near_distance, PN_stdfloat far_distance) {
  CDWriter cdata(_cycler, true);
  do_set_near_far(cdata, near_distance, far_distance);
}

/**
 * Sets the direction in which the lens is facing.  Normally, this is down the
 * forward axis (usually the Y axis), but it may be rotated.  This is only one
 * way of specifying the rotation; you may also specify an explicit vector in
 * which to look, or you may give a complete transformation matrix.
 */
INLINE void Lens::
set_view_hpr(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r) {
  set_view_hpr(LVecBase3(h, p, r));
}

/**
 * Specifies the direction in which the lens is facing by giving an axis to
 * look along, and a perpendicular (or at least non-parallel) up axis.
 *
 * See also set_view_hpr().
 */
INLINE void Lens::
set_view_vector(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z, PN_stdfloat i, PN_stdfloat j, PN_stdfloat k) {
  set_view_vector(LVector3(x, y, z), LVector3(i, j, k));
}

/**
 * Sets the distance between the left and right eyes of a stereo camera.  This
 * distance is used to apply a stereo effect when the lens is rendered on a
 * stereo display region.  It only has an effect on a PerspectiveLens.
 *
 * The left eye and the right eye are each offset along the X axis by half of
 * this distance, so that this parameter specifies the total distance between
 * them.
 *
 * Also see set_convergence_distance(), which relates.
 */
INLINE void Lens::
set_interocular_distance(PN_stdfloat interocular_distance) {
  CDWriter cdata(_cycler, true);
  do_set_interocular_distance(cdata, interocular_distance);
  do_throw_change_event(cdata);
}

/**
 * See set_interocular_distance().
 */
INLINE PN_stdfloat Lens::
get_interocular_distance() const {
  CDReader cdata(_cycler);
  return cdata->_interocular_distance;
}

/**
 * Sets the distance between between the camera plane and the point in the
 * distance that the left and right eyes are both looking at.  This distance
 * is used to apply a stereo effect when the lens is rendered on a stereo
 * display region.  It only has an effect on a PerspectiveLens.
 *
 * This parameter must be greater than 0, but may be as large as you like.  It
 * controls the distance at which the two stereo images will appear to
 * converge, which is a normal property of stereo vision.  Normally this
 * should be set to the distance from the camera to the area of interest in
 * your scene.  Anything beyond this distance will appear to go into the
 * screen, and anything closer will appear to come out of the screen.  If you
 * want to simulate parallel stereo, set this to infinity.
 *
 * Note that this creates an off-axis frustum, which means that the lenses are
 * still pointing in the same direction, which is usually more desirable than
 * the more naive toe-in approach, where the two lenses are simply tilted
 * toward each other.
 *
 * Prior to Panda3D 1.9.0, the convergence was being calculated incorrectly.
 * It has since been corrected.  To restore the legacy behavior you can set
 * the stereo-lens-old-convergence variable to true.
 *
 * Also see set_interocular_distance(), which relates.
 */
INLINE void Lens::
set_convergence_distance(PN_stdfloat convergence_distance) {
  CDWriter cdata(_cycler, true);
  do_set_convergence_distance(cdata, convergence_distance);
  do_throw_change_event(cdata);
}

/**
 * See set_convergence_distance().
 */
INLINE PN_stdfloat Lens::
get_convergence_distance() const {
  CDReader cdata(_cycler);
  return cdata->_convergence_distance;
}

/**
 * Sets an arbitrary transformation on the lens.  This replaces the individual
 * transformation components like set_view_hpr().
 *
 * Setting a transformation here will have a slightly different effect than
 * putting one on the LensNode that contains this lens.  In particular,
 * lighting and other effects computations will still be performed on the lens
 * in its untransformed (facing forward) position, but the actual projection
 * matrix will be transformed by this matrix.
 */
INLINE void Lens::
set_view_mat(const LMatrix4 &view_mat) {
  CDWriter cdata(_cycler, true);
  do_set_view_mat(cdata, view_mat);
}

/**
 * Returns the direction in which the lens is facing.
 */
INLINE const LMatrix4 &Lens::
get_view_mat() const {
  CDReader cdata(_cycler);
  return do_get_view_mat(cdata);
}

/**
 * Returns the keystone correction specified for the lens.
 */
INLINE const LVecBase2 &Lens::
get_keystone() const {
  CDReader cdata(_cycler);
  return cdata->_keystone;
}

/**
 * Returns the custom_film_mat specified for the lens.
 */
INLINE const LMatrix4 &Lens::
get_custom_film_mat() const {
  CDReader cdata(_cycler);
  return cdata->_custom_film_mat;
}

/**
 * Returns the complete transformation matrix from a 3-d point in space to a
 * point on the film, if such a matrix exists, or the identity matrix if the
 * lens is nonlinear.
 */
INLINE const LMatrix4 &Lens::
get_projection_mat(StereoChannel channel) const {
  CDReader cdata(_cycler);
  return do_get_projection_mat(cdata, channel);
}

/**
 * Returns the matrix that transforms from a 2-d point on the film to a 3-d
 * vector in space, if such a matrix exists.
 */
INLINE const LMatrix4 &Lens::
get_projection_mat_inv(StereoChannel stereo_channel) const {
  CDReader cdata(_cycler);
  return do_get_projection_mat_inv(cdata, stereo_channel);
}

/**
 * Returns the matrix that transforms from a point behind the lens to a point
 * on the film.
 */
INLINE const LMatrix4 &Lens::
get_film_mat() const {
  CDReader cdata(_cycler);
  return do_get_film_mat(cdata);
}

/**
 * Returns the matrix that transforms from a point on the film to a point
 * behind the lens.
 */
INLINE const LMatrix4 &Lens::
get_film_mat_inv() const {
  CDReader cdata(_cycler);
  return do_get_film_mat_inv(cdata);
}

/**
 * Returns the matrix that transforms from a point in front of the lens to a
 * point in space.
 */
INLINE const LMatrix4 &Lens::
get_lens_mat() const {
  CDReader cdata(_cycler);
  return do_get_lens_mat(cdata);
}

/**
 * Returns the matrix that transforms from a point in space to a point in
 * front of the lens.
 */
INLINE const LMatrix4 &Lens::
get_lens_mat_inv() const {
  CDReader cdata(_cycler);
  return do_get_lens_mat_inv(cdata);
}

/**
 * Returns the UpdateSeq that is incremented whenever the lens properties are
 * changed.  As long as this number remains the same, you may assume the lens
 * properties are unchanged.
 */
INLINE UpdateSeq Lens::
get_last_change() const {
  CDReader cdata(_cycler);
  return cdata->_last_change;
}

/**
 * Clears from _user_flags the bits in the first parameter, and sets the bits
 * in the second parameter.
 */
INLINE void Lens::
do_adjust_user_flags(CData *cdata, int clear_flags, int set_flags) {
  cdata->_user_flags = (cdata->_user_flags & ~clear_flags) | (short)set_flags;
}

/**
 * Clears from _comp_flags the bits in the first parameter, and sets the bits
 * in the second parameter.
 */
INLINE void Lens::
do_adjust_comp_flags(CData *cdata, int clear_flags, int set_flags) {
  cdata->_comp_flags = (cdata->_comp_flags & ~clear_flags) | (short)set_flags;
}

/**
 *
 */
INLINE void Lens::
do_set_film_offset(CData *cdata, const LVecBase2 &film_offset) {
  cdata->_film_offset = film_offset;
  do_adjust_comp_flags(cdata, CF_mat, 0);
  do_throw_change_event(cdata);
}

/**
 *
 */
INLINE const LVector2 &Lens::
do_get_film_offset(const CData *cdata) const {
  return cdata->_film_offset;
}

/**
 *
 */
INLINE void Lens::
do_set_near(CData *cdata, PN_stdfloat near_distance) {
  if (near_distance != cdata->_near_distance) {
    cdata->_near_distance = near_distance;
    do_adjust_comp_flags(cdata, CF_projection_mat | CF_projection_mat_inv, 0);
    do_throw_change_event(cdata);
  }
}

/**
 *
 */
INLINE PN_stdfloat Lens::
do_get_near(const CData *cdata) const {
  return cdata->_near_distance;
}

/**
 *
 */
INLINE void Lens::
do_set_far(CData *cdata, PN_stdfloat far_distance) {
  if (far_distance != cdata->_far_distance) {
    cdata->_far_distance = far_distance;
    do_adjust_comp_flags(cdata, CF_projection_mat | CF_projection_mat_inv, 0);
    do_throw_change_event(cdata);
  }
}

/**
 *
 */
INLINE PN_stdfloat Lens::
do_get_far(const CData *cdata) const {
  return cdata->_far_distance;
}

/**
 *
 */
INLINE void Lens::
do_set_near_far(CData *cdata, PN_stdfloat near_distance, PN_stdfloat far_distance) {
  if (near_distance != cdata->_near_distance || far_distance != cdata->_far_distance) {
    cdata->_near_distance = near_distance;
    cdata->_far_distance = far_distance;
    do_adjust_comp_flags(cdata, CF_projection_mat | CF_projection_mat_inv, 0);
    do_throw_change_event(cdata);
  }
}

INLINE std::ostream &
operator << (std::ostream &out, const Lens &lens) {
  lens.output(out);
  return out;
}
