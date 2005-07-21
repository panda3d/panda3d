// Filename: transformState.I
// Created by:  drose (25Feb02)
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
//     Function: TransformState::make_pos
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_pos(const LVecBase3f &pos) {
  return make_pos_hpr_scale(pos, 
                            LVecBase3f(0.0f, 0.0f, 0.0f),
                            LVecBase3f(1.0f, 1.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_hpr
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_hpr(const LVecBase3f &hpr) {
  return make_pos_hpr_scale(LVecBase3f(0.0f, 0.0f, 0.0f),
                            hpr, 
                            LVecBase3f(1.0f, 1.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_quat
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_quat(const LQuaternionf &quat) {
  return make_pos_quat_scale(LVecBase3f(0.0f, 0.0f, 0.0f),
                             quat, 
                             LVecBase3f(1.0f, 1.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_pos_hpr
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_pos_hpr(const LVecBase3f &pos, const LVecBase3f &hpr) {
  return make_pos_hpr_scale(pos, hpr,
                            LVecBase3f(1.0, 1.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_scale
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_scale(float scale) {
  // We actually map this 3-d uniform make_scale() to the 2-d
  // version--might as well call it a 2-d scale.
  return make_scale2d(scale);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_scale
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_scale(const LVecBase3f &scale) {
  return make_pos_hpr_scale(LVecBase3f(0.0f, 0.0f, 0.0f),
                            LVecBase3f(0.0f, 0.0f, 0.0f),
                            scale);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_shear
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_shear(const LVecBase3f &shear) {
  return make_pos_hpr_scale_shear(LVecBase3f(0.0f, 0.0f, 0.0f),
                                  LVecBase3f(0.0f, 0.0f, 0.0f),
                                  LVecBase3f(1.0f, 1.0f, 1.0f),
                                  shear);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_pos_hpr_scale
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_pos_hpr_scale(const LVecBase3f &pos, const LVecBase3f &hpr, 
                   const LVecBase3f &scale) {
  return make_pos_hpr_scale_shear(pos, hpr, scale, LVecBase3f::zero());
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_pos_quat_scale
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_pos_quat_scale(const LVecBase3f &pos, const LQuaternionf &quat, 
                    const LVecBase3f &scale) {
  return make_pos_quat_scale_shear(pos, quat, scale, LVecBase3f::zero());
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_pos2d
//       Access: Published, Static
//  Description: Makes a new 2-d TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_pos2d(const LVecBase2f &pos) {
  return make_pos_rotate_scale2d(pos, 0.0f, LVecBase2f(1.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_rotate2d
//       Access: Published, Static
//  Description: Makes a new 2-d TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_rotate2d(float rotate) {
  return make_pos_rotate_scale2d(LVecBase2f(0.0f, 0.0f), rotate, 
                                 LVecBase2f(1.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_pos_rotate2d
//       Access: Published, Static
//  Description: Makes a new 2-d TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_pos_rotate2d(const LVecBase2f &pos, float rotate) {
  return make_pos_rotate_scale2d(pos, rotate,
                                 LVecBase2f(1.0, 1.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_scale2d
//       Access: Published, Static
//  Description: Makes a new 2-d TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_scale2d(float scale) {
  return make_pos_rotate_scale2d(LVecBase2f(0.0f, 0.0f), 0.0f,
                                 LVecBase2f(scale, scale));
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_scale2d
//       Access: Published, Static
//  Description: Makes a new 2-d TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_scale2d(const LVecBase2f &scale) {
  return make_pos_rotate_scale2d(LVecBase2f(0.0f, 0.0f), 0.0f, scale);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_shear2d
//       Access: Published, Static
//  Description: Makes a new 2-d TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_shear2d(float shear) {
  return make_pos_rotate_scale_shear2d(LVecBase2f(0.0f, 0.0f), 0.0f, 
                                       LVecBase2f(1.0f, 1.0f), shear);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_pos_rotate_scale2d
//       Access: Published, Static
//  Description: Makes a new 2-d TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) TransformState::
make_pos_rotate_scale2d(const LVecBase2f &pos, float rotate, 
                        const LVecBase2f &scale) {
  return make_pos_rotate_scale_shear2d(pos, rotate, scale, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::is_identity
//       Access: Published
//  Description: Returns true if the transform represents the identity
//               matrix, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
is_identity() const {
  return ((_flags & F_is_identity) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::is_invalid
//       Access: Published
//  Description: Returns true if the transform represents an invalid
//               matrix, for instance the result of inverting a
//               singular matrix, or false if the transform is valid.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
is_invalid() const {
  return ((_flags & F_is_invalid) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::is_singular
//       Access: Published
//  Description: Returns true if the transform represents a singular
//               transform (that is, it has a zero scale, and it
//               cannot be inverted), or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
is_singular() const {
  check_singular();
  return ((_flags & F_is_singular) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::is_2d
//       Access: Published
//  Description: Returns true if the transform has been constructed
//               entirely using the 2-d transform operations,
//               e.g. make_pos2d(), and therefore operates strictly in
//               two-dimensional space on X and Y only.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
is_2d() const {
  return ((_flags & F_is_2d) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::has_components
//       Access: Published
//  Description: Returns true if the transform can be described by
//               separate pos, hpr, and scale components.  Most
//               transforms we use in everyday life can be so
//               described, but some kinds of transforms (for
//               instance, those involving a skew) cannot.
//
//               This is not related to whether the transform was
//               originally described componentwise.  Even a transform
//               that was constructed with a 4x4 may return true here
//               if the matrix is a simple affine matrix with no skew.
//
//               If this returns true, you may safely call get_hpr()
//               and get_scale() to retrieve the components.  (You
//               may always safely call get_pos() whether this returns
//               true or false.)
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
has_components() const {
  check_components();
  return ((_flags & F_has_components) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::components_given
//       Access: Published
//  Description: Returns true if the transform was specified
//               componentwise, or false if it was specified with a
//               general 4x4 matrix.  If this is true, the components
//               returned by get_pos() and get_scale() will be exactly
//               those that were set; otherwise, these functions will
//               return computed values.  If this is true, the
//               rotation may have been set either with a hpr trio or
//               with a quaternion; hpr_given() or quat_given() can
//               resolve the difference.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
components_given() const {
  return ((_flags & F_components_given) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::hpr_given
//       Access: Published
//  Description: Returns true if the rotation was specified via a trio
//               of Euler angles, false otherwise.  If this is true,
//               get_hpr() will be exactly as set; otherwise, it will
//               return a computed value.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
hpr_given() const {
  return ((_flags & F_hpr_given) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::quat_given
//       Access: Published
//  Description: Returns true if the rotation was specified via a 
//               quaternion, false otherwise.  If this is true,
//               get_quat() will be exactly as set; otherwise, it will
//               return a computed value.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
quat_given() const {
  return ((_flags & F_quat_given) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::has_pos
//       Access: Published
//  Description: Returns true if the transform's pos component can be
//               extracted out separately.  This is generally always
//               true, unless the transform is invalid
//               (i.e. is_invalid() returns true).
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
has_pos() const {
  return !is_invalid();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::has_hpr
//       Access: Published
//  Description: Returns true if the transform's rotation component
//               can be extracted out separately and described as a
//               set of Euler angles.  This is generally true only
//               when has_components() is true.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
has_hpr() const {
  return has_components();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::has_quat
//       Access: Published
//  Description: Returns true if the transform's rotation component
//               can be extracted out separately and described as a
//               quaternion.  This is generally true only when
//               has_components() is true.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
has_quat() const {
  return has_components();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::has_scale
//       Access: Published
//  Description: Returns true if the transform's scale component
//               can be extracted out separately.  This is generally
//               true only when has_components() is true.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
has_scale() const {
  return has_components();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::has_identity_scale
//       Access: Published
//  Description: Returns true if the scale is uniform 1.0, or false if
//               the scale has some real value.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
has_identity_scale() const {
  check_components();
  return (_flags & F_identity_scale) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::has_uniform_scale
//       Access: Published
//  Description: Returns true if the scale is uniform across all three
//               axes (and therefore can be expressed as a single
//               number), or false if the transform has a different
//               scale in different dimensions.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
has_uniform_scale() const {
  check_components();
  return (_flags & F_uniform_scale) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::has_shear
//       Access: Published
//  Description: Returns true if the transform's shear component
//               can be extracted out separately.  This is generally
//               true only when has_components() is true.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
has_shear() const {
  return has_components();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::has_nonzero_shear
//       Access: Published
//  Description: Returns true if the shear component is non-zero,
//               false if it is zero or if the matrix cannot be
//               decomposed.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
has_nonzero_shear() const {
  check_components();
  return (_flags & F_has_nonzero_shear) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::has_mat
//       Access: Published
//  Description: Returns true if the transform can be described as a
//               matrix.  This is generally always true, unless
//               is_invalid() is true.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
has_mat() const {
  return !is_invalid();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_pos
//       Access: Published
//  Description: Returns the pos component of the transform.  It is an
//               error to call this if has_pos() returned false.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase3f &TransformState::
get_pos() const {
  check_components();
  nassertr(has_pos(), _pos);
  return _pos;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_hpr
//       Access: Published
//  Description: Returns the rotation component of the transform as a
//               trio of Euler angles.  It is an error to call this if
//               has_components() returned false.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase3f &TransformState::
get_hpr() const {
  check_hpr();
  nassertr(!is_invalid(), _hpr);
  return _hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_quat
//       Access: Published
//  Description: Returns the rotation component of the transform as a
//               quaternion.  It is an error to call this if
//               has_components() returned false.
////////////////////////////////////////////////////////////////////
INLINE const LQuaternionf &TransformState::
get_quat() const {
  check_quat();
  nassertr(!is_invalid(), _quat);
  return _quat;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_scale
//       Access: Published
//  Description: Returns the scale component of the transform.  It is an
//               error to call this if has_components() returned
//               false.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase3f &TransformState::
get_scale() const {
  check_components();
  nassertr(!is_invalid(), _scale);
  return _scale;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_uniform_scale
//       Access: Published
//  Description: Returns the scale component of the transform, as a
//               single number.  It is an error to call this if
//               has_uniform_scale() returned false.
////////////////////////////////////////////////////////////////////
INLINE float TransformState::
get_uniform_scale() const {
  check_components();
  nassertr(has_uniform_scale(), _scale[0]);
  return _scale[0];
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_shear
//       Access: Published
//  Description: Returns the shear component of the transform.  It is
//               an error to call this if has_components() returned
//               false.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase3f &TransformState::
get_shear() const {
  check_components();
  nassertr(!is_invalid(), _shear);
  return _shear;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_mat
//       Access: Published
//  Description: Returns the matrix that describes the transform.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &TransformState::
get_mat() const {
  nassertr(has_mat(), LMatrix4f::ident_mat());
  check_mat();
  return _mat;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_pos2d
//       Access: Published
//  Description: Returns the pos component of the 2-d transform.  It
//               is an error to call this if has_pos() or is_2d()
//               returned false.
////////////////////////////////////////////////////////////////////
INLINE LVecBase2f TransformState::
get_pos2d() const {
  check_components();
  nassertr(has_pos() && is_2d(), LVecBase2f::zero());
  return LVecBase2f(_pos[0], _pos[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_rotate2d
//       Access: Published
//  Description: Returns the rotation component of the 2-d transform
//               as an angle in degrees clockwise about the origin.
//               It is an error to call this if has_components() or
//               is_2d() returned false.
////////////////////////////////////////////////////////////////////
INLINE float TransformState::
get_rotate2d() const {
  check_hpr();
  nassertr(!is_invalid() && is_2d(), 0);
  switch (get_default_coordinate_system()) {
  default:
  case CS_zup_right:
    return _hpr[0];
  case CS_zup_left:
    return -_hpr[0];
  case CS_yup_right:
    return -_hpr[2];
  case CS_yup_left:
    return _hpr[2];
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_scale2d
//       Access: Published
//  Description: Returns the scale component of the 2-d transform.  It
//               is an error to call this if has_components() or
//               is_2d() returned false.
////////////////////////////////////////////////////////////////////
INLINE LVecBase2f TransformState::
get_scale2d() const {
  check_components();
  nassertr(!is_invalid() && is_2d(), LVecBase2f::zero());
  return LVecBase2f(_scale[0], _scale[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_shear2d
//       Access: Published
//  Description: Returns the shear component of the 2-d transform.  It
//               is an error to call this if has_components() or
//               is_2d() returned false.
////////////////////////////////////////////////////////////////////
INLINE float TransformState::
get_shear2d() const {
  check_components();
  nassertr(!is_invalid() && is_2d(), 0.0f);
  return _shear[0];
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_mat3
//       Access: Published
//  Description: Returns the 3x3 matrix that describes the 2-d
//               transform.  It is an error to call this if is_2d()
//               returned false.
////////////////////////////////////////////////////////////////////
INLINE LMatrix3f TransformState::
get_mat3() const {
  nassertr(has_mat() && is_2d(), LMatrix3f::ident_mat());
  check_mat();
  return LMatrix3f(_mat(0, 0), _mat(0, 1), _mat(0, 3),
                   _mat(1, 0), _mat(1, 1), _mat(1, 3),
                   _mat(3, 0), _mat(3, 1), _mat(3, 3));
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_geom_rendering
//       Access: Published
//  Description: Returns the union of the Geom::GeomRendering bits
//               that will be required once this TransformState is
//               applied to a geom which includes the indicated
//               geom_rendering bits.  The RenderState's
//               get_geom_rendering() should already have been
//               applied.
////////////////////////////////////////////////////////////////////
INLINE int TransformState::
get_geom_rendering(int geom_rendering) const {
  if ((geom_rendering & GeomEnums::GR_point_perspective) != 0) {
    if (!has_identity_scale()) {
      geom_rendering |= GeomEnums::GR_point_scale;
    }
  }

  return geom_rendering;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::cache_ref
//       Access: Published
//  Description: Overrides this method to update PStats appropriately.
////////////////////////////////////////////////////////////////////
INLINE int TransformState::
cache_ref() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  int result = NodeCachedReferenceCount::cache_ref();
  consider_update_pstats(old_referenced_bits);
  return result;
#else  // DO_PSTATS
  return NodeCachedReferenceCount::cache_ref();
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::cache_unref
//       Access: Published
//  Description: Overrides this method to update PStats appropriately.
////////////////////////////////////////////////////////////////////
INLINE int TransformState::
cache_unref() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  int result = NodeCachedReferenceCount::cache_unref();
  consider_update_pstats(old_referenced_bits);
  return result;
#else  // DO_PSTATS
  return NodeCachedReferenceCount::cache_unref();
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::node_ref
//       Access: Published
//  Description: Overrides this method to update PStats appropriately.
////////////////////////////////////////////////////////////////////
INLINE int TransformState::
node_ref() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  int result = NodeCachedReferenceCount::node_ref();
  consider_update_pstats(old_referenced_bits);
  return result;
#else  // DO_PSTATS
  return NodeCachedReferenceCount::node_ref();
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::node_unref
//       Access: Published
//  Description: Overrides this method to update PStats appropriately.
////////////////////////////////////////////////////////////////////
INLINE int TransformState::
node_unref() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  int result = NodeCachedReferenceCount::node_unref();
  consider_update_pstats(old_referenced_bits);
  return result;
#else  // DO_PSTATS
  return NodeCachedReferenceCount::node_unref();
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::check_singular
//       Access: Private
//  Description: Ensures that we know whether the matrix is singular.
////////////////////////////////////////////////////////////////////
INLINE void TransformState::
check_singular() const {
  // This pretends to be a const function, even though it's not,
  // because it only updates a transparent cache value.
  if ((_flags & F_singular_known) == 0) {
    ((TransformState *)this)->calc_singular();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::check_components
//       Access: Private
//  Description: Ensures that we know the components of the transform
//               (or that we know they cannot be derived).
////////////////////////////////////////////////////////////////////
INLINE void TransformState::
check_components() const {
  // This pretends to be a const function, even though it's not,
  // because it only updates a transparent cache value.
  if ((_flags & F_components_known) == 0) {
    ((TransformState *)this)->calc_components();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::check_hpr
//       Access: Private
//  Description: Ensures that we know the hpr of the transform
//               (or that we know they cannot be derived).
////////////////////////////////////////////////////////////////////
INLINE void TransformState::
check_hpr() const {
  // This pretends to be a const function, even though it's not,
  // because it only updates a transparent cache value.
  if ((_flags & F_hpr_known) == 0) {
    ((TransformState *)this)->calc_hpr();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::check_quat
//       Access: Private
//  Description: Ensures that we know the quat of the transform
//               (or that we know they cannot be derived).
////////////////////////////////////////////////////////////////////
INLINE void TransformState::
check_quat() const {
  // This pretends to be a const function, even though it's not,
  // because it only updates a transparent cache value.
  if ((_flags & F_quat_known) == 0) {
    ((TransformState *)this)->calc_quat();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::check_mat
//       Access: Private
//  Description: Ensures that we know the overall matrix.
////////////////////////////////////////////////////////////////////
INLINE void TransformState::
check_mat() const {
  // This pretends to be a const function, even though it's not,
  // because it only updates a transparent cache value.
  if ((_flags & F_mat_known) == 0) {
    ((TransformState *)this)->calc_mat();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::check_uniform_scale
//       Access: Private
//  Description: Should be called immediately after _scale (and
//               F_has_components) is set, this checks for a
//               identity and/or uniform scale (as well as a non-zero
//               shear) and sets the bit appropriately.
////////////////////////////////////////////////////////////////////
INLINE void TransformState::
check_uniform_scale() {
  if (IS_NEARLY_EQUAL(_scale[0], _scale[1]) && 
      IS_NEARLY_EQUAL(_scale[0], _scale[2])) {
    _flags |= F_uniform_scale;
    if (IS_NEARLY_EQUAL(_scale[0], 1.0f)) {
      _flags |= F_identity_scale;
    }
  }

  if (!_shear.almost_equal(LVecBase3f::zero())) {
    _flags |= F_has_nonzero_shear;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::check_uniform_scale2d
//       Access: Private
//  Description: Should be called immediately after _scale (and
//               F_has_components) is set, for a known 2-d scale, this
//               checks for a identity and/or uniform scale (as well
//               as a non-zero shear) and sets the bit appropriately.
////////////////////////////////////////////////////////////////////
INLINE void TransformState::
check_uniform_scale2d() {
  if (IS_NEARLY_EQUAL(_scale[0], _scale[1])) {
    _scale[2] = _scale[0];
    _flags |= F_uniform_scale;
    if (IS_NEARLY_EQUAL(_scale[0], 1.0f)) {
      _flags |= F_identity_scale;
    }
  }

  if (!_shear.almost_equal(LVecBase3f::zero())) {
    _flags |= F_has_nonzero_shear;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::set_destructing
//       Access: Private
//  Description: This function should only be called from the
//               destructor; it indicates that this TransformState
//               object is beginning destruction.  It is only used as
//               a sanity check, and is only meaningful when NDEBUG is
//               not defined.
////////////////////////////////////////////////////////////////////
INLINE void TransformState::
set_destructing() {
#ifndef NDEBUG
  _flags |= F_is_destructing;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::is_destructing
//       Access: Private
//  Description: Returns true if the TransformState object is
//               currently within its destructor
//               (i.e. set_destructing() has been called).  This is
//               only used as a sanity check, and is only meaningful
//               when NDEBUG is not defined.
////////////////////////////////////////////////////////////////////
INLINE bool TransformState::
is_destructing() const {
#ifndef NDEBUG
  return (_flags & F_is_destructing) != 0;
#else
  return false;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::consider_update_pstats
//       Access: Private
//  Description: Calls update_pstats() if the state of the referenced
//               bits has changed from the indicated value.
////////////////////////////////////////////////////////////////////
INLINE void TransformState::
consider_update_pstats(int old_referenced_bits) const {
  int new_referenced_bits = get_referenced_bits();
  if (old_referenced_bits != new_referenced_bits) {
    update_pstats(old_referenced_bits, new_referenced_bits);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::Composition::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TransformState::Composition::
Composition() {
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::Composition::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TransformState::Composition::
Composition(const TransformState::Composition &copy) :
  _result(copy._result)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::CompositionCycleDescEntry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE TransformState::CompositionCycleDescEntry::
CompositionCycleDescEntry(const TransformState *obj,
                          const TransformState *result,
                          bool inverted) :
  _obj(obj),
  _result(result),
  _inverted(inverted)
{
}
