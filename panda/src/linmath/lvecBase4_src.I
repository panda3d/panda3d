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
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::
FLOATNAME(LVecBase4)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::
FLOATNAME(LVecBase4)(const FLOATNAME(LVecBase4) &copy) : _v(copy._v) {
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::
FLOATNAME(LVecBase4)(const FLOATNAME(UnalignedLVecBase4) &copy) {
  set(copy[0], copy[1], copy[2], copy[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
operator = (const FLOATNAME(LVecBase4) &copy) {
  TAU_PROFILE("void LVecBase4::operator = (LVecBase4 &)", " ", TAU_USER);
  _v = copy._v;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
operator = (const FLOATNAME(UnalignedLVecBase4) &copy) {
  set(copy[0], copy[1], copy[2], copy[3]);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Fill Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
operator = (FLOATTYPE fill_value) {
  fill(fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::
FLOATNAME(LVecBase4)(FLOATTYPE fill_value) {
  fill(fill_value);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::
FLOATNAME(LVecBase4)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w) {
  TAU_PROFILE("LVecBase4::LVecBase4(FLOATTYPE, ...)", " ", TAU_USER);
  set(x, y, z, w);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::
FLOATNAME(LVecBase4)(const FLOATNAME(LVecBase3) &copy, FLOATTYPE w) {
  set(copy[0], copy[1], copy[2], w);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Constructor
//       Access: Published
//  Description: Constructs an LVecBase4 from an LPoint3.  The w
//               coordinate is set to 1.0.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::
FLOATNAME(LVecBase4)(const FLOATNAME(LPoint3) &point) {
  set(point[0], point[1], point[2], 1);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Constructor
//       Access: Published
//  Description: Constructs an LVecBase4 from an LVector3.  The w
//               coordinate is set to 0.0.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::
FLOATNAME(LVecBase4)(const FLOATNAME(LVector3) &vector) {
  set(vector[0], vector[1], vector[2], 0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::
~FLOATNAME(LVecBase4)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::zero Named Constructor
//       Access: Published
//  Description: Returns a zero-length vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
zero() {
  return _zero;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::unit_x Named Constructor
//       Access: Published
//  Description: Returns a unit X vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
unit_x() {
  return _unit_x;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::unit_y Named Constructor
//       Access: Published
//  Description: Returns a unit Y vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
unit_y() {
  return _unit_y;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::unit_z Named Constructor
//       Access: Published
//  Description: Returns a unit Z vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
unit_z() {
  return _unit_z;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::unit_w Named Constructor
//       Access: Published
//  Description: Returns a unit W vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
unit_w() {
  return _unit_w;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Indexing Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
operator [](int i) const {
  nassertr(i >= 0 && i < 4, 0);
  return _v(i);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Indexing Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE &FLOATNAME(LVecBase4)::
operator [](int i) {
  nassertr(i >= 0 && i < 4, _v(0));
  return _v(i);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::size
//       Access: Published, Static
//  Description: Returns 4: the number of components of a LVecBase4.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LVecBase4)::
size() {
  return 4;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::is_nan
//       Access: Published
//  Description: Returns true if any component of the vector is
//               not-a-number, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase4)::
is_nan() const {
#ifdef FLOATTYPE_IS_INT
  return false;
#else
  TAU_PROFILE("bool LVecBase4::is_nan()", " ", TAU_USER);
  return cnan(_v(0)) || cnan(_v(1)) || cnan(_v(2)) || cnan(_v(3));
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_cell
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
get_cell(int i) const {
  nassertr(i >= 0 && i < 4, 0);
  return _v(i);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_x
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
get_x() const {
  return _v(0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_y
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
get_y() const {
  return _v(1);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_z
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
get_z() const {
  return _v(2);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_w
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
get_w() const {
  return _v(3);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set_cell
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
set_cell(int i, FLOATTYPE value) {
  nassertv(i >= 0 && i < 4);
  _v(i) = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set_x
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
set_x(FLOATTYPE value) {
  _v(0) = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set_y
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
set_y(FLOATTYPE value) {
  _v(1) = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set_z
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
set_z(FLOATTYPE value) {
  _v(2) = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set_w
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
set_w(FLOATTYPE value) {
  _v(3) = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::add_to_cell
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
add_to_cell(int i, FLOATTYPE value) {
  nassertv(i >= 0 && i < 4);
  _v(i) += value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::add_x
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
add_x(FLOATTYPE value) {
  _v(0) += value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::add_y
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
add_y(FLOATTYPE value) {
  _v(1) += value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::add_z
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
add_z(FLOATTYPE value) {
  _v(2) += value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::add_w
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
add_w(FLOATTYPE value) {
  _v(3) += value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_data
//       Access: Published
//  Description: Returns the address of the first of the four data
//               elements in the vector.  The remaining elements
//               occupy the next positions consecutively in memory.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATTYPE *FLOATNAME(LVecBase4)::
get_data() const {
  return &_v(0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_num_components
//       Access: Published
//  Description: Returns the number of elements in the vector, four.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LVecBase4)::
get_num_components() const {
  return 4;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::begin
//       Access: Published
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::iterator FLOATNAME(LVecBase4)::
begin() {
  return &_v(0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::end
//       Access: Published
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::iterator FLOATNAME(LVecBase4)::
end() {
  return begin() + num_components;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::begin
//       Access: Published
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::const_iterator FLOATNAME(LVecBase4)::
begin() const {
  return &_v(0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::end
//       Access: Published
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4)::const_iterator FLOATNAME(LVecBase4)::
end() const {
  return begin() + num_components;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::fill
//       Access: Published
//  Description: Sets each element of the vector to the indicated
//               fill_value.  This is particularly useful for
//               initializing to zero.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
fill(FLOATTYPE fill_value) {
  TAU_PROFILE("void LVecBase4::fill()", " ", TAU_USER);
#ifdef HAVE_EIGEN
  _v = EVector4::Constant(fill_value);
#else
  _v(0) = fill_value;
  _v(1) = fill_value;
  _v(2) = fill_value;
  _v(3) = fill_value;
#endif  // HAVE_EIGEN
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
set(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w) {
  TAU_PROFILE("void LVecBase4::set()", " ", TAU_USER);
  _v(0) = x;
  _v(1) = y;
  _v(2) = z;
  _v(3) = w;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::dot
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
dot(const FLOATNAME(LVecBase4) &other) const {
  TAU_PROFILE("FLOATTYPE LVecBase4::dot()", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return _v.dot(other._v);
#else
  return
    _v(0) * other._v(0) + _v(1) * other._v(1) +
    _v(2) * other._v(2) + _v(3) * other._v(3);
#endif  // HAVE_EIGEN
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::length_squared
//       Access: Published
//  Description: Returns the square of the vector's length, cheap and
//               easy.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
length_squared() const {
  TAU_PROFILE("FLOATTYPE LVecBase4::length_squared()", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return _v.squaredNorm();
#else
  return (*this).dot(*this);
#endif  // HAVE_EIGEN
}

#ifndef FLOATTYPE_IS_INT
////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::length
//       Access: Published
//  Description: Returns the length of the vector, by the Pythagorean
//               theorem.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
length() const {
  TAU_PROFILE("FLOATTYPE LVecBase4::length()", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return _v.norm();
#else
  return csqrt((*this).dot(*this));
#endif  // HAVE_EIGEN
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::normalize
//       Access: Published
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
#endif  // FLOATTYPE_IS_INT

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator <
//       Access: Published
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
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase4)::
operator == (const FLOATNAME(LVecBase4) &other) const {
  TAU_PROFILE("bool LVecBase4::operator ==(const LVecBase4 &)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return _v == other._v;
#else
  return (_v(0) == other._v(0) &&
          _v(1) == other._v(1) &&
          _v(2) == other._v(2) &&
          _v(3) == other._v(3));
#endif  // HAVE_EIGEN
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator !=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase4)::
operator != (const FLOATNAME(LVecBase4) &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::compare_to
//       Access: Published
//  Description: This flavor of compare_to uses a default threshold
//               value based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LVecBase4)::
compare_to(const FLOATNAME(LVecBase4) &other) const {
  TAU_PROFILE("int LVecBase4::compare_to(const LVecBase4 &)", " ", TAU_USER);
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
  if (_v(3) != other._v(3)) {
    return (_v(3) < other._v(3)) ? -1 : 1;
  }
  return 0;
#else
  return compare_to(other, NEARLY_ZERO(FLOATTYPE));
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_hash
//       Access: Published
//  Description: Returns a suitable hash for phash_map.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH size_t FLOATNAME(LVecBase4)::
get_hash() const {
  TAU_PROFILE("size_t LVecBase4::get_hash()", " ", TAU_USER);
  return add_hash(0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::add_hash
//       Access: Published
//  Description: Adds the vector into the running hash.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH size_t FLOATNAME(LVecBase4)::
add_hash(size_t hash) const {
  TAU_PROFILE("size_t LVecBase4::add_hash(size_t)", " ", TAU_USER);
#ifdef FLOATTYPE_IS_INT
  int_hash ihasher;
  hash = ihasher.add_hash(hash, _v(0));
  hash = ihasher.add_hash(hash, _v(1));
  hash = ihasher.add_hash(hash, _v(2));
  hash = ihasher.add_hash(hash, _v(3));
  return hash;
#else
  return add_hash(hash, NEARLY_ZERO(FLOATTYPE));
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::generate_hash
//       Access: Published
//  Description: Adds the vector to the indicated hash generator.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
generate_hash(ChecksumHashGenerator &hashgen) const {
  TAU_PROFILE("LVecBase4::generate_hash(ChecksumHashGenerator &)", " ", TAU_USER);
#ifdef FLOATTYPE_IS_INT
  hashgen.add_int(_v(0));
  hashgen.add_int(_v(1));
  hashgen.add_int(_v(2));
  hashgen.add_int(_v(3));
#else
  generate_hash(hashgen, NEARLY_ZERO(FLOATTYPE));
#endif
}

#ifndef FLOATTYPE_IS_INT
////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::compare_to
//       Access: Published
//  Description: Sorts vectors lexicographically, componentwise.
//               Returns a number less than 0 if this vector sorts
//               before the other one, greater than zero if it sorts
//               after, 0 if they are equivalent (within the indicated
//               tolerance).
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LVecBase4)::
compare_to(const FLOATNAME(LVecBase4) &other, FLOATTYPE threshold) const {
  TAU_PROFILE("int LVecBase4::compare_to(const LVecBase4 &, FLOATTYPE)", " ", TAU_USER);
  if (!IS_THRESHOLD_COMPEQ(_v(0), other._v(0), threshold)) {
    return (_v(0) < other._v(0)) ? -1 : 1;
  }
  if (!IS_THRESHOLD_COMPEQ(_v(1), other._v(1), threshold)) {
    return (_v(1) < other._v(1)) ? -1 : 1;
  }
  if (!IS_THRESHOLD_COMPEQ(_v(2), other._v(2), threshold)) {
    return (_v(2) < other._v(2)) ? -1 : 1;
  }
  if (!IS_THRESHOLD_COMPEQ(_v(3), other._v(3), threshold)) {
    return (_v(3) < other._v(3)) ? -1 : 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_hash
//       Access: Published
//  Description: Returns a suitable hash for phash_map.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH size_t FLOATNAME(LVecBase4)::
get_hash(FLOATTYPE threshold) const {
  TAU_PROFILE("size_t LVecBase4::get_hash(FLOATTYPE)", " ", TAU_USER);
  return add_hash(0, threshold);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::add_hash
//       Access: Published
//  Description: Adds the vector into the running hash.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH size_t FLOATNAME(LVecBase4)::
add_hash(size_t hash, FLOATTYPE threshold) const {
  TAU_PROFILE("LVecBase4::add_hash(size_t, FLOATTYPE)", " ", TAU_USER);
  float_hash fhasher(threshold);
  hash = fhasher.add_hash(hash, _v(0));
  hash = fhasher.add_hash(hash, _v(1));
  hash = fhasher.add_hash(hash, _v(2));
  hash = fhasher.add_hash(hash, _v(3));
  return hash;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::generate_hash
//       Access: Published
//  Description: Adds the vector to the indicated hash generator.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
generate_hash(ChecksumHashGenerator &hashgen, FLOATTYPE threshold) const {
  TAU_PROFILE("LVecBase4::generate_hash(ChecksumHashGenerator &, FLOATTYPE)", " ", TAU_USER);
  hashgen.add_fp(_v(0), threshold);
  hashgen.add_fp(_v(1), threshold);
  hashgen.add_fp(_v(2), threshold);
  hashgen.add_fp(_v(3), threshold);
}
#endif  // FLOATTYPE_IS_INT

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::unary -
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
operator - () const {
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase4)(-_v);
#else
  return FLOATNAME(LVecBase4)(-_v(0), -_v(1), -_v(2), -_v(3));
#endif  // HAVE_EIGEN
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::vector + vector
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
operator + (const FLOATNAME(LVecBase4) &other) const {
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase4)(_v + other._v);
#else
  return FLOATNAME(LVecBase4)(_v(0) + other._v(0),
                              _v(1) + other._v(1),
                              _v(2) + other._v(2),
                              _v(3) + other._v(3));
#endif  // HAVE_EIGEN
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::vector - vector
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
operator - (const FLOATNAME(LVecBase4) &other) const {
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase4)(_v - other._v);
#else
  return FLOATNAME(LVecBase4)(_v(0) - other._v(0),
                              _v(1) - other._v(1),
                              _v(2) - other._v(2),
                              _v(3) - other._v(3));
#endif  // HAVE_EIGEN
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::vector * scalar
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
operator * (FLOATTYPE scalar) const {
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase4)(_v * scalar);
#else
  return FLOATNAME(LVecBase4)(_v(0) * scalar,
                              _v(1) * scalar,
                              _v(2) * scalar,
                              _v(3) * scalar);
#endif  // HAVE_EIGEN
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::vector / scalar
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
operator / (FLOATTYPE scalar) const {
#ifdef FLOATTYPE_IS_INT
  return FLOATNAME(LVecBase4)(_v(0) / scalar,
                              _v(1) / scalar,
                              _v(2) / scalar,
                              _v(3) / scalar);
#else
  FLOATTYPE recip_scalar = 1.0f/scalar;
  return operator * (recip_scalar);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator +=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
operator += (const FLOATNAME(LVecBase4) &other) {
#ifdef HAVE_EIGEN
  _v += other._v;
#else
  _v(0) += other._v(0);
  _v(1) += other._v(1);
  _v(2) += other._v(2);
  _v(3) += other._v(3);
#endif  // HAVE_EIGEN
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator -=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
operator -= (const FLOATNAME(LVecBase4) &other) {
#ifdef HAVE_EIGEN
  _v -= other._v;
#else
  _v(0) -= other._v(0);
  _v(1) -= other._v(1);
  _v(2) -= other._v(2);
  _v(3) -= other._v(3);
#endif  // HAVE_EIGEN
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator *=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
operator *= (FLOATTYPE scalar) {
#ifdef HAVE_EIGEN
  _v *= scalar;
#else
  _v(0) *= scalar;
  _v(1) *= scalar;
  _v(2) *= scalar;
  _v(3) *= scalar;
#endif  // HAVE_EIGEN
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator /=
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
operator /= (FLOATTYPE scalar) {
#ifdef FLOATTYPE_IS_INT
  _v(0) /= scalar;
  _v(1) /= scalar;
  _v(2) /= scalar;
  _v(3) /= scalar;
#else
  FLOATTYPE recip_scalar = 1.0f/scalar;
  operator *= (recip_scalar);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::componentwise_mult
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
componentwise_mult(const FLOATNAME(LVecBase4) &other) {
#ifdef HAVE_EIGEN
  _v = _v.cwiseProduct(other._v);
#else
  _v(0) *= other._v(0);
  _v(1) *= other._v(1);
  _v(2) *= other._v(2);
  _v(3) *= other._v(3);
#endif  // HAVE_EIGEN
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::fmax
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
fmax(const FLOATNAME(LVecBase4) &other) const {
  TAU_PROFILE("LVecBase4::fmax()", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase4)(_v.cwiseMax(other._v));
#else
  return FLOATNAME(LVecBase4)(_v(0) > other._v(0) ? _v(0) : other._v(0),
                              _v(1) > other._v(1) ? _v(1) : other._v(1),
                              _v(2) > other._v(2) ? _v(2) : other._v(2),
                              _v(3) > other._v(3) ? _v(3) : other._v(3));
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::fmin
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
fmin(const FLOATNAME(LVecBase4) &other) const {
  TAU_PROFILE("LVecBase4::fmin()", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase4)(_v.cwiseMin(other._v));
#else
  return FLOATNAME(LVecBase4)(_v(0) < other._v(0) ? _v(0) : other._v(0),
                              _v(1) < other._v(1) ? _v(1) : other._v(1),
                              _v(2) < other._v(2) ? _v(2) : other._v(2),
                              _v(3) < other._v(3) ? _v(3) : other._v(3));
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::almost_equal
//       Access: Published
//  Description: Returns true if two vectors are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase4)::
almost_equal(const FLOATNAME(LVecBase4) &other, FLOATTYPE threshold) const {
  TAU_PROFILE("bool LVecBase4::almost_equal(LVecBase4 &, FLOATTYPE)", " ", TAU_USER);
  return (IS_THRESHOLD_EQUAL(_v(0), other._v(0), threshold) &&
          IS_THRESHOLD_EQUAL(_v(1), other._v(1), threshold) &&
          IS_THRESHOLD_EQUAL(_v(2), other._v(2), threshold) &&
          IS_THRESHOLD_EQUAL(_v(3), other._v(3), threshold));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::almost_equal
//       Access: Published
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
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
output(ostream &out) const {
  out << MAYBE_ZERO(_v(0)) << " "
      << MAYBE_ZERO(_v(1)) << " "
      << MAYBE_ZERO(_v(2)) << " "
      << MAYBE_ZERO(_v(3));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::write_datagram_fixed
//       Access: Published
//  Description: Writes the vector to the Datagram using add_float32()
//               or add_float64(), depending on the type of floats in
//               the vector, regardless of the setting of
//               Datagram::set_stdfloat_double().  This is appropriate
//               when you want to write a fixed-width value to the
//               datagram, especially when you are not writing a bam
//               file.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
write_datagram_fixed(Datagram &destination) const {
#if FLOATTOKEN == 'i'
  destination.add_int32(_v(0));
  destination.add_int32(_v(1));
  destination.add_int32(_v(2));
  destination.add_int32(_v(3));
#elif FLOATTOKEN == 'f'
  destination.add_float32(_v(0));
  destination.add_float32(_v(1));
  destination.add_float32(_v(2));
  destination.add_float32(_v(3));
#else
  destination.add_float64(_v(0));
  destination.add_float64(_v(1));
  destination.add_float64(_v(2));
  destination.add_float64(_v(3));
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::read_datagram_fixed
//       Access: Published
//  Description: Reads the vector from the Datagram using get_float32()
//               or get_float64().  See write_datagram_fixed().
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
read_datagram_fixed(DatagramIterator &source) {
#if FLOATTOKEN == 'i'
  _v(0) = source.get_int32();
  _v(1) = source.get_int32();
  _v(2) = source.get_int32();
  _v(3) = source.get_int32();
#elif FLOATTOKEN == 'f'
  _v(0) = source.get_float32();
  _v(1) = source.get_float32();
  _v(2) = source.get_float32();
  _v(3) = source.get_float32();
#else
  _v(0) = source.get_float64();
  _v(1) = source.get_float64();
  _v(2) = source.get_float64();
  _v(3) = source.get_float64();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::write_datagram
//       Access: Published
//  Description: Writes the vector to the Datagram using
//               add_stdfloat().  This is appropriate when you want to
//               write the vector using the standard width setting,
//               especially when you are writing a bam file.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
write_datagram(Datagram &destination) const {
#if FLOATTOKEN == 'i'
  destination.add_int32(_v(0));
  destination.add_int32(_v(1));
  destination.add_int32(_v(2));
  destination.add_int32(_v(3));
#else
  destination.add_stdfloat(_v(0));
  destination.add_stdfloat(_v(1));
  destination.add_stdfloat(_v(2));
  destination.add_stdfloat(_v(3));
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::read_datagram
//       Access: Published
//  Description: Reads the vector from the Datagram using get_stdfloat().
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
read_datagram(DatagramIterator &source) {
#if FLOATTOKEN == 'i'
  _v(0) = source.get_int32();
  _v(1) = source.get_int32();
  _v(2) = source.get_int32();
  _v(3) = source.get_int32();
#else
  _v(0) = source.get_stdfloat();
  _v(1) = source.get_stdfloat();
  _v(2) = source.get_stdfloat();
  _v(3) = source.get_stdfloat();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: UnalignedLVecBase4::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(UnalignedLVecBase4)::
FLOATNAME(UnalignedLVecBase4)() {
}

////////////////////////////////////////////////////////////////////
//     Function: UnalignedLVecBase4::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(UnalignedLVecBase4)::
FLOATNAME(UnalignedLVecBase4)(const FLOATNAME(LVecBase4) &copy) {
  set(copy[0], copy[1], copy[2], copy[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: UnalignedLVecBase4::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(UnalignedLVecBase4)::
FLOATNAME(UnalignedLVecBase4)(const FLOATNAME(UnalignedLVecBase4) &copy) : _v(copy._v) {
}

////////////////////////////////////////////////////////////////////
//     Function: UnalignedLVecBase4::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(UnalignedLVecBase4) &FLOATNAME(UnalignedLVecBase4)::
operator = (const FLOATNAME(LVecBase4) &copy) {
  set(copy[0], copy[1], copy[2], copy[3]);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: UnalignedLVecBase4::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(UnalignedLVecBase4) &FLOATNAME(UnalignedLVecBase4)::
operator = (const FLOATNAME(UnalignedLVecBase4) &copy) {
  TAU_PROFILE("void UnalignedLVecBase4::operator =(UnalignedLVecBase4 &)", " ", TAU_USER);
  _v = copy._v;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: UnalignedLVecBase4::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(UnalignedLVecBase4)::
FLOATNAME(UnalignedLVecBase4)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w) {
  TAU_PROFILE("UnalignedLVecBase4::UnalignedLVecBase4(FLOATTYPE, ...)", " ", TAU_USER);
  set(x, y, z, w);
}

////////////////////////////////////////////////////////////////////
//     Function: UnalignedLVecBase4::set
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(UnalignedLVecBase4)::
set(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w) {
  TAU_PROFILE("void UnalignedLVecBase4::set()", " ", TAU_USER);
  _v(0) = x;
  _v(1) = y;
  _v(2) = z;
  _v(3) = w;
}

////////////////////////////////////////////////////////////////////
//     Function: UnalignedLVecBase4::Indexing Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(UnalignedLVecBase4)::
operator [](int i) const {
  nassertr(i >= 0 && i < 4, 0);
  return _v(i);
}

////////////////////////////////////////////////////////////////////
//     Function: UnalignedLVecBase4::Indexing Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE &FLOATNAME(UnalignedLVecBase4)::
operator [](int i) {
  nassertr(i >= 0 && i < 4, _v(0));
  return _v(i);
}

////////////////////////////////////////////////////////////////////
//     Function: UnalignedLVecBase4::size
//       Access: Public, Static
//  Description: Returns 4: the number of components of a LVecBase4.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(UnalignedLVecBase4)::
size() {
  return 4;
}

////////////////////////////////////////////////////////////////////
//     Function: UnalignedLVecBase4::get_data
//       Access: Public
//  Description: Returns the address of the first of the three data
//               elements in the vector.  The remaining elements
//               occupy the next positions consecutively in memory.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATTYPE *FLOATNAME(UnalignedLVecBase4)::
get_data() const {
  return &_v(0);
}

////////////////////////////////////////////////////////////////////
//     Function: UnalignedLVecBase4::get_num_components
//       Access: Public
//  Description: Returns the number of elements in the vector, 4.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(UnalignedLVecBase4)::
get_num_components() const {
  return 4;
}
