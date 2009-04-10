// Filename: lvecBase4_src.I
// Created by:  drose (08Mar00)
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
//     Function: LVecBase4::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::
FLOATNAME(LVecBase4)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::
FLOATNAME(LVecBase4)(const FLOATNAME(LVecBase4) &copy) {
  TAU_PROFILE("LVecBase4::LVecBase4(LVecBase4 &)", " ", TAU_USER);
  _v.v._0 = copy._v.v._0;
  _v.v._1 = copy._v.v._1;
  _v.v._2 = copy._v.v._2;
  _v.v._3 = copy._v.v._3;
//  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
operator = (const FLOATNAME(LVecBase4) &copy) {
  TAU_PROFILE("void LVecBase4::operator = (LVecBase4 &)", " ", TAU_USER);
  _v.v._0 = copy._v.v._0;
  _v.v._1 = copy._v.v._1;
  _v.v._2 = copy._v.v._2;
  _v.v._3 = copy._v.v._3;
//  set(copy[0], copy[1], copy[2], copy[3]);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Fill Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
operator = (FLOATTYPE fill_value) {
  fill(fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::
FLOATNAME(LVecBase4)(FLOATTYPE fill_value) {
  fill(fill_value);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::
FLOATNAME(LVecBase4)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w) {
  TAU_PROFILE("void LVecBase4::operator = (FLOATTYPE, ...)", " ", TAU_USER);
  _v.v._0 = x;
  _v.v._1 = y;
  _v.v._2 = z;
  _v.v._3 = w;
//  set(x, y, z, w);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::
~FLOATNAME(LVecBase4)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
zero() {
  return _zero;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
unit_x() {
  return _unit_x;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
unit_y() {
  return _unit_y;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::unit_z Named Constructor
//       Access: Public
//  Description: Returns a unit Z vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
unit_z() {
  return _unit_z;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::unit_w Named Constructor
//       Access: Public
//  Description: Returns a unit W vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
unit_w() {
  return _unit_w;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Indexing Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
operator [](int i) const {
  nassertr(i >= 0 && i < 4, 0.0);
  return _v.data[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Indexing Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE &FLOATNAME(LVecBase4)::
operator [](int i) {
  nassertr(i >= 0 && i < 4, _v.data[0]);
  return _v.data[i];
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::__setitem__
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
__setitem__(int i, FLOATTYPE v) {
  nassertv(i >= 0 && i < 4);
  _v.data[i] = v;
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::size
//       Access: Public, Static
//  Description: Returns 4: the number of components of a LVecBase4.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LVecBase4)::
size() {
  return 4;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::is_nan
//       Access: Public
//  Description: Returns true if any component of the vector is
//               not-a-number, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase4)::
is_nan() const {
  TAU_PROFILE("bool LVecBase4::is_nan()", " ", TAU_USER);
  return cnan(_v.v._0) || cnan(_v.v._1) || cnan(_v.v._2) || cnan(_v.v._3);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_cell
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
get_cell(int i) const {
  nassertr(i >= 0 && i < 4, 0.0);
  return _v.data[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_x
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
get_x() const {
  return _v.v._0;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_y
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
get_y() const {
  return _v.v._1;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_z
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
get_z() const {
  return _v.v._2;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_w
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
get_w() const {
  return _v.v._3;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set_cell
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
set_cell(int i, FLOATTYPE value) {
  nassertv(i >= 0 && i < 4);
  _v.data[i] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set_x
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
set_x(FLOATTYPE value) {
  _v.v._0 = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set_y
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
set_y(FLOATTYPE value) {
  _v.v._1 = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set_z
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
set_z(FLOATTYPE value) {
  _v.v._2 = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set_w
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
set_w(FLOATTYPE value) {
  _v.v._3 = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::add_to_cell
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
add_to_cell(int i, FLOATTYPE value) {
  nassertv(i >= 0 && i < 4);
  _v.data[i] += value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::add_x
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
add_x(FLOATTYPE value) {
  _v.v._0 += value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::add_y
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
add_y(FLOATTYPE value) {
  _v.v._1 += value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::add_z
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
add_z(FLOATTYPE value) {
  _v.v._2 += value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::add_w
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
add_w(FLOATTYPE value) {
  _v.v._3 += value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_data
//       Access: Public
//  Description: Returns the address of the first of the four data
//               elements in the vector.  The remaining elements
//               occupy the next positions consecutively in memory.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATTYPE *FLOATNAME(LVecBase4)::
get_data() const {
  return _v.data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_num_components
//       Access: Public
//  Description: Returns the number of elements in the vector, four.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LVecBase4)::
get_num_components() const {
  return 4;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::iterator FLOATNAME(LVecBase4)::
begin() {
  return _v.data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::iterator FLOATNAME(LVecBase4)::
end() {
  return begin() + get_num_components();
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::const_iterator FLOATNAME(LVecBase4)::
begin() const {
  return _v.data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::const_iterator FLOATNAME(LVecBase4)::
end() const {
  return begin() + get_num_components();
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::fill
//       Access: Public
//  Description: Sets each element of the vector to the indicated
//               fill_value.  This is particularly useful for
//               initializing to zero.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
fill(FLOATTYPE fill_value) {
  TAU_PROFILE("void LVecBase4::fill()", " ", TAU_USER);
  _v.v._0 = fill_value;
  _v.v._1 = fill_value;
  _v.v._2 = fill_value;
  _v.v._3 = fill_value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
set(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w) {
  TAU_PROFILE("void LVecBase4::set()", " ", TAU_USER);
  _v.v._0 = x;
  _v.v._1 = y;
  _v.v._2 = z;
  _v.v._3 = w;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::length
//       Access: Public
//  Description: Returns the length of the vector, by the Pythagorean
//               theorem.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
length() const {
  return csqrt((*this).dot(*this));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::length_squared
//       Access: Public
//  Description: Returns the square of the vector's length, cheap and
//               easy.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
length_squared() const {
  return (*this).dot(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::normalize
//       Access: Public
//  Description: Normalizes the vector in place.  Returns true if the
//               vector was normalized, false if it was a zero-length
//               vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase4)::
normalize() {
  FLOATTYPE l2 = length_squared();
  if (l2 == (FLOATTYPE)0.0f) {
    set(0.0f, 0.0f, 0.0f, 0.0f);
    return false;

  } else if (!IS_THRESHOLD_EQUAL(l2, 1.0f, NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE))) {
    (*this) /= csqrt(l2);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::dot
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
dot(const FLOATNAME(LVecBase4) &other) const {
  TAU_PROFILE("FLOATTYPE LVecBase4::dot()", " ", TAU_USER);
  return
    _v.v._0 * other._v.v._0 + _v.v._1 * other._v.v._1 +
    _v.v._2 * other._v.v._2 + _v.v._3 * other._v.v._3;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::project
//       Access: Published
//  Description: Returns a new vector representing the projection of
//               this vector onto another one.  The resulting vector
//               will be a scalar multiple of onto.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
project(const FLOATNAME(LVecBase4) &onto) const {
  return onto * (dot(onto) / onto.length_squared());
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator <
//       Access: Public
//  Description: This performs a lexicographical comparison.  It's of
//               questionable mathematical meaning, but sometimes has
//               a practical purpose for sorting unique vectors,
//               especially in an STL container.  Also see
//               compare_to().
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase4)::
operator < (const FLOATNAME(LVecBase4) &other) const {
  TAU_PROFILE("bool LVecBase4::operator <(const LVecBase4 &)", " ", TAU_USER);
  return (compare_to(other) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator ==
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase4)::
operator == (const FLOATNAME(LVecBase4) &other) const {
  TAU_PROFILE("bool LVecBase4::operator ==(const LVecBase4 &)", " ", TAU_USER);
  return (_v.v._0 == other._v.v._0 &&
          _v.v._1 == other._v.v._1 &&
          _v.v._2 == other._v.v._2 &&
          _v.v._3 == other._v.v._3);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator !=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase4)::
operator != (const FLOATNAME(LVecBase4) &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::compare_to
//       Access: Public
//  Description: This flavor of compare_to uses a default threshold
//               value based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LVecBase4)::
compare_to(const FLOATNAME(LVecBase4) &other) const {
  TAU_PROFILE("int LVecBase4::compare_to(const LVecBase4 &)", " ", TAU_USER);
  return compare_to(other, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::compare_to
//       Access: Public
//  Description: Sorts vectors lexicographically, componentwise.
//               Returns a number less than 0 if this vector sorts
//               before the other one, greater than zero if it sorts
//               after, 0 if they are equivalent (within the indicated
//               tolerance).
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LVecBase4)::
compare_to(const FLOATNAME(LVecBase4) &other, FLOATTYPE threshold) const {
  TAU_PROFILE("int LVecBase4::compare_to(const LVecBase4 &, FLOATTYPE)", " ", TAU_USER);
  if (!IS_THRESHOLD_COMPEQ(_v.v._0, other._v.v._0, threshold)) {
    return (_v.v._0 < other._v.v._0) ? -1 : 1;
  }
  if (!IS_THRESHOLD_COMPEQ(_v.v._1, other._v.v._1, threshold)) {
    return (_v.v._1 < other._v.v._1) ? -1 : 1;
  }
  if (!IS_THRESHOLD_COMPEQ(_v.v._2, other._v.v._2, threshold)) {
    return (_v.v._2 < other._v.v._2) ? -1 : 1;
  }
  if (!IS_THRESHOLD_COMPEQ(_v.v._3, other._v.v._3, threshold)) {
    return (_v.v._3 < other._v.v._3) ? -1 : 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_hash
//       Access: Public
//  Description: Returns a suitable hash for phash_map.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH size_t FLOATNAME(LVecBase4)::
get_hash() const {
  TAU_PROFILE("size_t LVecBase4::get_hash()", " ", TAU_USER);
  return add_hash(0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_hash
//       Access: Public
//  Description: Returns a suitable hash for phash_map.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH size_t FLOATNAME(LVecBase4)::
get_hash(FLOATTYPE threshold) const {
  TAU_PROFILE("size_t LVecBase4::get_hash(FLOATTYPE)", " ", TAU_USER);
  return add_hash(0, threshold);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::add_hash
//       Access: Public
//  Description: Adds the vector into the running hash.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH size_t FLOATNAME(LVecBase4)::
add_hash(size_t hash) const {
  TAU_PROFILE("size_t LVecBase4::add_hash(size_t)", " ", TAU_USER);
  return add_hash(hash, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::add_hash
//       Access: Public
//  Description: Adds the vector into the running hash.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH size_t FLOATNAME(LVecBase4)::
add_hash(size_t hash, FLOATTYPE threshold) const {
  TAU_PROFILE("LVecBase4::add_hash(size_t, FLOATTYPE)", " ", TAU_USER);
  float_hash fhasher(threshold);
  hash = fhasher.add_hash(hash, _v.v._0);
  hash = fhasher.add_hash(hash, _v.v._1);
  hash = fhasher.add_hash(hash, _v.v._2);
  hash = fhasher.add_hash(hash, _v.v._3);
  return hash;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::unary -
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
operator - () const {
  return FLOATNAME(LVecBase4)(-_v.v._0, -_v.v._1, -_v.v._2, -_v.v._3);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::vector + vector
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
operator + (const FLOATNAME(LVecBase4) &other) const {
  return FLOATNAME(LVecBase4)(_v.v._0 + other._v.v._0,
                              _v.v._1 + other._v.v._1,
                              _v.v._2 + other._v.v._2,
                              _v.v._3 + other._v.v._3);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::vector - vector
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
operator - (const FLOATNAME(LVecBase4) &other) const {
  return FLOATNAME(LVecBase4)(_v.v._0 - other._v.v._0,
                              _v.v._1 - other._v.v._1,
                              _v.v._2 - other._v.v._2,
                              _v.v._3 - other._v.v._3);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::vector * scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LVecBase4)(_v.v._0 * scalar,
                              _v.v._1 * scalar,
                              _v.v._2 * scalar,
                              _v.v._3 * scalar);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::vector / scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
operator / (FLOATTYPE scalar) const {
  FLOATTYPE recip_scalar = 1.0f/scalar;
  return FLOATNAME(LVecBase4)(_v.v._0 * recip_scalar,
                              _v.v._1 * recip_scalar,
                              _v.v._2 * recip_scalar,
                              _v.v._3 * recip_scalar);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator +=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
operator += (const FLOATNAME(LVecBase4) &other) {
  _v.v._0 += other._v.v._0;
  _v.v._1 += other._v.v._1;
  _v.v._2 += other._v.v._2;
  _v.v._3 += other._v.v._3;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator -=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
operator -= (const FLOATNAME(LVecBase4) &other) {
  _v.v._0 -= other._v.v._0;
  _v.v._1 -= other._v.v._1;
  _v.v._2 -= other._v.v._2;
  _v.v._3 -= other._v.v._3;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator *=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
operator *= (FLOATTYPE scalar) {
  _v.v._0 *= scalar;
  _v.v._1 *= scalar;
  _v.v._2 *= scalar;
  _v.v._3 *= scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator /=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
operator /= (FLOATTYPE scalar) {
  FLOATTYPE recip_scalar = 1.0f/scalar;
  _v.v._0 *= recip_scalar;
  _v.v._1 *= recip_scalar;
  _v.v._2 *= recip_scalar;
  _v.v._3 *= recip_scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::fmax
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
fmax(const FLOATNAME(LVecBase4) &other) {
  TAU_PROFILE("LVecBase4::fmax()", " ", TAU_USER);
  return FLOATNAME(LVecBase4)(_v.v._0 > other._v.v._0 ? _v.v._0 : other._v.v._0,
                              _v.v._1 > other._v.v._1 ? _v.v._1 : other._v.v._1,
                              _v.v._2 > other._v.v._2 ? _v.v._2 : other._v.v._2,
                              _v.v._3 > other._v.v._3 ? _v.v._3 : other._v.v._3);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::fmin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
fmin(const FLOATNAME(LVecBase4) &other) {
  TAU_PROFILE("LVecBase4::fmin()", " ", TAU_USER);
  return FLOATNAME(LVecBase4)(_v.v._0 < other._v.v._0 ? _v.v._0 : other._v.v._0,
                              _v.v._1 < other._v.v._1 ? _v.v._1 : other._v.v._1,
                              _v.v._2 < other._v.v._2 ? _v.v._2 : other._v.v._2,
                              _v.v._3 < other._v.v._3 ? _v.v._3 : other._v.v._3);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::almost_equal
//       Access: Public
//  Description: Returns true if two vectors are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase4)::
almost_equal(const FLOATNAME(LVecBase4) &other, FLOATTYPE threshold) const {
  TAU_PROFILE("bool LVecBase4::almost_equal(LVecBase4 &, FLOATTYPE)", " ", TAU_USER);
  return (IS_THRESHOLD_EQUAL(_v.v._0, other._v.v._0, threshold) &&
          IS_THRESHOLD_EQUAL(_v.v._1, other._v.v._1, threshold) &&
          IS_THRESHOLD_EQUAL(_v.v._2, other._v.v._2, threshold) &&
          IS_THRESHOLD_EQUAL(_v.v._3, other._v.v._3, threshold));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::almost_equal
//       Access: Public
//  Description: Returns true if two vectors are memberwise equal
//               within a default tolerance based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase4)::
almost_equal(const FLOATNAME(LVecBase4) &other) const {
  TAU_PROFILE("bool LVecBase4::almost_equal(LVecBase4 &)", " ", TAU_USER);
  return almost_equal(other, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
output(ostream &out) const {
  out << MAYBE_ZERO(_v.v._0) << " "
      << MAYBE_ZERO(_v.v._1) << " "
      << MAYBE_ZERO(_v.v._2) << " "
      << MAYBE_ZERO(_v.v._3);
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::python_repr
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
python_repr(ostream &out, const string &class_name) const {
  out << class_name << "(" 
      << MAYBE_ZERO(_v.v._0) << ", "
      << MAYBE_ZERO(_v.v._1) << ", "
      << MAYBE_ZERO(_v.v._2) << ", "
      << MAYBE_ZERO(_v.v._3) << ")";
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::generate_hash
//       Access: Public
//  Description: Adds the vector to the indicated hash generator.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
generate_hash(ChecksumHashGenerator &hashgen) const {
  TAU_PROFILE("LVecBase4::generate_hash(ChecksumHashGenerator &)", " ", TAU_USER);
  generate_hash(hashgen, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::generate_hash
//       Access: Public
//  Description: Adds the vector to the indicated hash generator.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
generate_hash(ChecksumHashGenerator &hashgen, FLOATTYPE threshold) const {
  TAU_PROFILE("LVecBase4::generate_hash(ChecksumHashGenerator &, FLOATTYPE)", " ", TAU_USER);
  hashgen.add_fp(_v.v._0, threshold);
  hashgen.add_fp(_v.v._1, threshold);
  hashgen.add_fp(_v.v._2, threshold);
  hashgen.add_fp(_v.v._3, threshold);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::write_datagram
//       Access: Public
//  Description: Function to write itself into a datagram
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
write_datagram(Datagram &destination) const {
#if FLOATTOKEN == 'f'
  destination.add_float32(_v.v._0);
  destination.add_float32(_v.v._1);
  destination.add_float32(_v.v._2);
  destination.add_float32(_v.v._3);
#else
  destination.add_float64(_v.v._0);
  destination.add_float64(_v.v._1);
  destination.add_float64(_v.v._2);
  destination.add_float64(_v.v._3);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::read_datagram
//       Access: Public
//  Description: Function to read itself from a datagramIterator
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
read_datagram(DatagramIterator &source) {
#if FLOATTOKEN == 'f'
  _v.v._0 = source.get_float32();
  _v.v._1 = source.get_float32();
  _v.v._2 = source.get_float32();
  _v.v._3 = source.get_float32();
#else
  _v.v._0 = source.get_float64();
  _v.v._1 = source.get_float64();
  _v.v._2 = source.get_float64();
  _v.v._3 = source.get_float64();
#endif
}
