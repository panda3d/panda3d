// Filename: lvecBase4_src.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4)::
FLOATNAME(LVecBase4)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4)::
FLOATNAME(LVecBase4)(const FLOATNAME(LVecBase4) &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
operator = (const FLOATNAME(LVecBase4) &copy) {
  set(copy[0], copy[1], copy[2], copy[3]);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Fill Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
operator = (FLOATTYPE fill_value) {
  fill(fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4)::
FLOATNAME(LVecBase4)(FLOATTYPE fill_value) {
  fill(fill_value);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4)::
FLOATNAME(LVecBase4)(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w) {
  set(x, y, z, w);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4)::
~FLOATNAME(LVecBase4)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length vector.
////////////////////////////////////////////////////////////////////
INLINE const FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
zero() {
  return _zero;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X vector.
////////////////////////////////////////////////////////////////////
INLINE const FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
unit_x() {
  return _unit_x;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y vector.
////////////////////////////////////////////////////////////////////
INLINE const FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
unit_y() {
  return _unit_y;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::unit_z Named Constructor
//       Access: Public
//  Description: Returns a unit Z vector.
////////////////////////////////////////////////////////////////////
INLINE const FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
unit_z() {
  return _unit_z;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::unit_w Named Constructor
//       Access: Public
//  Description: Returns a unit W vector.
////////////////////////////////////////////////////////////////////
INLINE const FLOATNAME(LVecBase4) &FLOATNAME(LVecBase4)::
unit_w() {
  return _unit_w;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Indexing Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LVecBase4)::
operator [](int i) const {
  //  nassertr(i >= 0 && i < 4, 0);
  return _data[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::Indexing Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE &FLOATNAME(LVecBase4)::
operator [](int i) {
  //  nassertr(i >= 0 && i < 4, _data[0]);
  return _data[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::is_nan
//       Access: Public
//  Description: Returns true if any component of the vector is
//               not-a-number, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LVecBase4)::
is_nan() const {
  return cnan(_data[0]) || cnan(_data[1]) || cnan(_data[2]) || cnan(_data[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_cell
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LVecBase4)::
get_cell(int i) const {
  nassertr(i >= 0 && i < 4, 0);
  return _data[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_x
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LVecBase4)::
get_x() const {
  return _data[0];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_y
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LVecBase4)::
get_y() const {
  return _data[1];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_z
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LVecBase4)::
get_z() const {
  return _data[2];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_w
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LVecBase4)::
get_w() const {
  return _data[3];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set_cell
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase4)::
set_cell(int i, FLOATTYPE value) {
  nassertv(i >= 0 && i < 4);
  _data[i] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set_x
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase4)::
set_x(FLOATTYPE value) {
  _data[0] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set_y
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase4)::
set_y(FLOATTYPE value) {
  _data[1] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set_z
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase4)::
set_z(FLOATTYPE value) {
  _data[2] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set_w
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase4)::
set_w(FLOATTYPE value) {
  _data[3] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_data
//       Access: Public
//  Description: Returns the address of the first of the four data
//               elements in the vector.  The remaining elements
//               occupy the next positions consecutively in memory.
////////////////////////////////////////////////////////////////////
INLINE const FLOATTYPE *FLOATNAME(LVecBase4)::
get_data() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::get_num_components
//       Access: Public
//  Description: Returns the number of elements in the vector, four.
////////////////////////////////////////////////////////////////////
INLINE int FLOATNAME(LVecBase4)::
get_num_components() const {
  return 4;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4)::iterator FLOATNAME(LVecBase4)::
begin() {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4)::iterator FLOATNAME(LVecBase4)::
end() {
  return begin() + get_num_components();
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4)::const_iterator FLOATNAME(LVecBase4)::
begin() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4)::const_iterator FLOATNAME(LVecBase4)::
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
INLINE void FLOATNAME(LVecBase4)::
fill(FLOATTYPE fill_value) {
  _data[0] = fill_value;
  _data[1] = fill_value;
  _data[2] = fill_value;
  _data[3] = fill_value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::set
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase4)::
set(FLOATTYPE x, FLOATTYPE y, FLOATTYPE z, FLOATTYPE w) {
  _data[0] = x;
  _data[1] = y;
  _data[2] = z;
  _data[3] = w;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::dot
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LVecBase4)::
dot(const FLOATNAME(LVecBase4) &other) const {
  return 
    _data[0] * other[0] + _data[1] * other[1] + 
    _data[2] * other[2] + _data[3] * other[3];
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
INLINE bool FLOATNAME(LVecBase4)::
operator < (const FLOATNAME(LVecBase4) &other) const {
  return (compare_to(other) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator ==
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LVecBase4)::
operator == (const FLOATNAME(LVecBase4) &other) const {
  return (_data[0] == other[0] &&
	  _data[1] == other[1] &&
	  _data[2] == other[2] &&
	  _data[3] == other[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator !=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LVecBase4)::
operator != (const FLOATNAME(LVecBase4) &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::compare_to
//       Access: Public
//  Description: This flavor of compare_to uses a default threshold
//               value based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE int FLOATNAME(LVecBase4)::
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
INLINE int FLOATNAME(LVecBase4)::
compare_to(const FLOATNAME(LVecBase4) &other, FLOATTYPE threshold) const {
  if (!IS_THRESHOLD_EQUAL(_data[0], other[0], threshold)) {
    return (_data[0] < other[0]) ? -1 : 1;
  }
  if (!IS_THRESHOLD_EQUAL(_data[1], other[1], threshold)) {
    return (_data[1] < other[1]) ? -1 : 1;
  }
  if (!IS_THRESHOLD_EQUAL(_data[2], other[2], threshold)) {
    return (_data[2] < other[2]) ? -1 : 1;
  }
  if (!IS_THRESHOLD_EQUAL(_data[3], other[3], threshold)) {
    return (_data[3] < other[3]) ? -1 : 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::unary -
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
operator - () const {
  return FLOATNAME(LVecBase4)(-_data[0], -_data[1], -_data[2], -_data[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::vector + vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
operator + (const FLOATNAME(LVecBase4) &other) const {
  return FLOATNAME(LVecBase4)(_data[0] + other[0],
			      _data[1] + other[1],
			      _data[2] + other[2],
			      _data[3] + other[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::vector - vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
operator - (const FLOATNAME(LVecBase4) &other) const {
  return FLOATNAME(LVecBase4)(_data[0] - other[0],
			      _data[1] - other[1],
			      _data[2] - other[2],
			      _data[3] - other[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::vector * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
operator * (FLOATTYPE scalar) const {
  return FLOATNAME(LVecBase4)(_data[0] * scalar,
			      _data[1] * scalar,
			      _data[2] * scalar,
			      _data[3] * scalar);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::vector / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4) FLOATNAME(LVecBase4)::
operator / (FLOATTYPE scalar) const {
  return FLOATNAME(LVecBase4)(_data[0] / scalar,
			      _data[1] / scalar,
			      _data[2] / scalar,
			      _data[3] / scalar);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator +=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase4)::
operator += (const FLOATNAME(LVecBase4) &other) {
  _data[0] += other[0];
  _data[1] += other[1];
  _data[2] += other[2];
  _data[3] += other[3];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator -=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase4)::
operator -= (const FLOATNAME(LVecBase4) &other) {
  _data[0] -= other[0];
  _data[1] -= other[1];
  _data[2] -= other[2];
  _data[3] -= other[3];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator *=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase4)::
operator *= (FLOATTYPE scalar) {
  _data[0] *= scalar;
  _data[1] *= scalar;
  _data[2] *= scalar;
  _data[3] *= scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::operator /=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase4)::
operator /= (FLOATTYPE scalar) {
  _data[0] /= scalar;
  _data[1] /= scalar;
  _data[2] /= scalar;
  _data[3] /= scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::almost_equal
//       Access: Public
//  Description: Returns true if two vectors are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LVecBase4)::
almost_equal(const FLOATNAME(LVecBase4) &other, FLOATTYPE threshold) const {
  return (IS_THRESHOLD_EQUAL(_data[0], other[0], threshold) &&
	  IS_THRESHOLD_EQUAL(_data[1], other[1], threshold) &&
	  IS_THRESHOLD_EQUAL(_data[2], other[2], threshold) &&
	  IS_THRESHOLD_EQUAL(_data[3], other[3], threshold));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::almost_equal
//       Access: Public
//  Description: Returns true if two vectors are memberwise equal
//               within a default tolerance based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LVecBase4)::
almost_equal(const FLOATNAME(LVecBase4) &other) const {
  return almost_equal(other, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase4)::
output(ostream &out) const {
  out << MAYBE_ZERO(_data[0]) << " " 
      << MAYBE_ZERO(_data[1]) << " " 
      << MAYBE_ZERO(_data[2]) << " " 
      << MAYBE_ZERO(_data[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::write_datagram
//       Access: Public
//  Description: Function to write itself into a datagram
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase4)::
write_datagram(Datagram &destination) const {
  destination.add_float32(_data[0]);
  destination.add_float32(_data[1]);
  destination.add_float32(_data[2]);
  destination.add_float32(_data[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase4::read_datagram
//       Access: Public
//  Description: Function to read itself from a datagramIterator
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LVecBase4)::
read_datagram(DatagramIterator &source) {
  _data[0] = source.get_float32();
  _data[1] = source.get_float32();
  _data[2] = source.get_float32();
  _data[3] = source.get_float32();
}
