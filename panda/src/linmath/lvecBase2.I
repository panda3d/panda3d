// Filename: lvecBase2.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "nearly_zero.h"

#include <notify.h>
#include <datagram.h>
#include <datagramIterator.h>

template<class NumType>
TypeHandle LVecBase2<NumType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType>::
LVecBase2() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType>::
LVecBase2(const LVecBase2<NumType> &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> &LVecBase2<NumType>::
operator = (const LVecBase2<NumType> &copy) {
  set(copy[0], copy[1]);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Fill Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> &LVecBase2<NumType>::
operator = (NumType fill_value) {
  fill(fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType>::
LVecBase2(NumType fill_value) {
  fill(fill_value);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType>::
LVecBase2(NumType x, NumType y) {
  set(x, y);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> LVecBase2<NumType>::
zero() {
  return LVecBase2<NumType>(0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> LVecBase2<NumType>::
unit_x() {
  return LVecBase2<NumType>(1.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> LVecBase2<NumType>::
unit_y() {
  return LVecBase2<NumType>(0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType>::
~LVecBase2() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Indexing Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LVecBase2<NumType>::
operator [](int i) const {
  nassertr(i >= 0 && i < 2, 0);
  return _data[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Indexing Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType &LVecBase2<NumType>::
operator [](int i) {
  nassertr(i >= 0 && i < 2, _data[0]);
  return _data[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::is_nan
//       Access: Public
//  Description: Returns true if any component of the vector is
//               not-a-number, false otherwise.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LVecBase2<NumType>::
is_nan() const {
  return cnan(_data[0]) || cnan(_data[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::get_cell
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LVecBase2<NumType>::
get_cell(int i) const {
  nassertr(i >= 0 && i < 2, 0);
  return _data[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::get_x
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LVecBase2<NumType>::
get_x() const {
  return _data[0];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::get_y
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LVecBase2<NumType>::
get_y() const {
  return _data[1];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::set_cell
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LVecBase2<NumType>::
set_cell(int i, NumType value) {
  nassertv(i >= 0 && i < 2);
  _data[i] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::set_x
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LVecBase2<NumType>::
set_x(NumType value) {
  _data[0] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::set_y
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LVecBase2<NumType>::
set_y(NumType value) {
  _data[1] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::get_data
//       Access: Public
//  Description: Returns the address of the first of the two data
//               elements in the vector.  The next element
//               occupies the next position consecutively in memory.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE const NumType *LVecBase2<NumType>::
get_data() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::get_num_components
//       Access: Public
//  Description: Returns the number of elements in the vector, two.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE int LVecBase2<NumType>::
get_num_components() const {
  return 2;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType>::iterator LVecBase2<NumType>::
begin() {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType>::iterator LVecBase2<NumType>::
end() {
  return begin() + get_num_components();
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType>::const_iterator LVecBase2<NumType>::
begin() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType>::const_iterator LVecBase2<NumType>::
end() const {
  return begin() + get_num_components();
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::fill
//       Access: Public
//  Description: Sets each element of the vector to the indicated
//               fill_value.  This is particularly useful for
//               initializing to zero.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LVecBase2<NumType>::
fill(NumType fill_value) {
  _data[0] = fill_value;
  _data[1] = fill_value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::set
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LVecBase2<NumType>::
set(NumType x, NumType y) {
  _data[0] = x;
  _data[1] = y;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::dot
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LVecBase2<NumType>::
dot(const LVecBase2<NumType> &other) const {
  return _data[0] * other[0] + _data[1] * other[1];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::operator <
//       Access: Public
//  Description: This performs a lexicographical comparison.  It's of
//               questionable mathematical meaning, but sometimes has
//               a practical purpose for sorting unique vectors,
//               especially in an STL container.  Also see
//               compare_to().
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LVecBase2<NumType>::
operator < (const LVecBase2<NumType> &other) const {
  return (compare_to(other) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::operator ==
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LVecBase2<NumType>::
operator == (const LVecBase2<NumType> &other) const {
  return (_data[0] == other[0] &&
	  _data[1] == other[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::operator !=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LVecBase2<NumType>::
operator != (const LVecBase2<NumType> &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::compare_to
//       Access: Public
//  Description: This flavor of compare_to uses a default threshold
//               value based on the numeric type.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE int LVecBase2<NumType>::
compare_to(const LVecBase2<NumType> &other) const {
  return compare_to(other, NEARLY_ZERO(NumType));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::compare_to
//       Access: Public
//  Description: Sorts vectors lexicographically, componentwise.
//               Returns a number less than 0 if this vector sorts
//               before the other one, greater than zero if it sorts
//               after, 0 if they are equivalent (within the indicated
//               tolerance).
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE int LVecBase2<NumType>::
compare_to(const LVecBase2<NumType> &other, NumType threshold) const {
  if (!IS_THRESHOLD_EQUAL(_data[0], other[0], threshold)) {
    return (_data[0] < other[0]) ? -1 : 1;
  }
  if (!IS_THRESHOLD_EQUAL(_data[1], other[1], threshold)) {
    return (_data[1] < other[1]) ? -1 : 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::unary -
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> LVecBase2<NumType>::
operator - () const {
  return LVecBase2<NumType>(-_data[0], -_data[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::vector + vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> LVecBase2<NumType>::
operator + (const LVecBase2<NumType> &other) const {
  return LVecBase2<NumType>(_data[0] + other[0],
			    _data[1] + other[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::vector - vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> LVecBase2<NumType>::
operator - (const LVecBase2<NumType> &other) const {
  return LVecBase2<NumType>(_data[0] - other[0],
			    _data[1] - other[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::vector * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> LVecBase2<NumType>::
operator * (NumType scalar) const {
  return LVecBase2<NumType>(_data[0] * scalar,
			    _data[1] * scalar);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::vector / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> LVecBase2<NumType>::
operator / (NumType scalar) const {
  return LVecBase2<NumType>(_data[0] / scalar,
			    _data[1] / scalar);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::operator +=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LVecBase2<NumType>::
operator += (const LVecBase2<NumType> &other) {
  _data[0] += other[0];
  _data[1] += other[1];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::operator -=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LVecBase2<NumType>::
operator -= (const LVecBase2<NumType> &other) {
  _data[0] -= other[0];
  _data[1] -= other[1];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::operator *=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LVecBase2<NumType>::
operator *= (NumType scalar) {
  _data[0] *= scalar;
  _data[1] *= scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::operator /=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LVecBase2<NumType>::
operator /= (NumType scalar) {
  _data[0] /= scalar;
  _data[1] /= scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::almost_equal
//       Access: Public
//  Description: Returns true if two vectors are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LVecBase2<NumType>::
almost_equal(const LVecBase2<NumType> &other, NumType threshold) const {
  return (IS_THRESHOLD_EQUAL(_data[0], other[0], threshold) &&
	  IS_THRESHOLD_EQUAL(_data[1], other[1], threshold));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::almost_equal
//       Access: Public
//  Description: Returns true if two vectors are memberwise equal
//               within a default tolerance based on the numeric type.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LVecBase2<NumType>::
almost_equal(const LVecBase2<NumType> &other) const {
  return almost_equal(other, NEARLY_ZERO(NumType));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LVecBase2<NumType>::
output(ostream &out) const {
  out << MAYBE_ZERO(_data[0]) << " " 
      << MAYBE_ZERO(_data[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void LVecBase2<NumType>::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    do_init_type(NumType);
    string name =
      "LVecBase2<" + get_type_handle(NumType).get_name() + ">";
    register_type(_type_handle, name);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::write_datagram
//       Access: Public
//  Description: Function to write itself into a datagram
////////////////////////////////////////////////////////////////////
template<class NumType>
void LVecBase2<NumType>::
write_datagram(Datagram &destination) const {
  destination.add_float32(_data[0]);
  destination.add_float32(_data[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::read_datagram
//       Access: Public
//  Description: Function to read itself from a datagramIterator
////////////////////////////////////////////////////////////////////
template<class NumType>
void LVecBase2<NumType>::
read_datagram(DatagramIterator &source) {
  _data[0] = source.get_float32();
  _data[1] = source.get_float32();
}

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a vector from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////
template<class NumType, class NumType2>
INLINE LVecBase2<NumType2> 
lcast_to(NumType2 *, const LVecBase2<NumType> &source) {
  return LVecBase2<NumType2>(source[0], source[1]);
}
