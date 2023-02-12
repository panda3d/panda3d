/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file transformState.I
 * @author drose
 * @date 2002-02-25
 */

/**
 * Opposite of operator ==.
 */
INLINE bool TransformState::
operator != (const TransformState &other) const {
  return !(operator == (other));
}

/**
 * Provides an arbitrary ordering among all unique TransformStates, so we can
 * store the essentially different ones in a big set and throw away the rest.
 *
 * Note that if this returns 0, it doesn't necessarily imply that operator ==
 * returns true; it uses a very slightly different comparison threshold.
 */
INLINE int TransformState::
compare_to(const TransformState &other) const {
  return compare_to(other, _uniquify_matrix);
}

/**
 * Returns a suitable hash value for phash_map.
 */
INLINE size_t TransformState::
get_hash() const {
  check_hash();
  return _hash;
}

/**
 * Constructs an identity transform.
 */
INLINE CPT(TransformState) TransformState::
make_identity() {
  if (UNLIKELY(_states_lock == nullptr)) {
    init_states();
  }

  return _identity_state;
}

/**
 * Constructs an invalid transform; for instance, the result of inverting a
 * singular matrix.
 */
INLINE CPT(TransformState) TransformState::
make_invalid() {
  if (UNLIKELY(_states_lock == nullptr)) {
    init_states();
  }

  return _invalid_state;
}

/**
 * Makes a new TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_pos(const LVecBase3 &pos) {
  return make_pos_hpr_scale(pos,
                            LVecBase3(0.0f, 0.0f, 0.0f),
                            LVecBase3(1.0f, 1.0f, 1.0f));
}

/**
 * Makes a new TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_hpr(const LVecBase3 &hpr) {
  return make_pos_hpr_scale(LVecBase3(0.0f, 0.0f, 0.0f),
                            hpr,
                            LVecBase3(1.0f, 1.0f, 1.0f));
}

/**
 * Makes a new TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_quat(const LQuaternion &quat) {
  return make_pos_quat_scale(LVecBase3(0.0f, 0.0f, 0.0f),
                             quat,
                             LVecBase3(1.0f, 1.0f, 1.0f));
}

/**
 * Makes a new TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_pos_hpr(const LVecBase3 &pos, const LVecBase3 &hpr) {
  return make_pos_hpr_scale(pos, hpr,
                            LVecBase3(1.0, 1.0f, 1.0f));
}

/**
 * Makes a new TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_pos_quat(const LVecBase3 &pos, const LQuaternion &quat) {
  return make_pos_quat_scale(pos, quat,
                             LVecBase3(1.0, 1.0f, 1.0f));
}

/**
 * Makes a new TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_scale(PN_stdfloat scale) {
  // We actually map this 3-d uniform make_scale() to the 2-d version--might
  // as well call it a 2-d scale.
  return make_scale2d(scale);
}

/**
 * Makes a new TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_scale(const LVecBase3 &scale) {
  return make_pos_hpr_scale(LVecBase3(0.0f, 0.0f, 0.0f),
                            LVecBase3(0.0f, 0.0f, 0.0f),
                            scale);
}

/**
 * Makes a new TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_shear(const LVecBase3 &shear) {
  return make_pos_hpr_scale_shear(LVecBase3(0.0f, 0.0f, 0.0f),
                                  LVecBase3(0.0f, 0.0f, 0.0f),
                                  LVecBase3(1.0f, 1.0f, 1.0f),
                                  shear);
}

/**
 * Makes a new TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_pos_hpr_scale(const LVecBase3 &pos, const LVecBase3 &hpr,
                   const LVecBase3 &scale) {
  return make_pos_hpr_scale_shear(pos, hpr, scale, LVecBase3::zero());
}

/**
 * Makes a new TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_pos_quat_scale(const LVecBase3 &pos, const LQuaternion &quat,
                    const LVecBase3 &scale) {
  return make_pos_quat_scale_shear(pos, quat, scale, LVecBase3::zero());
}

/**
 * Makes a new 2-d TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_pos2d(const LVecBase2 &pos) {
  return make_pos_rotate_scale2d(pos, 0.0f, LVecBase2(1.0f, 1.0f));
}

/**
 * Makes a new 2-d TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_rotate2d(PN_stdfloat rotate) {
  return make_pos_rotate_scale2d(LVecBase2(0.0f, 0.0f), rotate,
                                 LVecBase2(1.0f, 1.0f));
}

/**
 * Makes a new 2-d TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_pos_rotate2d(const LVecBase2 &pos, PN_stdfloat rotate) {
  return make_pos_rotate_scale2d(pos, rotate,
                                 LVecBase2(1.0, 1.0f));
}

/**
 * Makes a new 2-d TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_scale2d(PN_stdfloat scale) {
  return make_pos_rotate_scale2d(LVecBase2(0.0f, 0.0f), 0.0f,
                                 LVecBase2(scale, scale));
}

/**
 * Makes a new 2-d TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_scale2d(const LVecBase2 &scale) {
  return make_pos_rotate_scale2d(LVecBase2(0.0f, 0.0f), 0.0f, scale);
}

/**
 * Makes a new 2-d TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_shear2d(PN_stdfloat shear) {
  return make_pos_rotate_scale_shear2d(LVecBase2(0.0f, 0.0f), 0.0f,
                                       LVecBase2(1.0f, 1.0f), shear);
}

/**
 * Makes a new 2-d TransformState with the specified components.
 */
INLINE CPT(TransformState) TransformState::
make_pos_rotate_scale2d(const LVecBase2 &pos, PN_stdfloat rotate,
                        const LVecBase2 &scale) {
  return make_pos_rotate_scale_shear2d(pos, rotate, scale, 0.0f);
}

/**
 * Returns true if the transform represents the identity matrix, false
 * otherwise.
 */
INLINE bool TransformState::
is_identity() const {
  return this == _identity_state;
}

/**
 * Returns true if the transform represents an invalid matrix, for instance
 * the result of inverting a singular matrix, or false if the transform is
 * valid.
 */
INLINE bool TransformState::
is_invalid() const {
  return this == _invalid_state;
}

/**
 * Returns true if the transform represents a singular transform (that is, it
 * has a zero scale, and it cannot be inverted), or false otherwise.
 */
INLINE bool TransformState::
is_singular() const {
  check_singular();
  return ((_flags & F_is_singular) != 0);
}

/**
 * Returns true if the transform has been constructed entirely using the 2-d
 * transform operations, e.g.  make_pos2d(), and therefore operates strictly
 * in two-dimensional space on X and Y only.
 */
INLINE bool TransformState::
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
INLINE bool TransformState::
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
INLINE bool TransformState::
components_given() const {
  return ((_flags & F_components_given) != 0);
}

/**
 * Returns true if the rotation was specified via a trio of Euler angles,
 * false otherwise.  If this is true, get_hpr() will be exactly as set;
 * otherwise, it will return a computed value.
 */
INLINE bool TransformState::
hpr_given() const {
  return ((_flags & F_hpr_given) != 0);
}

/**
 * Returns true if the rotation was specified via a quaternion, false
 * otherwise.  If this is true, get_quat() will be exactly as set; otherwise,
 * it will return a computed value.
 */
INLINE bool TransformState::
quat_given() const {
  return ((_flags & F_quat_given) != 0);
}

/**
 * Returns true if the transform's pos component can be extracted out
 * separately.  This is generally always true, unless the transform is invalid
 * (i.e.  is_invalid() returns true).
 */
INLINE bool TransformState::
has_pos() const {
  return !is_invalid();
}

/**
 * Returns true if the transform's rotation component can be extracted out
 * separately and described as a set of Euler angles.  This is generally true
 * only when has_components() is true.
 */
INLINE bool TransformState::
has_hpr() const {
  return has_components();
}

/**
 * Returns true if the transform's rotation component can be extracted out
 * separately and described as a quaternion.  This is generally true only when
 * has_components() is true.
 */
INLINE bool TransformState::
has_quat() const {
  return has_components();
}

/**
 * Returns true if the transform's scale component can be extracted out
 * separately.  This is generally true only when has_components() is true.
 */
INLINE bool TransformState::
has_scale() const {
  return has_components();
}

/**
 * Returns true if the scale is uniform 1.0, or false if the scale has some
 * real value.
 */
INLINE bool TransformState::
has_identity_scale() const {
  check_components();
  return (_flags & F_identity_scale) != 0;
}

/**
 * Returns true if the scale is uniform across all three axes (and therefore
 * can be expressed as a single number), or false if the transform has a
 * different scale in different dimensions.
 */
INLINE bool TransformState::
has_uniform_scale() const {
  check_components();
  return (_flags & F_uniform_scale) != 0;
}

/**
 * Returns true if the transform's shear component can be extracted out
 * separately.  This is generally true only when has_components() is true.
 */
INLINE bool TransformState::
has_shear() const {
  return has_components();
}

/**
 * Returns true if the shear component is non-zero, false if it is zero or if
 * the matrix cannot be decomposed.
 */
INLINE bool TransformState::
has_nonzero_shear() const {
  check_components();
  return (_flags & F_has_nonzero_shear) != 0;
}

/**
 * Returns true if the transform can be described as a matrix.  This is
 * generally always true, unless is_invalid() is true.
 */
INLINE bool TransformState::
has_mat() const {
  return !is_invalid();
}

/**
 * Returns the pos component of the transform.  It is an error to call this if
 * has_pos() returned false.
 */
INLINE const LPoint3 &TransformState::
get_pos() const {
  check_components();
  nassertr(has_pos(), _pos);
  return _pos;
}

/**
 * Returns the rotation component of the transform as a trio of Euler angles.
 * It is an error to call this if has_components() returned false.
 */
INLINE const LVecBase3 &TransformState::
get_hpr() const {
  check_hpr();
  nassertr(!is_invalid(), _hpr);
  return _hpr;
}

/**
 * Returns the rotation component of the transform as a quaternion.  The
 * return value will be normalized if a normalized quaternion was given to the
 * constructor (or if the quaternion was computed implicitly); it will be non-
 * normalized if a non-normalized quaternion was given to the constructor.
 * See also get_norm_quat().
 *
 * It is an error to call this if has_components() returned false.
 */
INLINE const LQuaternion &TransformState::
get_quat() const {
  check_quat();
  nassertr(!is_invalid(), _quat);
  return _quat;
}

/**
 * Returns the rotation component of the transform as a quaternion.  Unlike
 * the result of get_quat(), the return value of this method is guaranteed to
 * be normalized.  It is an error to call this if has_components() returned
 * false.
 */
INLINE const LQuaternion &TransformState::
get_norm_quat() const {
  check_norm_quat();
  nassertr(!is_invalid(), _norm_quat);
  return _norm_quat;
}

/**
 * Returns the scale component of the transform.  It is an error to call this
 * if has_components() returned false.
 */
INLINE const LVecBase3 &TransformState::
get_scale() const {
  check_components();
  nassertr(!is_invalid(), _scale);
  return _scale;
}

/**
 * Returns the scale component of the transform, as a single number.  It is an
 * error to call this if has_uniform_scale() returned false.
 */
INLINE PN_stdfloat TransformState::
get_uniform_scale() const {
  check_components();
  nassertr(has_uniform_scale(), _scale[0]);
  return _scale[0];
}

/**
 * Returns the shear component of the transform.  It is an error to call this
 * if has_components() returned false.
 */
INLINE const LVecBase3 &TransformState::
get_shear() const {
  check_components();
  nassertr(!is_invalid(), _shear);
  return _shear;
}

/**
 * Returns the matrix that describes the transform.
 */
INLINE const LMatrix4 &TransformState::
get_mat() const {
  nassertr(has_mat(), LMatrix4::ident_mat());
  check_mat();
  return _mat;
}

/**
 * Returns the inverse of the matrix, or nullptr if it's singular.
 */
INLINE const LMatrix4 *TransformState::
get_inverse_mat() const {
  if (!is_singular()) {
    nassertr(_inv_mat != nullptr, nullptr);
    return _inv_mat;
  }
  else {
    return nullptr;
  }
}

/**
 * Returns the pos component of the 2-d transform.  It is an error to call
 * this if has_pos() or is_2d() returned false.
 */
INLINE LVecBase2 TransformState::
get_pos2d() const {
  check_components();
  nassertr(has_pos() && is_2d(), LVecBase2::zero());
  return LVecBase2(_pos[0], _pos[1]);
}

/**
 * Returns the rotation component of the 2-d transform as an angle in degrees
 * clockwise about the origin.  It is an error to call this if
 * has_components() or is_2d() returned false.
 */
INLINE PN_stdfloat TransformState::
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

/**
 * Returns the scale component of the 2-d transform.  It is an error to call
 * this if has_components() or is_2d() returned false.
 */
INLINE LVecBase2 TransformState::
get_scale2d() const {
  check_components();
  nassertr(!is_invalid() && is_2d(), LVecBase2::zero());
  return LVecBase2(_scale[0], _scale[1]);
}

/**
 * Returns the shear component of the 2-d transform.  It is an error to call
 * this if has_components() or is_2d() returned false.
 */
INLINE PN_stdfloat TransformState::
get_shear2d() const {
  check_components();
  nassertr(!is_invalid() && is_2d(), 0.0f);
  return _shear[0];
}

/**
 * Returns the 3x3 matrix that describes the 2-d transform.  It is an error to
 * call this if is_2d() returned false.
 */
INLINE LMatrix3 TransformState::
get_mat3() const {
  nassertr(has_mat() && is_2d(), LMatrix3::ident_mat());
  check_mat();
  return LMatrix3(_mat(0, 0), _mat(0, 1), _mat(0, 3),
                   _mat(1, 0), _mat(1, 1), _mat(1, 3),
                   _mat(3, 0), _mat(3, 1), _mat(3, 3));
}

/**
 * Returns the inverse of this transform.  If you are going to immediately
 * compose this result with another TransformState, it is faster to do it in
 * one operation with invert_compose().
 */
INLINE CPT(TransformState) TransformState::
get_inverse() const {
  return invert_compose(TransformState::make_identity());
}

/**
 * Returns the pointer to the unique TransformState in the cache that is
 * equivalent to this one.  This may be the same pointer as this object, or it
 * may be a different pointer; but it will be an equivalent object, and it
 * will be a shared pointer.  This may be called from time to time to improve
 * cache benefits.
 */
INLINE CPT(TransformState) TransformState::
get_unique() const {
  return return_unique((TransformState *)this);
}

/**
 * Returns the union of the Geom::GeomRendering bits that will be required
 * once this TransformState is applied to a geom which includes the indicated
 * geom_rendering bits.  The RenderState's get_geom_rendering() should already
 * have been applied.
 */
INLINE int TransformState::
get_geom_rendering(int geom_rendering) const {
  if ((geom_rendering & GeomEnums::GR_point_perspective) != 0) {
    if (!has_identity_scale()) {
      geom_rendering |= GeomEnums::GR_point_scale;
    }
  }

  return geom_rendering;
}

/**
 * Overrides this method to update PStats appropriately.
 */
INLINE void TransformState::
cache_ref() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  NodeCachedReferenceCount::cache_ref();
  consider_update_pstats(old_referenced_bits);
#else  // DO_PSTATS
  NodeCachedReferenceCount::cache_ref();
#endif  // DO_PSTATS
}

/**
 * Overrides this method to update PStats appropriately.
 */
INLINE bool TransformState::
cache_unref() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  bool result = do_cache_unref();
  consider_update_pstats(old_referenced_bits);
  return result;
#else  // DO_PSTATS
  return do_cache_unref();
#endif  // DO_PSTATS
}

/**
 * Overrides this method to update PStats appropriately.
 */
INLINE void TransformState::
node_ref() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  NodeCachedReferenceCount::node_ref();
  consider_update_pstats(old_referenced_bits);
#else  // DO_PSTATS
  NodeCachedReferenceCount::node_ref();
#endif  // DO_PSTATS
}

/**
 * Overrides this method to update PStats appropriately.
 */
INLINE bool TransformState::
node_unref() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  bool result = do_node_unref();
  consider_update_pstats(old_referenced_bits);
  return result;
#else  // DO_PSTATS
  return do_node_unref();
#endif  // DO_PSTATS
}

/**
 * Returns the number of entries in the composition cache for this
 * TransformState.  This is the number of other TransformStates whose
 * composition with this one has been cached.  This number is not useful for
 * any practical reason other than performance analysis.
 */
INLINE size_t TransformState::
get_composition_cache_num_entries() const {
  LightReMutexHolder holder(*_states_lock);
  return _composition_cache.get_num_entries();
}

/**
 * Returns the number of entries in the invert_composition cache for this
 * TransformState.  This is similar to the composition cache, but it records
 * cache entries for the invert_compose() operation.  See
 * get_composition_cache_num_entries().
 */
INLINE size_t TransformState::
get_invert_composition_cache_num_entries() const {
  LightReMutexHolder holder(*_states_lock);
  return _invert_composition_cache.get_num_entries();
}

/**
 * Returns the number of slots in the composition cache for this
 * TransformState.  You may use this as an upper bound when walking through
 * all of the composition cache results via get_composition_cache_source() or
 * result().
 *
 * This has no practical value other than for examining the cache for
 * performance analysis.
 */
INLINE size_t TransformState::
get_composition_cache_size() const {
  LightReMutexHolder holder(*_states_lock);
  return _composition_cache.get_num_entries();
}

/**
 * Returns the source TransformState of the nth element in the composition
 * cache.  Returns NULL if there doesn't happen to be an entry in the nth
 * element.  See get_composition_cache_result().
 *
 * This has no practical value other than for examining the cache for
 * performance analysis.
 */
INLINE const TransformState *TransformState::
get_composition_cache_source(size_t n) const {
  LightReMutexHolder holder(*_states_lock);
  return _composition_cache.get_key(n);
}

/**
 * Returns the result TransformState of the nth element in the composition
 * cache.  Returns NULL if there doesn't happen to be an entry in the nth
 * element.
 *
 * In general, a->compose(a->get_composition_cache_source(n)) ==
 * a->get_composition_cache_result(n).
 *
 * This has no practical value other than for examining the cache for
 * performance analysis.
 */
INLINE const TransformState *TransformState::
get_composition_cache_result(size_t n) const {
  LightReMutexHolder holder(*_states_lock);
  return _composition_cache.get_data(n)._result;
}

/**
 * Returns the number of slots in the composition cache for this
 * TransformState.  You may use this as an upper bound when walking through
 * all of the composition cache results via
 * get_invert_composition_cache_source() or result().
 *
 * This has no practical value other than for examining the cache for
 * performance analysis.
 */
INLINE size_t TransformState::
get_invert_composition_cache_size() const {
  LightReMutexHolder holder(*_states_lock);
  return _invert_composition_cache.get_num_entries();
}

/**
 * Returns the source TransformState of the nth element in the invert
 * composition cache.  Returns NULL if there doesn't happen to be an entry in
 * the nth element.  See get_invert_composition_cache_result().
 *
 * This has no practical value other than for examining the cache for
 * performance analysis.
 */
INLINE const TransformState *TransformState::
get_invert_composition_cache_source(size_t n) const {
  LightReMutexHolder holder(*_states_lock);
  return _invert_composition_cache.get_key(n);
}

/**
 * Returns the result TransformState of the nth element in the invert
 * composition cache.  Returns NULL if there doesn't happen to be an entry in
 * the nth element.
 *
 * In general, a->invert_compose(a->get_invert_composition_cache_source(n)) ==
 * a->get_invert_composition_cache_result(n).
 *
 * This has no practical value other than for examining the cache for
 * performance analysis.
 */
INLINE const TransformState *TransformState::
get_invert_composition_cache_result(size_t n) const {
  LightReMutexHolder holder(*_states_lock);
  return _invert_composition_cache.get_data(n)._result;
}

/**
 * Flushes the PStatCollectors used during traversal.
 */
INLINE void TransformState::
flush_level() {
  _node_counter.flush_level();
  _cache_counter.flush_level();
}

/**
 * Overrides this method to update PStats appropriately.
 */
INLINE void TransformState::
cache_ref_only() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  NodeCachedReferenceCount::cache_ref_only();
  consider_update_pstats(old_referenced_bits);
#else  // DO_PSTATS
  NodeCachedReferenceCount::cache_ref_only();
#endif  // DO_PSTATS
}

/**
 * Overrides this method to update PStats appropriately.
 */
INLINE void TransformState::
cache_unref_only() const {
#ifdef DO_PSTATS
  int old_referenced_bits = get_referenced_bits();
  NodeCachedReferenceCount::cache_unref_only();
  consider_update_pstats(old_referenced_bits);
#else  // DO_PSTATS
  NodeCachedReferenceCount::cache_unref_only();
#endif  // DO_PSTATS
}

/**
 * Reimplements NodeReferenceCount::node_unref().  We do this because we have
 * a non-virtual unref() method.
 */
INLINE bool TransformState::
do_node_unref() const {
  node_unref_only();
  return unref();
}

/**
 * Reimplements CachedTypedWritableReferenceCount::cache_unref().  We do this
 * because we have a non-virtual unref() method.
 */
INLINE bool TransformState::
do_cache_unref() const {
  NodeCachedReferenceCount::cache_unref_only();
  return unref();
}

/**
 * Ensures that we know the hash value.
 */
INLINE void TransformState::
check_hash() const {
  // This pretends to be a const function, even though it's not, because it
  // only updates a transparent cache value.
  if (_hash == H_unknown) {
    calc_hash();
  }
}

/**
 * Ensures that we know whether the matrix is singular.
 */
INLINE void TransformState::
check_singular() const {
  // This pretends to be a const function, even though it's not, because it
  // only updates a transparent cache value.
  if ((_flags & F_singular_known) == 0) {
    ((TransformState *)this)->calc_singular();
  }
}

/**
 * Ensures that we know the components of the transform (or that we know they
 * cannot be derived).
 */
INLINE void TransformState::
check_components() const {
  // This pretends to be a const function, even though it's not, because it
  // only updates a transparent cache value.
  if ((_flags & F_components_known) == 0) {
    ((TransformState *)this)->calc_components();
  }
}

/**
 * Ensures that we know the hpr of the transform (or that we know they cannot
 * be derived).
 */
INLINE void TransformState::
check_hpr() const {
  // This pretends to be a const function, even though it's not, because it
  // only updates a transparent cache value.
  if ((_flags & F_hpr_known) == 0) {
    ((TransformState *)this)->calc_hpr();
  }
}

/**
 * Ensures that we know the quat of the transform (or that we know they cannot
 * be derived).
 */
INLINE void TransformState::
check_quat() const {
  // This pretends to be a const function, even though it's not, because it
  // only updates a transparent cache value.
  if ((_flags & F_quat_known) == 0) {
    ((TransformState *)this)->calc_quat();
  }
}

/**
 * Ensures that we know the normalized quat of the transform (or that we know
 * they cannot be derived).
 */
INLINE void TransformState::
check_norm_quat() const {
  // This pretends to be a const function, even though it's not, because it
  // only updates a transparent cache value.
  if ((_flags & F_norm_quat_known) == 0) {
    ((TransformState *)this)->calc_norm_quat();
  }
}

/**
 * Ensures that we know the overall matrix.
 */
INLINE void TransformState::
check_mat() const {
  // This pretends to be a const function, even though it's not, because it
  // only updates a transparent cache value.
  if ((_flags & F_mat_known) == 0) {
    ((TransformState *)this)->calc_mat();
  }
}

/**
 * Derives the components from the matrix, if possible.
 */
INLINE void TransformState::
calc_components() {
  LightMutexHolder holder(_lock);
  do_calc_components();
}

/**
 * Derives the hpr, from the matrix if necessary, or from the quat.
 */
INLINE void TransformState::
calc_hpr() {
  LightMutexHolder holder(_lock);
  do_calc_hpr();
}

/**
 * Computes the matrix from the components.
 */
INLINE void TransformState::
calc_mat() {
  LightMutexHolder holder(_lock);
  do_calc_mat();
}

/**
 * Should be called immediately after _scale (and F_has_components) is set,
 * this checks for a identity and/or uniform scale (as well as a non-zero
 * shear) and sets the bit appropriately.
 *
 * It does not matter whether the lock is or is not held before calling this
 * method.
 */
INLINE void TransformState::
check_uniform_scale() {
  if (IS_NEARLY_EQUAL(_scale[0], _scale[1]) &&
      IS_NEARLY_EQUAL(_scale[0], _scale[2])) {
    _flags |= F_uniform_scale;
    if (IS_NEARLY_EQUAL(_scale[0], 1.0f)) {
      _flags |= F_identity_scale;
    }
  }

  if (!_shear.almost_equal(LVecBase3::zero())) {
    _flags |= F_has_nonzero_shear;
  }
}

/**
 * Should be called immediately after _scale (and F_has_components) is set,
 * for a known 2-d scale, this checks for a identity and/or uniform scale (as
 * well as a non-zero shear) and sets the bit appropriately.
 *
 * It does not matter whether the lock is or is not held before calling this
 * method.
 */
INLINE void TransformState::
check_uniform_scale2d() {
  if (IS_NEARLY_EQUAL(_scale[0], _scale[1])) {
    _scale[2] = _scale[0];
    _flags |= F_uniform_scale;
    if (IS_NEARLY_EQUAL(_scale[0], 1.0f)) {
      _flags |= F_identity_scale;
    }
  }

  if (!_shear.almost_equal(LVecBase3::zero())) {
    _flags |= F_has_nonzero_shear;
  }
}

/**
 * This function should only be called from the destructor; it indicates that
 * this TransformState object is beginning destruction.  It is only used as a
 * sanity check, and is only meaningful when NDEBUG is not defined.
 */
INLINE void TransformState::
set_destructing() {
#ifndef NDEBUG
  _flags |= F_is_destructing;
#endif
}

/**
 * Returns true if the TransformState object is currently within its
 * destructor (i.e.  set_destructing() has been called).  This is only used as
 * a sanity check, and is only meaningful when NDEBUG is not defined.
 */
INLINE bool TransformState::
is_destructing() const {
#ifndef NDEBUG
  return (_flags & F_is_destructing) != 0;
#else
  return false;
#endif
}

/**
 * Calls update_pstats() if the state of the referenced bits has changed from
 * the indicated value.
 */
INLINE void TransformState::
consider_update_pstats(int old_referenced_bits) const {
#ifdef DO_PSTATS
  int new_referenced_bits = get_referenced_bits();
  if (old_referenced_bits != new_referenced_bits) {
    update_pstats(old_referenced_bits, new_referenced_bits);
  }
#endif  // DO_PSTATS
}

/**
 *
 */
INLINE TransformState::CompositionCycleDescEntry::
CompositionCycleDescEntry(const TransformState *obj,
                          const TransformState *result,
                          bool inverted) :
  _obj(obj),
  _result(result),
  _inverted(inverted)
{
}
