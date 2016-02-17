/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file matrixLens.I
 * @author drose
 * @date 2001-12-12
 */

/**
 *
 */
INLINE MatrixLens::
MatrixLens() :
  _user_mat(LMatrix4::ident_mat()),
  _ml_flags(0)
{
  // The default film size for a MatrixLens is 2, which makes the default
  // range for both X and Y be [-1, 1].  This also, incidentally, makes the
  // film_mat be identity.
  set_film_size(2.0);
}

/**
 *
 */
INLINE MatrixLens::
MatrixLens(const MatrixLens &copy) :
  Lens(copy),
  _user_mat(copy._user_mat),
  _left_eye_mat(copy._left_eye_mat),
  _right_eye_mat(copy._right_eye_mat),
  _ml_flags(copy._ml_flags)
{
}

/**
 *
 */
INLINE void MatrixLens::
operator = (const MatrixLens &copy) {
  Lens::operator = (copy);
  _user_mat = copy._user_mat;
  _left_eye_mat = copy._left_eye_mat;
  _right_eye_mat = copy._right_eye_mat;
  _ml_flags = copy._ml_flags;
}

/**
 * Explicitly specifies the projection matrix.  This matrix should convert X
 * and Y to the range [-film_size/2, film_size/2], where (-fs/2,-fs/2) is the
 * lower left corner of the screen and (fs/2, fs/2) is the upper right.  Z
 * should go to the range [-1, 1], where -1 is the far plane and 1 is the near
 * plane.  Note that this is a left-handed Y-up coordinate system.
 *
 * The default film_size for a MatrixLens is 2, so the default range is [-1,
 * 1] for both X and Y.  This is consistent with the GL conventions for
 * projection matrices.
 */
INLINE void MatrixLens::
set_user_mat(const LMatrix4 &user_mat) {
  Lens::CDWriter lens_cdata(Lens::_cycler, true);
  _user_mat = user_mat;
  do_adjust_comp_flags(lens_cdata, CF_mat, 0);
}

/**
 * Returns the explicit projection matrix as set by the user.  This does not
 * include transforms on the lens or film (e.g.  a film offset or view hpr).
 */
INLINE const LMatrix4 &MatrixLens::
get_user_mat() const {
  return _user_mat;
}

/**
 * Sets a custom projection matrix for the left eye.  This is only used if the
 * lens is attached to a stereo camera, in which case the left eye matrix will
 * be used to draw the scene in the left eye (but the center matrix--the
 * user_mat--will still be used to cull the scene).
 *
 * This matrix should not be too different from the center matrix (set by
 * set_user_mat()) or culling errors may become obvious.
 */
INLINE void MatrixLens::
set_left_eye_mat(const LMatrix4 &left_eye_mat) {
  Lens::CDWriter lens_cdata(Lens::_cycler, true);
  _left_eye_mat = left_eye_mat;
  _ml_flags |= MF_has_left_eye;
  do_adjust_comp_flags(lens_cdata, CF_mat, 0);
}

/**
 * Removes the custom projection matrix set for the left eye, and uses the
 * center matrix (set by set_user_mat) instead.
 */
INLINE void MatrixLens::
clear_left_eye_mat() {
  Lens::CDWriter lens_cdata(Lens::_cycler, true);
  _ml_flags &= ~MF_has_left_eye;
  do_adjust_comp_flags(lens_cdata, CF_mat, 0);
}

/**
 * Returns true if the camera has a custom projection matrix set for the left
 * eye, or false if the center matrix (set by set_user_mat) will be used for
 * the left eye.
 */
INLINE bool MatrixLens::
has_left_eye_mat() const {
  return (_ml_flags & MF_has_left_eye) != 0;
}

/**
 * Returns the custom projection matrix for the left eye, if any, or the
 * center matrix if there is no custom matrix set for the left eye.
 */
INLINE const LMatrix4 &MatrixLens::
get_left_eye_mat() const {
  if ((_ml_flags & MF_has_left_eye) != 0) {
    return _left_eye_mat;
  }
  return _user_mat;
}

/**
 * Sets a custom projection matrix for the right eye.  This is only used if
 * the lens is attached to a stereo camera, in which case the right eye matrix
 * will be used to draw the scene in the right eye (but the center matrix--the
 * user_mat--will still be used to cull the scene).
 *
 * This matrix should not be too different from the center matrix (set by
 * set_user_mat()) or culling errors may become obvious.
 */
INLINE void MatrixLens::
set_right_eye_mat(const LMatrix4 &right_eye_mat) {
  Lens::CDWriter lens_cdata(Lens::_cycler, true);
  _right_eye_mat = right_eye_mat;
  _ml_flags |= MF_has_right_eye;
  do_adjust_comp_flags(lens_cdata, CF_mat, 0);
}

/**
 * Removes the custom projection matrix set for the right eye, and uses the
 * center matrix (set by set_user_mat) instead.
 */
INLINE void MatrixLens::
clear_right_eye_mat() {
  Lens::CDWriter lens_cdata(Lens::_cycler, true);
  _ml_flags &= ~MF_has_right_eye;
  do_adjust_comp_flags(lens_cdata, CF_mat, 0);
}

/**
 * Returns true if the camera has a custom projection matrix set for the right
 * eye, or false if the center matrix (set by set_user_mat) will be used for
 * the right eye.
 */
INLINE bool MatrixLens::
has_right_eye_mat() const {
  return (_ml_flags & MF_has_right_eye) != 0;
}

/**
 * Returns the custom projection matrix for the right eye, if any, or the
 * center matrix if there is no custom matrix set for the right eye.
 */
INLINE const LMatrix4 &MatrixLens::
get_right_eye_mat() const {
  if ((_ml_flags & MF_has_right_eye) != 0) {
    return _right_eye_mat;
  }
  return _user_mat;
}
