// Filename: lvecBase4_src.I
// Created by:  drose (08Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::is_nan
//       Access: Public
//  Description: Returns true if any component of the vector is
//               not-a-number, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase4)::
is_nan() const {
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
  _v.v._0 = x;
  _v.v._1 = y;
  _v.v._2 = z;
  _v.v._3 = w;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::dot
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase4)::
dot(const FLOATNAME(LVecBase4) &other) const {
  return
    _v.v._0 * other._v.v._0 + _v.v._1 * other._v.v._1 +
    _v.v._2 * other._v.v._2 + _v.v._3 * other._v.v._3;
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
  return (compare_to(other) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator ==
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase4)::
operator == (const FLOATNAME(LVecBase4) &other) const {
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
//     Function: LVecBase4::almost_equal
//       Access: Public
//  Description: Returns true if two vectors are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase4)::
almost_equal(const FLOATNAME(LVecBase4) &other, FLOATTYPE threshold) const {
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

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::generate_hash
//       Access: Public
//  Description: Adds the vector to the indicated hash generator.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
generate_hash(ChecksumHashGenerator &hashgen) const {
  generate_hash(hashgen, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::generate_hash
//       Access: Public
//  Description: Adds the vector to the indicated hash generator.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
generate_hash(ChecksumHashGenerator &hashgen, FLOATTYPE threshold) const {
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
  destination.add_float32(_v.v._0);
  destination.add_float32(_v.v._1);
  destination.add_float32(_v.v._2);
  destination.add_float32(_v.v._3);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::read_datagram
//       Access: Public
//  Description: Function to read itself from a datagramIterator
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase4)::
read_datagram(DatagramIterator &source) {
  _v.v._0 = source.get_float32();
  _v.v._1 = source.get_float32();
  _v.v._2 = source.get_float32();
  _v.v._3 = source.get_float32();
}
