// Filename: lvecBase3_src.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3)::
FLOATNAME(LVecBase3)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3)::
FLOATNAME(LVecBase3)(const FLOATNAME(LVecBase3) &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3) &FLOATNAME(LVecBase3)::
operator = (const FLOATNAME(LVecBase3) &copy) {
  set(copy[0], copy[1], copy[2]);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Fill Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3) &FLOATNAME(LVecBase3)::
operator = (FLOATTYPE fill_value) {
  fill(fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3)::
FLOATNAME(LVecBase3)(FLOATTYPE fill_value) {
  fill(fill_value);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3)::
FLOATNAME(LVecBase3)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z) {
  set(x, y, z);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length vector.
////////////////////////////////////////////////////////////////////
INLINE const FLOATNAME(LVecBase3) &FLOATNAME(LVecBase3)::
zero() {
  return _zero;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X vector.
////////////////////////////////////////////////////////////////////
INLINE const FLOATNAME(LVecBase3) &FLOATNAME(LVecBase3)::
unit_x() {
  return _unit_x;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y vector.
////////////////////////////////////////////////////////////////////
INLINE const FLOATNAME(LVecBase3) &FLOATNAME(LVecBase3)::
unit_y() {
  return _unit_y;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::unit_z Named Constructor
//       Access: Public
//  Description: Returns a unit Z vector.
////////////////////////////////////////////////////////////////////
INLINE const FLOATNAME(LVecBase3) &FLOATNAME(LVecBase3)::
unit_z() {
  return _unit_z;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3)::
~FLOATNAME(LVecBase3)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Indexing Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LVecBase3)::
operator [](int i) const {
  //nassertr(i >= 0 && i < 3, 0);
  return _data[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::Indexing Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE &FLOATNAME(LVecBase3)::
operator [](int i) {
  //nassertr(i >= 0 && i < 3, _data[0]);
  return _data[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::is_nan
//       Access: Public
//  Description: Returns true if any component of the vector is
//               not-a-number, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LVecBase3)::
is_nan() const {
  return cnan(_data[0]) || cnan(_data[1]) || cnan(_data[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::get_cell
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LVecBase3)::
get_cell(int i) const {
  nassertr(i >= 0 && i < 3, 0);
  return _data[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::get_x
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LVecBase3)::
get_x() const {
  return _data[0];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::get_y
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LVecBase3)::
get_y() const {
  return _data[1];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::get_z
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LVecBase3)::
get_z() const {
  return _data[2];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::set_cell
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase3)::
set_cell(int i, FLOATTYPE value) {
  nassertv(i >= 0 && i < 3);
  _data[i] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::set_x
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase3)::
set_x(FLOATTYPE value) {
  _data[0] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::set_y
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase3)::
set_y(FLOATTYPE value) {
  _data[1] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::set_z
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase3)::
set_z(FLOATTYPE value) {
  _data[2] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::get_data
//       Access: Public
//  Description: Returns the address of the first of the three data
//               elements in the vector.  The remaining elements
//               occupy the next positions consecutively in memory.
////////////////////////////////////////////////////////////////////
INLINE const FLOATTYPE *FLOATNAME(LVecBase3)::
get_data() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::get_num_components
//       Access: Public
//  Description: Returns the number of elements in the vector, three.
////////////////////////////////////////////////////////////////////
INLINE int FLOATNAME(LVecBase3)::
get_num_components() const {
  return 3;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3)::iterator FLOATNAME(LVecBase3)::
begin() {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3)::iterator FLOATNAME(LVecBase3)::
end() {
  return begin() + get_num_components();
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3)::const_iterator FLOATNAME(LVecBase3)::
begin() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3)::const_iterator FLOATNAME(LVecBase3)::
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
INLINE void FLOATNAME(LVecBase3)::
fill(FLOATTYPE fill_value) {
  _data[0] = fill_value;
  _data[1] = fill_value;
  _data[2] = fill_value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::set
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase3)::
set(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z) {
  _data[0] = x;
  _data[1] = y;
  _data[2] = z;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::dot
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LVecBase3)::
dot(const FLOATNAME(LVecBase3) &other) const {
  return _data[0] * other[0] + _data[1] * other[1] + _data[2] * other[2];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::cross
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
cross(const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)(_data[1] * other[2] - other[1] * _data[2],
			      other[0] * _data[2] - _data[0] * other[2],
			      _data[0] * other[1] - other[0] * _data[1]);
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
INLINE bool FLOATNAME(LVecBase3)::
operator < (const FLOATNAME(LVecBase3) &other) const {
  return (compare_to(other) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::operator ==
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LVecBase3)::
operator == (const FLOATNAME(LVecBase3) &other) const {
  return (_data[0] == other[0] &&
	  _data[1] == other[1] &&
	  _data[2] == other[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::operator !=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LVecBase3)::
operator != (const FLOATNAME(LVecBase3) &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::compare_to
//       Access: Public
//  Description: This flavor of compare_to uses a default threshold
//               value based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE int FLOATNAME(LVecBase3)::
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
INLINE int FLOATNAME(LVecBase3)::
compare_to(const FLOATNAME(LVecBase3) &other, FLOATTYPE threshold) const {
  if (!IS_THRESHOLD_EQUAL(_data[0], other[0], threshold)) {
    return (_data[0] < other[0]) ? -1 : 1;
  }
  if (!IS_THRESHOLD_EQUAL(_data[1], other[1], threshold)) {
    return (_data[1] < other[1]) ? -1 : 1;
  }
  if (!IS_THRESHOLD_EQUAL(_data[2], other[2], threshold)) {
    return (_data[2] < other[2]) ? -1 : 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::unary -
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
operator - () const {
  return FLOATNAME(LVecBase3)(-_data[0], -_data[1], -_data[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::vector + vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
operator + (const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)(_data[0] + other[0],
			      _data[1] + other[1],
			      _data[2] + other[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::vector - vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
operator - (const FLOATNAME(LVecBase3) &other) const {
  return FLOATNAME(LVecBase3)(_data[0] - other[0],
			      _data[1] - other[1],
			      _data[2] - other[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::vector * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LVecBase3)(_data[0] * scalar,
			      _data[1] * scalar,
			      _data[2] * scalar);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::vector / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3) FLOATNAME(LVecBase3)::
operator / (FLOATTYPE scalar) const {
  return FLOATNAME(LVecBase3)(_data[0] / scalar,
			      _data[1] / scalar,
			      _data[2] / scalar);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::operator +=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase3)::
operator += (const FLOATNAME(LVecBase3) &other) {
  _data[0] += other[0];
  _data[1] += other[1];
  _data[2] += other[2];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::operator -=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase3)::
operator -= (const FLOATNAME(LVecBase3) &other) {
  _data[0] -= other[0];
  _data[1] -= other[1];
  _data[2] -= other[2];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::operator *=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase3)::
operator *= (FLOATTYPE scalar) {
  _data[0] *= scalar;
  _data[1] *= scalar;
  _data[2] *= scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::operator /=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase3)::
operator /= (FLOATTYPE scalar) {
  _data[0] /= scalar;
  _data[1] /= scalar;
  _data[2] /= scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::cross product (with assigment)
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase3)::
cross_into(const FLOATNAME(LVecBase3) &other) {
  (*this) = cross(other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::almost_equal
//       Access: Public
//  Description: Returns true if two vectors are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LVecBase3)::
almost_equal(const FLOATNAME(LVecBase3) &other, FLOATTYPE threshold) const {
  return (IS_THRESHOLD_EQUAL(_data[0], other[0], threshold) &&
	  IS_THRESHOLD_EQUAL(_data[1], other[1], threshold) &&
	  IS_THRESHOLD_EQUAL(_data[2], other[2], threshold));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::almost_equal
//       Access: Public
//  Description: Returns true if two vectors are memberwise equal
//               within a default tolerance based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LVecBase3)::
almost_equal(const FLOATNAME(LVecBase3) &other) const {
  return almost_equal(other, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase3)::
output(ostream &out) const {
  out << MAYBE_ZERO(_data[0]) << " " 
      << MAYBE_ZERO(_data[1]) << " " 
      << MAYBE_ZERO(_data[2]);
}


////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::write_datagram
//       Access: Public
//  Description: Function to write itself into a datagram
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase3)::
write_datagram(Datagram &destination) const {
  destination.add_float32(_data[0]);
  destination.add_float32(_data[1]);
  destination.add_float32(_data[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase3::read_datagram
//       Access: Public
//  Description: Function to read itself from a datagramIterator
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase3)::
read_datagram(DatagramIterator &source) {
  _data[0] = source.get_float32();
  _data[1] = source.get_float32();
  _data[2] = source.get_float32();
}
