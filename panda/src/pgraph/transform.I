/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file transform.I
 * @author rdb
 * @date 2024-10-17
 */

/**
 * Converts to a TransformState implicitly.
 */
/*INLINE CPT(TransformState) Transform::
operator -> () const {
  return TransformState::make_mat(get_mat());
}*/

/**
 * Return true if these are the same.
 */
INLINE bool Transform::
operator == (const Transform &other) const {
  return memcmp(_columns, other._columns, sizeof(Column) * 3) == 0;
}

/**
 * Opposite of operator ==.
 */
INLINE bool Transform::
operator != (const Transform &other) const {
  return memcmp(_columns, other._columns, sizeof(Column) * 3) != 0;
}

/**
 *
 */
INLINE int Transform::
compare_to(const Transform &other) const {
  return memcmp(_columns, other._columns, sizeof(Column) * 3);
}

/**
 * Constructs an identity transform.
 */
INLINE Transform Transform::
make_identity() {
  Transform transform;
  transform._columns[0] = Column(1, 0, 0, 0);
  transform._columns[1] = Column(0, 1, 0, 0);
  transform._columns[2] = Column(0, 0, 1, 0);
  return transform;
}

/**
 * Makes a new Transform with the specified transformation matrix.  It is an
 * error if the given matrix is not affine or not invertible.
 */
INLINE Transform Transform::
make_mat(const LMatrix4 &mat) {
  Transform transform;
  transform._columns[0] = mat.get_col(0);
  transform._columns[1] = mat.get_col(1);
  transform._columns[2] = mat.get_col(2);

#ifndef NDEBUG
  // Must be an affine matrix.
  nassertr(mat.get_col(3).almost_equal(LVecBase4(0, 0, 0, 1)), transform);

  // Must be invertible.
  PN_stdfloat d0 = transform._columns[1][1] * transform._columns[2][2] - transform._columns[1][2] * transform._columns[2][1];
  PN_stdfloat d1 = transform._columns[0][2] * transform._columns[2][1] - transform._columns[0][1] * transform._columns[2][2];
  PN_stdfloat d2 = transform._columns[0][1] * transform._columns[1][2] - transform._columns[0][2] * transform._columns[1][1];
  PN_stdfloat det = transform._columns[0][0] * d0 + transform._columns[1][0] * d1 + transform._columns[2][0] * d2;
  nassertr(!IS_THRESHOLD_ZERO(det, (NEARLY_ZERO(PN_stdfloat) * NEARLY_ZERO(PN_stdfloat))), transform);
#endif

  return transform;
}

/**
 * Makes a new Transform with the specified position.
 */
INLINE Transform Transform::
make_pos(const LVecBase3 &pos) {
  Transform transform;
  transform._columns[0].set(0, 0, 0, pos[0]);
  transform._columns[1].set(0, 1, 0, pos[1]);
  transform._columns[2].set(0, 0, 2, pos[2]);
  return transform;
}

/**
 * Makes a new Transform with the specified rotation.
 */
INLINE Transform Transform::
make_hpr(const LVecBase3 &hpr) {
  return make_pos_hpr_scale(LPoint3(0, 0, 0), hpr, LVecBase3(1, 1, 1));
}

/**
 * Makes a new Transform with the specified quaternion.
 */
INLINE Transform Transform::
make_quat(const LQuaternion &quat) {
  return make_pos_quat(LPoint3(0), quat);
}

/**
 * Makes a new Transform with the specified scale.
 */
INLINE Transform Transform::
make_scale(const LVecBase3 &scale) {
  Transform transform;
  transform._columns[0] = Column(scale[0], 0, 0, 0);
  transform._columns[1] = Column(0, scale[1], 0, 0);
  transform._columns[2] = Column(0, 0, scale[2], 0);
  return transform;
}

/**
 * Makes a new Transform with the specified shear.
 */
INLINE Transform Transform::
make_shear(const LVecBase3 &shear) {
  Transform transform;
  transform.set_mat3(LMatrix3::shear_mat(shear));
  transform.set_pos(LVecBase3(0));
  return transform;
}

/**
 * Makes a new Transform with the specified components.
 */
INLINE Transform Transform::
make_pos_quat(const LVecBase3 &pos, const LQuaternion &quat) {
  PN_stdfloat length_sq = quat.dot(quat);
  PN_stdfloat s = (length_sq == 0.0f) ? 0.0f : (2.0f / length_sq);
  PN_stdfloat xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz;
  xs = quat._v(1) * s;   ys = quat._v(2) * s;   zs = quat._v(3) * s;
  wx = quat._v(0) * xs;  wy = quat._v(0) * ys;  wz = quat._v(0) * zs;
  xx = quat._v(1) * xs;  xy = quat._v(1) * ys;  xz = quat._v(1) * zs;
  yy = quat._v(2) * ys;  yz = quat._v(2) * zs;  zz = quat._v(3) * zs;

  Transform result;
  result._columns[0][0] = 1.0f - (yy + zz);
  result._columns[0][1] = xy - wz;
  result._columns[0][2] = xz + wy;
  result._columns[0][3] = pos[0];
  result._columns[1][0] = xy + wz;
  result._columns[1][1] = 1.0f - (xx + zz);
  result._columns[1][2] = yz - wx;
  result._columns[1][3] = pos[1];
  result._columns[2][0] = xz - wy;
  result._columns[2][1] = yz + wx;
  result._columns[2][2] = 1.0f - (xx + yy);
  result._columns[2][3] = pos[2];
  return result;
}

/**
 * Makes a new Transform with the specified components.
 */
INLINE Transform Transform::
make_pos_hpr(const LVecBase3 &pos, const LVecBase3 &hpr) {
  return make_pos_hpr_scale_shear(pos, hpr, LVecBase3(0), LVecBase3(0));
}

/**
 * Makes a new Transform with the specified components.
 */
INLINE Transform Transform::
make_pos_hpr_scale(const LVecBase3 &pos, const LVecBase3 &hpr,
                   LVecBase3 scale) {
  return make_pos_hpr_scale_shear(pos, hpr, scale, LVecBase3(0));
}

/**
 * Returns true if the transform represents the identity matrix, false
 * otherwise.
 */
INLINE bool Transform::
is_identity() const {
  return _columns[0] == Column(1, 0, 0, 0)
      && _columns[1] == Column(0, 1, 0, 0)
      && _columns[2] == Column(0, 0, 1, 0);
}

/**
 * Returns true if the transform represents an invalid matrix, for instance
 * the result of inverting a singular matrix, or false if the transform is
 * valid.
 */
INLINE bool Transform::
is_invalid() const {
  return false;
}
#if 0
/**
 * Returns true if the transform represents a singular transform (that is, it
 * has a zero scale, and it cannot be inverted), or false otherwise.
 */
INLINE bool Transform::
is_singular() const {
  check_singular();
  return ((_flags & F_is_singular) != 0);
}

/**
 * Returns true if the transform has been constructed entirely using the 2-d
 * transform operations, e.g.  make_pos2d(), and therefore operates strictly
 * in two-dimensional space on X and Y only.
 */
INLINE bool Transform::
is_2d() const {
  return ((_flags & F_is_2d) != 0);
}

/**
 * Returns true if the transform can be described by separate pos, hpr, and
 * scale components.  Most transforms we use in everyday life can be so
 * described, but some kinds of transforms (for instance, those involving a
 * skew) cannot.
 *
 * This is not related to whether the transform was originally described
 * componentwise.  Even a transform that was constructed with a 4x4 may return
 * true here if the matrix is a simple affine matrix with no skew.
 *
 * If this returns true, you may safely call get_hpr() and get_scale() to
 * retrieve the components.  (You may always safely call get_pos() whether
 * this returns true or false.)
 */
INLINE bool Transform::
has_components() const {
  check_components();
  return ((_flags & F_has_components) != 0);
}

/**
 * Returns true if the transform was specified componentwise, or false if it
 * was specified with a general 4x4 matrix.  If this is true, the components
 * returned by get_pos() and get_scale() will be exactly those that were set;
 * otherwise, these functions will return computed values.  If this is true,
 * the rotation may have been set either with a hpr trio or with a quaternion;
 * hpr_given() or quat_given() can resolve the difference.
 */
INLINE bool Transform::
components_given() const {
  return ((_flags & F_components_given) != 0);
}

/**
 * Returns true if the rotation was specified via a trio of Euler angles,
 * false otherwise.  If this is true, get_hpr() will be exactly as set;
 * otherwise, it will return a computed value.
 */
INLINE bool Transform::
hpr_given() const {
  return ((_flags & F_hpr_given) != 0);
}

/**
 * Returns true if the rotation was specified via a quaternion, false
 * otherwise.  If this is true, get_quat() will be exactly as set; otherwise,
 * it will return a computed value.
 */
INLINE bool Transform::
quat_given() const {
  return ((_flags & F_quat_given) != 0);
}

/**
 * Returns true if the transform's pos component can be extracted out
 * separately.  This is generally always true, unless the transform is invalid
 * (i.e.  is_invalid() returns true).
 */
INLINE bool Transform::
has_pos() const {
  return !is_invalid();
}

/**
 * Returns true if the transform's rotation component can be extracted out
 * separately and described as a set of Euler angles.  This is generally true
 * only when has_components() is true.
 */
INLINE bool Transform::
has_hpr() const {
  return has_components();
}

/**
 * Returns true if the transform's rotation component can be extracted out
 * separately and described as a quaternion.  This is generally true only when
 * has_components() is true.
 */
INLINE bool Transform::
has_quat() const {
  return has_components();
}

/**
 * Returns true if the transform's scale component can be extracted out
 * separately.  This is generally true only when has_components() is true.
 */
INLINE bool Transform::
has_scale() const {
  return has_components();
}

/**
 * Returns true if the scale is uniform 1.0, or false if the scale has some
 * real value.
 */
INLINE bool Transform::
has_identity_scale() const {
  check_components();
  return (_flags & F_identity_scale) != 0;
}

/**
 * Returns true if the scale is uniform across all three axes (and therefore
 * can be expressed as a single number), or false if the transform has a
 * different scale in different dimensions.
 */
INLINE bool Transform::
has_uniform_scale() const {
  check_components();
  return (_flags & F_uniform_scale) != 0;
}

/**
 * Returns true if the transform's shear component can be extracted out
 * separately.  This is generally true only when has_components() is true.
 */
INLINE bool Transform::
has_shear() const {
  return has_components();
}

/**
 * Returns true if the shear component is non-zero, false if it is zero or if
 * the matrix cannot be decomposed.
 */
INLINE bool Transform::
has_nonzero_shear() const {
  check_components();
  return (_flags & F_has_nonzero_shear) != 0;
}

/**
 * Returns true if the transform can be described as a matrix.  This is
 * generally always true, unless is_invalid() is true.
 */
INLINE bool Transform::
has_mat() const {
  return !is_invalid();
}
#endif

/**
 * Returns the translation component of the transform.
 */
INLINE LPoint3 Transform::
get_pos() const {
  return LPoint3(_columns[0][3], _columns[1][3], _columns[2][3]);
}

/**
 * Sets the translation component of the transform.
 */
INLINE void Transform::
set_pos(const LPoint3 &pos) {
  _columns[0][3] = pos[0];
  _columns[1][3] = pos[1];
  _columns[2][3] = pos[2];
}

/**
 * Returns the rotation component of the transform as a quaternion.
 */
INLINE LQuaternion Transform::
get_quat() const {
  LQuaternion result;
  result.set_from_matrix(get_mat3());
  return result;
}

/**
 * Returns the matrix that describes the transform.  The last column will
 * always be (0, 0, 0, 1).
 */
INLINE LMatrix4 Transform::
get_mat() const {
  return LMatrix4(_columns[0][0], _columns[1][0], _columns[2][0], 0,
                  _columns[0][1], _columns[1][1], _columns[2][1], 0,
                  _columns[0][2], _columns[1][2], _columns[2][2], 0,
                  _columns[0][3], _columns[1][3], _columns[2][3], 1);
}

/**
 * Returns the 3x3 matrix that excludes the translation component.
 */
INLINE LMatrix3 Transform::
get_mat3() const {
  return LMatrix3(_columns[0][0], _columns[1][0], _columns[2][0],
                  _columns[0][1], _columns[1][1], _columns[2][1],
                  _columns[0][2], _columns[1][2], _columns[2][2]);
}

/**
 * Replaces the rotation/scale/shear matrix without touching the rest of the
 * transformation.
 */
INLINE void Transform::
set_mat3(const LMatrix3 &mat) {
  _columns[0][0] = mat[0][0];
  _columns[0][1] = mat[1][0];
  _columns[0][2] = mat[2][0];
  _columns[1][0] = mat[0][1];
  _columns[1][1] = mat[1][1];
  _columns[1][2] = mat[2][1];
  _columns[2][0] = mat[0][2];
  _columns[2][1] = mat[1][2];
  _columns[2][2] = mat[2][2];
}

/**
 *
 */
INLINE Transform Transform::
compose(const Transform &other) const {
  Transform result;
  result._columns[0][0] = other._columns[0][0] * _columns[0][0] + other._columns[1][0] * _columns[0][1] + other._columns[2][0] * _columns[0][2];
  result._columns[0][1] = other._columns[0][1] * _columns[0][0] + other._columns[1][1] * _columns[0][1] + other._columns[2][1] * _columns[0][2];
  result._columns[0][2] = other._columns[0][2] * _columns[0][0] + other._columns[1][2] * _columns[0][1] + other._columns[2][2] * _columns[0][2];
  result._columns[0][3] = other._columns[0][3] * _columns[0][0] + other._columns[1][3] * _columns[0][1] + other._columns[2][3] * _columns[0][2] + _columns[0][3];
  result._columns[1][0] = other._columns[0][0] * _columns[1][0] + other._columns[1][0] * _columns[1][1] + other._columns[2][0] * _columns[1][2];
  result._columns[1][1] = other._columns[0][1] * _columns[1][0] + other._columns[1][1] * _columns[1][1] + other._columns[2][1] * _columns[1][2];
  result._columns[1][2] = other._columns[0][2] * _columns[1][0] + other._columns[1][2] * _columns[1][1] + other._columns[2][2] * _columns[1][2];
  result._columns[1][3] = other._columns[0][3] * _columns[1][0] + other._columns[1][3] * _columns[1][1] + other._columns[2][3] * _columns[1][2] + _columns[1][3];
  result._columns[2][0] = other._columns[0][0] * _columns[2][0] + other._columns[1][0] * _columns[2][1] + other._columns[2][0] * _columns[2][2];
  result._columns[2][1] = other._columns[0][1] * _columns[2][0] + other._columns[1][1] * _columns[2][1] + other._columns[2][1] * _columns[2][2];
  result._columns[2][2] = other._columns[0][2] * _columns[2][0] + other._columns[1][2] * _columns[2][1] + other._columns[2][2] * _columns[2][2];
  result._columns[2][3] = other._columns[0][3] * _columns[2][0] + other._columns[1][3] * _columns[2][1] + other._columns[2][3] * _columns[2][2] + _columns[2][3];
  return result;
}

/**
 *
 */
INLINE Transform Transform::
invert_compose(const Transform &other) const {
  return get_inverse().compose(other);
}

/**
 *
 */
INLINE Transform Transform::
get_inverse() const {
  PN_stdfloat d0 = _columns[1][1] * _columns[2][2] - _columns[1][2] * _columns[2][1];
  PN_stdfloat d1 = _columns[0][2] * _columns[2][1] - _columns[0][1] * _columns[2][2];
  PN_stdfloat d2 = _columns[0][1] * _columns[1][2] - _columns[0][2] * _columns[1][1];
  PN_stdfloat det = _columns[0][0] * d0 + _columns[1][0] * d1 + _columns[2][0] * d2;
  PN_stdfloat inv_det = (PN_stdfloat)1 / det;

  Transform result;
  result._columns[0][0] =  inv_det * d0;
  result._columns[0][1] =  inv_det * d1;
  result._columns[0][2] =  inv_det * d2;
  result._columns[0][3] = -(_columns[0][3] * result._columns[0][0] +
                            _columns[1][3] * result._columns[0][1] +
                            _columns[2][3] * result._columns[0][2]);

  result._columns[1][0] = -inv_det * (_columns[1][0] * _columns[2][2] - _columns[1][2] * _columns[2][0]);
  result._columns[1][1] =  inv_det * (_columns[0][0] * _columns[2][2] - _columns[0][2] * _columns[2][0]);
  result._columns[1][2] = -inv_det * (_columns[0][0] * _columns[1][2] - _columns[0][2] * _columns[1][0]);
  result._columns[1][3] = -(_columns[0][3] * result._columns[1][0] +
                            _columns[1][3] * result._columns[1][1] +
                            _columns[2][3] * result._columns[1][2]);

  result._columns[2][0] =  inv_det * (_columns[1][0] * _columns[2][1] - _columns[1][1] * _columns[2][0]);
  result._columns[2][1] = -inv_det * (_columns[0][0] * _columns[2][1] - _columns[0][1] * _columns[2][0]);
  result._columns[2][2] =  inv_det * (_columns[0][0] * _columns[1][1] - _columns[0][1] * _columns[1][0]);
  result._columns[2][3] = -(_columns[0][3] * result._columns[2][0] +
                            _columns[1][3] * result._columns[2][1] +
                            _columns[2][3] * result._columns[2][2]);
  return result;
}

/**
 *
 */
INLINE LPoint3 Transform::
xform_point(const LPoint3 &point) const {
  return LPoint3(
    point[0] * _columns[0][0] + point[1] * _columns[0][1] + point[2] * _columns[0][2] + _columns[0][3],
    point[0] * _columns[1][0] + point[1] * _columns[1][1] + point[2] * _columns[1][2] + _columns[1][3],
    point[0] * _columns[2][0] + point[1] * _columns[2][1] + point[2] * _columns[2][2] + _columns[2][3]);
}

INLINE LVector3 Transform::
xform_vec(const LVector3 &vec) const {
  return LVector3(
    vec[0] * _columns[0][0] + vec[1] * _columns[0][1] + vec[2] * _columns[0][2],
    vec[0] * _columns[1][0] + vec[1] * _columns[1][1] + vec[2] * _columns[1][2],
    vec[0] * _columns[2][0] + vec[1] * _columns[2][1] + vec[2] * _columns[2][2]);
}
