// Filename: lvecBase3_src.I
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
//     Function: LVecBase3::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3)::
FLOATNAME(LVecBase3)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3)::
FLOATNAME(LVecBase3)(const FLOATNAME(LVecBase3) &copy) {
  _v.v._0 = copy._v.v._0;
  _v.v._1 = copy._v.v._1;
  _v.v._2 = copy._v.v._2;
//  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) &FLOATNAME(LVecBase3)::
operator = (const FLOATNAME(LVecBase3) &copy) {
  _v.v._0 = copy._v.v._0;
  _v.v._1 = copy._v.v._1;
  _v.v._2 = copy._v.v._2;
//  set(copy[0], copy[1], copy[2]);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Fill Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) &FLOATNAME(LVecBase3)::
operator = (FLOATTYPE fill_value) {
  fill(fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3)::
FLOATNAME(LVecBase3)(FLOATTYPE fill_value) {
  fill(fill_value);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3)::
FLOATNAME(LVecBase3)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z) {
  _v.v._0 = x;
  _v.v._1 = y;
  _v.v._2 = z;
//  set(x, y, z);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVecBase3) &FLOATNAME(LVecBase3)::
zero() {
  return _zero;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVecBase3) &FLOATNAME(LVecBase3)::
unit_x() {
  return _unit_x;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVecBase3) &FLOATNAME(LVecBase3)::
unit_y() {
  return _unit_y;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::unit_z Named Constructor
//       Access: Public
//  Description: Returns a unit Z vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LVecBase3) &FLOATNAME(LVecBase3)::
unit_z() {
  return _unit_z;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3)::
~FLOATNAME(LVecBase3)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Indexing Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase3)::
operator [](int i) const {
  nassertr(i >= 0 && i < 3, 0.0);
  return _v.data[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Indexing Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE &FLOATNAME(LVecBase3)::
operator [](int i) {
  nassertr(i >= 0 && i < 3, _v.data[0]);
  return _v.data[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::is_nan
//       Access: Public
//  Description: Returns true if any component of the vector is
//               not-a-number, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase3)::
is_nan() const {
  return cnan(_v.v._0) || cnan(_v.v._1) || cnan(_v.v._2);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::get_cell
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase3)::
get_cell(int i) const {
  nassertr(i >= 0 && i < 3, 0);
  return _v.data[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::get_x
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase3)::
get_x() const {
  return _v.v._0;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::get_y
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase3)::
get_y() const {
  return _v.v._1;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::get_z
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase3)::
get_z() const {
  return _v.v._2;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::set_cell
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
set_cell(int i, FLOATTYPE value) {
  nassertv(i >= 0 && i < 3);
  _v.data[i] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::set_x
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
set_x(FLOATTYPE value) {
  _v.v._0 = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::set_y
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
set_y(FLOATTYPE value) {
  _v.v._1 = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::set_z
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
set_z(FLOATTYPE value) {
  _v.v._2 = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::add_to_cell
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
add_to_cell(int i, FLOATTYPE value) {
  nassertv(i >= 0 && i < 3);
  _v.data[i] += value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::add_x
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
add_x(FLOATTYPE value) {
  _v.v._0 += value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::add_y
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
add_y(FLOATTYPE value) {
  _v.v._1 += value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::add_z
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
add_z(FLOATTYPE value) {
  _v.v._2 += value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::get_data
//       Access: Public
//  Description: Returns the address of the first of the three data
//               elements in the vector.  The remaining elements
//               occupy the next positions consecutively in memory.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATTYPE *FLOATNAME(LVecBase3)::
get_data() const {
  return _v.data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::get_num_components
//       Access: Public
//  Description: Returns the number of elements in the vector, three.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LVecBase3)::
get_num_components() const {
  return 3;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3)::iterator FLOATNAME(LVecBase3)::
begin() {
  return _v.data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3)::iterator FLOATNAME(LVecBase3)::
end() {
  return begin() + get_num_components();
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3)::const_iterator FLOATNAME(LVecBase3)::
begin() const {
  return _v.data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3)::const_iterator FLOATNAME(LVecBase3)::
end() const {
  return begin() + get_num_components();
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::fill
//       Access: Public
//  Description: Sets each element of the vector to the indicated
//               fill_value.  This is particularly useful for
//               initializing to zero.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
fill(FLOATTYPE fill_value) {
  _v.v._0 = fill_value;
  _v.v._1 = fill_value;
  _v.v._2 = fill_value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::set
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
set(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z) {
  _v.v._0 = x;
  _v.v._1 = y;
  _v.v._2 = z;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::dot
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LVecBase3)::
dot(const FLOATNAME(LVecBase3) &other) const {
  return _v.v._0 * other._v.v._0 + _v.v._1 * other._v.v._1 + _v.v._2 * other._v.v._2;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::cross
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
cross(const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)(_v.v._1 * other._v.v._2 - other._v.v._1 * _v.v._2,
                              other._v.v._0 * _v.v._2 - _v.v._0 * other._v.v._2,
                              _v.v._0 * other._v.v._1 - other._v.v._0 * _v.v._1);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::operator <
//       Access: Public
//  Description: This performs a lexicographical comparison.  It's of
//               questionable mathematical meaning, but sometimes has
//               a practical purpose for sorting unique vectors,
//               especially in an STL container.  Also see
//               compare_to().
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase3)::
operator < (const FLOATNAME(LVecBase3) &other) const {
  return (compare_to(other) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::operator ==
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase3)::
operator == (const FLOATNAME(LVecBase3) &other) const {
  return (_v.v._0 == other._v.v._0 &&
          _v.v._1 == other._v.v._1 &&
          _v.v._2 == other._v.v._2);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::operator !=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase3)::
operator != (const FLOATNAME(LVecBase3) &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: get_standardized_rotation
//       Access: file
//  Description: return value in the range -180.0 to 179.99999.
//     See Also: get_standardized_hpr
////////////////////////////////////////////////////////////////////
static INLINE_LINMATH FLOATTYPE
get_standardized_rotation(FLOATTYPE angle_in_degrees) {
  if (angle_in_degrees<0.0) {
    angle_in_degrees = FLOATCONST(360.0) - fmod(angle_in_degrees * FLOATCONST(-1.0), FLOATCONST(360.0));
  } else {
    angle_in_degrees = fmod(angle_in_degrees, FLOATCONST(360.0));
  }
  // This can be changed to return values in the range 0.0 to 359.99999
  // by skipping this next part and returning now.
  
  return (angle_in_degrees<FLOATCONST(180.0))?
      angle_in_degrees:
      angle_in_degrees - FLOATCONST(360.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::compare_hpr_to
//       Access: Public
//  Description: Try to un-spin the hpr to a standard form.  Like
//               all standards, someone decides between many
//               arbitrary posible standards.  This function assumes
//               that 0 and 360 are the same, as is 720 and
//               -360.  Also 180 and -180 are the same.  Another
//               example is -90 and 270.
//               Each element will be in the range -180.0 to 179.99999.
//               The original usage of this function is for human
//               readable output and asserting that foo_hpr is roughly
//               equal to bar_hpr.
//     See Also: get_standardized_rotation
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
get_standardized_hpr() const {
  return FLOATNAME(LVecBase3)(
      get_standardized_rotation(_v.v._0),
      get_standardized_rotation(_v.v._1),
      get_standardized_rotation(_v.v._2));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::compare_to
//       Access: Public
//  Description: This flavor of compare_to uses a default threshold
//               value based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LVecBase3)::
compare_to(const FLOATNAME(LVecBase3) &other) const {
  return compare_to(other, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::compare_to
//       Access: Public
//  Description: Sorts vectors lexicographically, componentwise.
//               Returns a number less than 0 if this vector sorts
//               before the other one, greater than zero if it sorts
//               after, 0 if they are equivalent (within the indicated
//               tolerance).
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LVecBase3)::
compare_to(const FLOATNAME(LVecBase3) &other, FLOATTYPE threshold) const {
  if (!IS_THRESHOLD_COMPEQ(_v.v._0, other._v.v._0, threshold)) {
    return (_v.v._0 < other._v.v._0) ? -1 : 1;
  }
  if (!IS_THRESHOLD_COMPEQ(_v.v._1, other._v.v._1, threshold)) {
    return (_v.v._1 < other._v.v._1) ? -1 : 1;
  }
  if (!IS_THRESHOLD_COMPEQ(_v.v._2, other._v.v._2, threshold)) {
    return (_v.v._2 < other._v.v._2) ? -1 : 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::unary -
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
operator - () const {
  return FLOATNAME(LVecBase3)(-_v.v._0, -_v.v._1, -_v.v._2);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::vector + vector
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
operator + (const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)(_v.v._0 + other._v.v._0,
                              _v.v._1 + other._v.v._1,
                              _v.v._2 + other._v.v._2);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::vector - vector
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
operator - (const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)(_v.v._0 - other._v.v._0,
                              _v.v._1 - other._v.v._1,
                              _v.v._2 - other._v.v._2);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::vector * scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LVecBase3)(_v.v._0 * scalar,
                              _v.v._1 * scalar,
                              _v.v._2 * scalar);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::vector / scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
operator / (FLOATTYPE scalar) const {
  FLOATTYPE recip_scalar = 1.0f/scalar;
  return FLOATNAME(LVecBase3)(_v.v._0 * recip_scalar,
                              _v.v._1 * recip_scalar,
                              _v.v._2 * recip_scalar);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::operator +=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
operator += (const FLOATNAME(LVecBase3) &other) {
  _v.v._0 += other._v.v._0;
  _v.v._1 += other._v.v._1;
  _v.v._2 += other._v.v._2;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::operator -=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
operator -= (const FLOATNAME(LVecBase3) &other) {
  _v.v._0 -= other._v.v._0;
  _v.v._1 -= other._v.v._1;
  _v.v._2 -= other._v.v._2;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::operator *=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
operator *= (FLOATTYPE scalar) {
  _v.v._0 *= scalar;
  _v.v._1 *= scalar;
  _v.v._2 *= scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::operator /=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
operator /= (FLOATTYPE scalar) {
  FLOATTYPE recip_scalar = 1.0f/scalar;
  _v.v._0 *= recip_scalar;
  _v.v._1 *= recip_scalar;
  _v.v._2 *= recip_scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::cross product (with assigment)
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
cross_into(const FLOATNAME(LVecBase3) &other) {
  (*this) = cross(other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::almost_equal
//       Access: Public
//  Description: Returns true if two vectors are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase3)::
almost_equal(const FLOATNAME(LVecBase3) &other, FLOATTYPE threshold) const {
  return (IS_THRESHOLD_EQUAL(_v.v._0, other._v.v._0, threshold) &&
          IS_THRESHOLD_EQUAL(_v.v._1, other._v.v._1, threshold) &&
          IS_THRESHOLD_EQUAL(_v.v._2, other._v.v._2, threshold));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::almost_equal
//       Access: Public
//  Description: Returns true if two vectors are memberwise equal
//               within a default tolerance based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LVecBase3)::
almost_equal(const FLOATNAME(LVecBase3) &other) const {
  return almost_equal(other, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
output(ostream &out) const {
  out << MAYBE_ZERO(_v.v._0) << " "
      << MAYBE_ZERO(_v.v._1) << " "
      << MAYBE_ZERO(_v.v._2);
}


////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::generate_hash
//       Access: Public
//  Description: Adds the vector to the indicated hash generator.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
generate_hash(ChecksumHashGenerator &hashgen) const {
  generate_hash(hashgen, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::generate_hash
//       Access: Public
//  Description: Adds the vector to the indicated hash generator.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
generate_hash(ChecksumHashGenerator &hashgen, FLOATTYPE threshold) const {
  hashgen.add_fp(_v.v._0, threshold);
  hashgen.add_fp(_v.v._1, threshold);
  hashgen.add_fp(_v.v._2, threshold);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::write_datagram
//       Access: Public
//  Description: Function to write itself into a datagram
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
write_datagram(Datagram &destination) const {
#if FLOATTOKEN == 'f'
  destination.add_float32(_v.v._0);
  destination.add_float32(_v.v._1);
  destination.add_float32(_v.v._2);
#else
  destination.add_float64(_v.v._0);
  destination.add_float64(_v.v._1);
  destination.add_float64(_v.v._2);
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::read_datagram
//       Access: Public
//  Description: Function to read itself from a datagramIterator
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LVecBase3)::
read_datagram(DatagramIterator &source) {
#if FLOATTOKEN == 'f'
  _v.v._0 = source.get_float32();
  _v.v._1 = source.get_float32();
  _v.v._2 = source.get_float32();
#else
  _v.v._0 = source.get_float64();
  _v.v._1 = source.get_float64();
  _v.v._2 = source.get_float64();
#endif
}
