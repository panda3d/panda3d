/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lvecBase3_src.I
 * @author drose
 * @date 2000-03-08
 */

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase3)::
FLOATNAME(LVecBase3)(FLOATTYPE fill_value) {
  fill(fill_value);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase3)::
FLOATNAME(LVecBase3)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z) {
  TAU_PROFILE("LVecBase3::LVecBase3(FLOATTYPE, ...)", " ", TAU_USER);
  _v(0) = x;
  _v(1) = y;
  _v(2) = z;
// set(x, y, z);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase3)::
FLOATNAME(LVecBase3)(const FLOATNAME(LVecBase2) &copy, FLOATTYPE z) {
  set(copy[0], copy[1], z);
}

/**
 * Returns a zero-length vector.
 */
INLINE_LINMATH const FLOATNAME(LVecBase3) &FLOATNAME(LVecBase3)::
zero() {
  return _zero;
}

/**
 * Returns a unit X vector.
 */
INLINE_LINMATH const FLOATNAME(LVecBase3) &FLOATNAME(LVecBase3)::
unit_x() {
  return _unit_x;
}

/**
 * Returns a unit Y vector.
 */
INLINE_LINMATH const FLOATNAME(LVecBase3) &FLOATNAME(LVecBase3)::
unit_y() {
  return _unit_y;
}

/**
 * Returns a unit Z vector.
 */
INLINE_LINMATH const FLOATNAME(LVecBase3) &FLOATNAME(LVecBase3)::
unit_z() {
  return _unit_z;
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase3)::
operator [](int i) const {
  nassertr(i >= 0 && i < 3, 0);
  return _v(i);
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE &FLOATNAME(LVecBase3)::
operator [](int i) {
  nassertr(i >= 0 && i < 3, _v(0));
  return _v(i);
}

/**
 * Returns true if any component of the vector is not-a-number, false
 * otherwise.
 */
INLINE_LINMATH bool FLOATNAME(LVecBase3)::
is_nan() const {
#ifdef FLOATTYPE_IS_INT
  return false;
#else
  TAU_PROFILE("bool LVecBase3::is_nan()", " ", TAU_USER);
  return cnan(_v(0)) || cnan(_v(1)) || cnan(_v(2));
#endif
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase3)::
get_cell(int i) const {
  nassertr(i >= 0 && i < 3, 0);
  return _v(i);
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase3)::
get_x() const {
  return _v(0);
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase3)::
get_y() const {
  return _v(1);
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase3)::
get_z() const {
  return _v(2);
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
set_cell(int i, FLOATTYPE value) {
  nassertv(i >= 0 && i < 3);
  _v(i) = value;
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
set_x(FLOATTYPE value) {
  _v(0) = value;
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
set_y(FLOATTYPE value) {
  _v(1) = value;
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
set_z(FLOATTYPE value) {
  _v(2) = value;
}

/**
 * Returns a 2-component vector that shares just the first two components of
 * this vector.
 */
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LVecBase3)::
get_xy() const {
  return FLOATNAME(LVecBase2)(_v(0), _v(1));
}

/**
 * Returns a 2-component vector that shares just the first and last components
 * of this vector.
 */
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LVecBase3)::
get_xz() const {
  return FLOATNAME(LVecBase2)(_v(0), _v(2));
}

/**
 * Returns a 2-component vector that shares just the last two components of
 * this vector.
 */
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LVecBase3)::
get_yz() const {
  return FLOATNAME(LVecBase2)(_v(1), _v(2));
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
add_to_cell(int i, FLOATTYPE value) {
  nassertv(i >= 0 && i < 3);
  _v(i) += value;
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
add_x(FLOATTYPE value) {
  _v(0) += value;
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
add_y(FLOATTYPE value) {
  _v(1) += value;
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
add_z(FLOATTYPE value) {
  _v(2) += value;
}

/**
 * Returns the address of the first of the three data elements in the vector.
 * The remaining elements occupy the next positions consecutively in memory.
 */
INLINE_LINMATH const FLOATTYPE *FLOATNAME(LVecBase3)::
get_data() const {
  return &_v(0);
}

/**
 * Returns an iterator that may be used to traverse the elements of the
 * matrix, STL-style.
 */
INLINE_LINMATH FLOATNAME(LVecBase3)::iterator FLOATNAME(LVecBase3)::
begin() {
  return &_v(0);
}

/**
 * Returns an iterator that may be used to traverse the elements of the
 * matrix, STL-style.
 */
INLINE_LINMATH FLOATNAME(LVecBase3)::iterator FLOATNAME(LVecBase3)::
end() {
  return begin() + num_components;
}

/**
 * Returns an iterator that may be used to traverse the elements of the
 * matrix, STL-style.
 */
INLINE_LINMATH FLOATNAME(LVecBase3)::const_iterator FLOATNAME(LVecBase3)::
begin() const {
  return &_v(0);
}

/**
 * Returns an iterator that may be used to traverse the elements of the
 * matrix, STL-style.
 */
INLINE_LINMATH FLOATNAME(LVecBase3)::const_iterator FLOATNAME(LVecBase3)::
end() const {
  return begin() + num_components;
}

/**
 * Sets each element of the vector to the indicated fill_value.  This is
 * particularly useful for initializing to zero.
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
fill(FLOATTYPE fill_value) {
  TAU_PROFILE("void LVecBase3::fill()", " ", TAU_USER);
#ifdef HAVE_EIGEN
  _v = EVector3::Constant(fill_value);
#else
  _v(0) = fill_value;
  _v(1) = fill_value;
  _v(2) = fill_value;
#endif  // HAVE_EIGEN
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
set(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z) {
  TAU_PROFILE("void LVecBase3::set()", " ", TAU_USER);
  _v(0) = x;
  _v(1) = y;
  _v(2) = z;
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase3)::
dot(const FLOATNAME(LVecBase3) &other) const {
  TAU_PROFILE("FLOATTYPE LVecBase3::dot()", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return _v.dot(other._v);
#else
  return _v(0) * other._v(0) + _v(1) * other._v(1) + _v(2) * other._v(2);
#endif  // HAVE_EIGEN
}

/**
 * Returns the square of the vector's length, cheap and easy.
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase3)::
length_squared() const {
  TAU_PROFILE("FLOATTYPE LVecBase3::length_squared()", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return _v.squaredNorm();
#else
  return (*this).dot(*this);
#endif  // HAVE_EIGEN
}

#ifndef FLOATTYPE_IS_INT
/**
 * Returns the length of the vector, by the Pythagorean theorem.
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase3)::
length() const {
  TAU_PROFILE("FLOATTYPE LVecBase3::length()", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return _v.norm();
#else
  return csqrt((*this).dot(*this));
#endif  // HAVE_EIGEN
}

/**
 * Normalizes the vector in place.  Returns true if the vector was normalized,
 * false if it was a zero-length vector.
 */
INLINE_LINMATH bool FLOATNAME(LVecBase3)::
normalize() {
  TAU_PROFILE("bool LVecBase3::normalize()", " ", TAU_USER);
  FLOATTYPE l2 = length_squared();
  if (l2 == (FLOATTYPE)0.0f) {
    set(0.0f, 0.0f, 0.0f);
    return false;

  } else if (!IS_THRESHOLD_EQUAL(l2, 1.0f, (NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE)))) {
    (*this) /= csqrt(l2);
  }

  return true;
}

/**
 * Normalizes the vector and returns the normalized vector as a copy.  If the
 * vector was a zero-length vector, a zero length vector will be returned.
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
normalized() const {
  FLOATTYPE l2 = length_squared();
  if (l2 == (FLOATTYPE)0.0f) {
    return FLOATNAME(LVecBase3)(0.0f);
  }
  return (*this) / csqrt(l2);
}

/**
 * Returns a new vector representing the projection of this vector onto
 * another one.  The resulting vector will be a scalar multiple of onto.
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
project(const FLOATNAME(LVecBase3) &onto) const {
  return onto * (dot(onto) / onto.length_squared());
}
#endif  // FLOATTYPE_IS_INT

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
cross(const FLOATNAME(LVecBase3) &other) const {
  TAU_PROFILE("LVecBase3 LVecBase3::cross()", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase3)(_v.cross(other._v));
#else
  return FLOATNAME(LVecBase3)(_v(1) * other._v(2) - other._v(1) * _v(2),
                              other._v(0) * _v(2) - _v(0) * other._v(2),
                              _v(0) * other._v(1) - other._v(0) * _v(1));
#endif  // HAVE_EIGEN
}

/**
 * This performs a lexicographical comparison.  It's of questionable
 * mathematical meaning, but sometimes has a practical purpose for sorting
 * unique vectors, especially in an STL container.  Also see compare_to().
 */
INLINE_LINMATH bool FLOATNAME(LVecBase3)::
operator < (const FLOATNAME(LVecBase3) &other) const {
  TAU_PROFILE("bool LVecBase3::operator <(const LVecBase3 &)", " ", TAU_USER);
  return (compare_to(other) < 0);
}

/**
 *
 */
INLINE_LINMATH bool FLOATNAME(LVecBase3)::
operator == (const FLOATNAME(LVecBase3) &other) const {
  TAU_PROFILE("bool LVecBase3::operator ==(const LVecBase3 &)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return _v == other._v;
#else
  return (_v(0) == other._v(0) &&
          _v(1) == other._v(1) &&
          _v(2) == other._v(2));
#endif  // HAVE_EIGEN
}

/**
 *
 */
INLINE_LINMATH bool FLOATNAME(LVecBase3)::
operator != (const FLOATNAME(LVecBase3) &other) const {
  return !operator == (other);
}

#ifndef FLOATTYPE_IS_INT
/**
 * return value in the range -180.0 to 179.99999. See Also:
 * get_standardized_hpr
 */
static INLINE_LINMATH FLOATTYPE
get_standardized_rotation(FLOATTYPE angle_in_degrees) {
  if (angle_in_degrees<0.0) {
    angle_in_degrees = FLOATCONST(360.0) - fmod(angle_in_degrees * FLOATCONST(-1.0), FLOATCONST(360.0));
  } else {
    angle_in_degrees = fmod(angle_in_degrees, FLOATCONST(360.0));
  }
  // This can be changed to return values in the range 0.0 to 359.99999 by
  // skipping this next part and returning now.

  return (angle_in_degrees<FLOATCONST(180.0))?
      angle_in_degrees:
      angle_in_degrees - FLOATCONST(360.0);
}

/**
 * Try to un-spin the hpr to a standard form.  Like all standards, someone
 * decides between many arbitrary possible standards.  This function assumes
 * that 0 and 360 are the same, as is 720 and -360.  Also 180 and -180 are the
 * same.  Another example is -90 and 270. Each element will be in the range
 * -180.0 to 179.99999. The original usage of this function is for human
 * readable output.
 *
 * It doesn't work so well for asserting that foo_hpr is roughly equal to
 * bar_hpr.  Try using LQuaternionf::is_same_direction() for that.  See Also:
 * get_standardized_rotation, LQuaternion::is_same_direction
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
get_standardized_hpr() const {
  return FLOATNAME(LVecBase3)(
      get_standardized_rotation(_v(0)),
      get_standardized_rotation(_v(1)),
      get_standardized_rotation(_v(2)));
}
#endif  // !FLOATTYPE_IS_INT

/**
 * This flavor of compare_to uses a default threshold value based on the
 * numeric type.
 */
INLINE_LINMATH int FLOATNAME(LVecBase3)::
compare_to(const FLOATNAME(LVecBase3) &other) const {
  TAU_PROFILE("int LVecBase3::compare_to(const LVecBase3 &)", " ", TAU_USER);
#ifdef FLOATTYPE_IS_INT
  if (_v(0) != other._v(0)) {
    return (_v(0) < other._v(0)) ? -1 : 1;
  }
  if (_v(1) != other._v(1)) {
    return (_v(1) < other._v(1)) ? -1 : 1;
  }
  if (_v(2) != other._v(2)) {
    return (_v(2) < other._v(2)) ? -1 : 1;
  }
  return 0;
#else
  return compare_to(other, NEARLY_ZERO(FLOATTYPE));
#endif
}

/**
 * Returns a suitable hash for phash_map.
 */
INLINE_LINMATH size_t FLOATNAME(LVecBase3)::
get_hash() const {
  TAU_PROFILE("size_t LVecBase3::get_hash()", " ", TAU_USER);
  return add_hash(0);
}

/**
 * Adds the vector into the running hash.
 */
INLINE_LINMATH size_t FLOATNAME(LVecBase3)::
add_hash(size_t hash) const {
  TAU_PROFILE("size_t LVecBase3::add_hash(size_t)", " ", TAU_USER);
#ifdef FLOATTYPE_IS_INT
  hash = int_hash::add_hash(hash, _v(0));
  hash = int_hash::add_hash(hash, _v(1));
  hash = int_hash::add_hash(hash, _v(2));
  return hash;
#else
  return add_hash(hash, NEARLY_ZERO(FLOATTYPE));
#endif
}

/**
 * Adds the vector to the indicated hash generator.
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
generate_hash(ChecksumHashGenerator &hashgen) const {
  TAU_PROFILE("LVecBase3::generate_hash(ChecksumHashGenerator &)", " ", TAU_USER);
#ifdef FLOATTYPE_IS_INT
  hashgen.add_int(_v(0));
  hashgen.add_int(_v(1));
  hashgen.add_int(_v(2));
#else
  generate_hash(hashgen, NEARLY_ZERO(FLOATTYPE));
#endif
}

#ifndef FLOATTYPE_IS_INT
/**
 * Sorts vectors lexicographically, componentwise.  Returns a number less than
 * 0 if this vector sorts before the other one, greater than zero if it sorts
 * after, 0 if they are equivalent (within the indicated tolerance).
 */
INLINE_LINMATH int FLOATNAME(LVecBase3)::
compare_to(const FLOATNAME(LVecBase3) &other, FLOATTYPE threshold) const {
  TAU_PROFILE("int LVecBase3::compare_to(const LVecBase3 &, FLOATTYPE)", " ", TAU_USER);
  if (!IS_THRESHOLD_COMPEQ(_v(0), other._v(0), threshold)) {
    return (_v(0) < other._v(0)) ? -1 : 1;
  }
  if (!IS_THRESHOLD_COMPEQ(_v(1), other._v(1), threshold)) {
    return (_v(1) < other._v(1)) ? -1 : 1;
  }
  if (!IS_THRESHOLD_COMPEQ(_v(2), other._v(2), threshold)) {
    return (_v(2) < other._v(2)) ? -1 : 1;
  }
  return 0;
}

/**
 * Returns a suitable hash for phash_map.
 */
INLINE_LINMATH size_t FLOATNAME(LVecBase3)::
get_hash(FLOATTYPE threshold) const {
  TAU_PROFILE("size_t LVecBase3::get_hash(FLOATTYPE)", " ", TAU_USER);
  return add_hash(0, threshold);
}

/**
 * Adds the vector into the running hash.
 */
INLINE_LINMATH size_t FLOATNAME(LVecBase3)::
add_hash(size_t hash, FLOATTYPE threshold) const {
  TAU_PROFILE("LVecBase3::add_hash(size_t, FLOATTYPE)", " ", TAU_USER);
  float_hash fhasher(threshold);
  hash = fhasher.add_hash(hash, _v(0));
  hash = fhasher.add_hash(hash, _v(1));
  hash = fhasher.add_hash(hash, _v(2));
  return hash;
}

/**
 * Adds the vector to the indicated hash generator.
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
generate_hash(ChecksumHashGenerator &hashgen, FLOATTYPE threshold) const {
  TAU_PROFILE("LVecBase3::generate_hash(ChecksumHashGenerator &, FLOATTYPE)", " ", TAU_USER);
  hashgen.add_fp(_v(0), threshold);
  hashgen.add_fp(_v(1), threshold);
  hashgen.add_fp(_v(2), threshold);
}
#endif  // FLOATTYPE_IS_INT

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
operator - () const {
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase3)(-_v);
#else
  return FLOATNAME(LVecBase3)(-_v(0), -_v(1), -_v(2));
#endif  // HAVE_EIGEN
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
operator + (const FLOATNAME(LVecBase3) &other) const {
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase3)(_v + other._v);
#else
  return FLOATNAME(LVecBase3)(_v(0) + other._v(0),
                              _v(1) + other._v(1),
                              _v(2) + other._v(2));
#endif  // HAVE_EIGEN
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
operator - (const FLOATNAME(LVecBase3) &other) const {
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase3)(_v - other._v);
#else
  return FLOATNAME(LVecBase3)(_v(0) - other._v(0),
                              _v(1) - other._v(1),
                              _v(2) - other._v(2));
#endif  // HAVE_EIGEN
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
operator * (FLOATTYPE scalar) const {
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase3)(_v * scalar);
#else
  return FLOATNAME(LVecBase3)(_v(0) * scalar,
                              _v(1) * scalar,
                              _v(2) * scalar);
#endif  // HAVE_EIGEN
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
operator / (FLOATTYPE scalar) const {
#ifdef FLOATTYPE_IS_INT
  return FLOATNAME(LVecBase3)(_v(0) / scalar,
                              _v(1) / scalar,
                              _v(2) / scalar);
#else
  FLOATTYPE recip_scalar = 1.0f/scalar;
  return operator * (recip_scalar);
#endif
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
operator += (const FLOATNAME(LVecBase3) &other) {
#ifdef HAVE_EIGEN
  _v += other._v;
#else
  _v(0) += other._v(0);
  _v(1) += other._v(1);
  _v(2) += other._v(2);
#endif  // HAVE_EIGEN
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
operator -= (const FLOATNAME(LVecBase3) &other) {
#ifdef HAVE_EIGEN
  _v -= other._v;
#else
  _v(0) -= other._v(0);
  _v(1) -= other._v(1);
  _v(2) -= other._v(2);
#endif  // HAVE_EIGEN
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
operator *= (FLOATTYPE scalar) {
#ifdef HAVE_EIGEN
  _v *= scalar;
#else
  _v(0) *= scalar;
  _v(1) *= scalar;
  _v(2) *= scalar;
#endif  // HAVE_EIGEN
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
operator /= (FLOATTYPE scalar) {
#ifdef FLOATTYPE_IS_INT
  _v(0) /= scalar;
  _v(1) /= scalar;
  _v(2) /= scalar;
#else
  FLOATTYPE recip_scalar = 1.0f/scalar;
  operator *= (recip_scalar);
#endif
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
componentwise_mult(const FLOATNAME(LVecBase3) &other) {
#ifdef HAVE_EIGEN
  _v = _v.cwiseProduct(other._v);
#else
  _v(0) *= other._v(0);
  _v(1) *= other._v(1);
  _v(2) *= other._v(2);
#endif  // HAVE_EIGEN
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
fmax(const FLOATNAME(LVecBase3) &other) const {
  TAU_PROFILE("LVecBase3::fmax()", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase3)(_v.cwiseMax(other._v));
#else
  return FLOATNAME(LVecBase3)(_v(0) > other._v(0) ? _v(0) : other._v(0),
                              _v(1) > other._v(1) ? _v(1) : other._v(1),
                              _v(2) > other._v(2) ? _v(2) : other._v(2));
#endif
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
fmin(const FLOATNAME(LVecBase3) &other) const {
  TAU_PROFILE("LVecBase3::fmin()", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase3)(_v.cwiseMin(other._v));
#else
  return FLOATNAME(LVecBase3)(_v(0) < other._v(0) ? _v(0) : other._v(0),
                              _v(1) < other._v(1) ? _v(1) : other._v(1),
                              _v(2) < other._v(2) ? _v(2) : other._v(2));
#endif
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
cross_into(const FLOATNAME(LVecBase3) &other) {
  (*this) = cross(other);
}

/**
 * Returns true if two vectors are memberwise equal within a specified
 * tolerance.
 */
INLINE_LINMATH bool FLOATNAME(LVecBase3)::
almost_equal(const FLOATNAME(LVecBase3) &other, FLOATTYPE threshold) const {
  TAU_PROFILE("bool LVecBase3::almost_equal(LVecBase3 &, FLOATTYPE)", " ", TAU_USER);
  return (IS_THRESHOLD_EQUAL(_v(0), other._v(0), threshold) &&
          IS_THRESHOLD_EQUAL(_v(1), other._v(1), threshold) &&
          IS_THRESHOLD_EQUAL(_v(2), other._v(2), threshold));
}

/**
 * Returns true if two vectors are memberwise equal within a default tolerance
 * based on the numeric type.
 */
INLINE_LINMATH bool FLOATNAME(LVecBase3)::
almost_equal(const FLOATNAME(LVecBase3) &other) const {
  TAU_PROFILE("bool LVecBase3::almost_equal(LVecBase3 &)", " ", TAU_USER);
  return almost_equal(other, NEARLY_ZERO(FLOATTYPE));
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
output(std::ostream &out) const {
  out << MAYBE_ZERO(_v(0)) << " "
      << MAYBE_ZERO(_v(1)) << " "
      << MAYBE_ZERO(_v(2));
}

/**
 * Writes the vector to the Datagram using add_float32() or add_float64(),
 * depending on the type of floats in the vector, regardless of the setting of
 * Datagram::set_stdfloat_double().  This is appropriate when you want to
 * write a fixed-width value to the datagram, especially when you are not
 * writing a bam file.
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
write_datagram_fixed(Datagram &destination) const {
#if FLOATTOKEN == 'i'
  destination.add_int32(_v(0));
  destination.add_int32(_v(1));
  destination.add_int32(_v(2));
#elif FLOATTOKEN == 'f'
  destination.add_float32(_v(0));
  destination.add_float32(_v(1));
  destination.add_float32(_v(2));
#else
  destination.add_float64(_v(0));
  destination.add_float64(_v(1));
  destination.add_float64(_v(2));
#endif
}

/**
 * Reads the vector from the Datagram using get_float32() or get_float64().
 * See write_datagram_fixed().
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
read_datagram_fixed(DatagramIterator &source) {
#if FLOATTOKEN == 'i'
  _v(0) = source.get_int32();
  _v(1) = source.get_int32();
  _v(2) = source.get_int32();
#elif FLOATTOKEN == 'f'
  _v(0) = source.get_float32();
  _v(1) = source.get_float32();
  _v(2) = source.get_float32();
#else
  _v(0) = source.get_float64();
  _v(1) = source.get_float64();
  _v(2) = source.get_float64();
#endif
}

/**
 * Writes the vector to the Datagram using add_stdfloat().  This is
 * appropriate when you want to write the vector using the standard width
 * setting, especially when you are writing a bam file.
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
write_datagram(Datagram &destination) const {
#if FLOATTOKEN == 'i'
  destination.add_int32(_v(0));
  destination.add_int32(_v(1));
  destination.add_int32(_v(2));
#else
  destination.add_stdfloat(_v(0));
  destination.add_stdfloat(_v(1));
  destination.add_stdfloat(_v(2));
#endif
}

/**
 * Reads the vector from the Datagram using get_stdfloat().
 */
INLINE_LINMATH void FLOATNAME(LVecBase3)::
read_datagram(DatagramIterator &source) {
#if FLOATTOKEN == 'i'
  _v(0) = source.get_int32();
  _v(1) = source.get_int32();
  _v(2) = source.get_int32();
#else
  _v(0) = source.get_stdfloat();
  _v(1) = source.get_stdfloat();
  _v(2) = source.get_stdfloat();
#endif
}
