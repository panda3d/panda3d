// Filename: lvecBase2.I
// Created by:  drose (08Mar00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// 	 Class : LVecBase2
// Description : This is the base class for all two-component
//               vectors and points.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA FLOATNAME(LVecBase2) {
PUBLISHED:
  typedef const FLOATTYPE1 *iterator;
  typedef const FLOATTYPE1 *const_iterator;

  INLINE FLOATNAME(LVecBase2)();
  INLINE FLOATNAME(LVecBase2)(const FLOATNAME(LVecBase2) &copy);
  INLINE FLOATNAME(LVecBase2) &operator = (const FLOATNAME(LVecBase2) &copy);
  INLINE FLOATNAME(LVecBase2) &operator = (FLOATTYPE1 fill_value);
  INLINE FLOATNAME(LVecBase2)(FLOATTYPE1 fill_value);
  INLINE FLOATNAME(LVecBase2)(FLOATTYPE1 x, FLOATTYPE1 y);

  INLINE static FLOATNAME(LVecBase2) zero();
  INLINE static FLOATNAME(LVecBase2) unit_x();
  INLINE static FLOATNAME(LVecBase2) unit_y();

  INLINE ~FLOATNAME(LVecBase2)();

  INLINE FLOATTYPE1 operator [](int i) const;
  INLINE FLOATTYPE1 &operator [](int i);

  INLINE bool is_nan() const;

  INLINE FLOATTYPE1 get_cell(int i) const;
  INLINE FLOATTYPE1 get_x() const;
  INLINE FLOATTYPE1 get_y() const;
  INLINE void set_cell(int i, FLOATTYPE1 value);
  INLINE void set_x(FLOATTYPE1 value);
  INLINE void set_y(FLOATTYPE1 value);

  INLINE const FLOATTYPE1 *get_data() const;
  INLINE int get_num_components() const;

public:
  INLINE iterator begin();
  INLINE iterator end();

  INLINE const_iterator begin() const;
  INLINE const_iterator end() const;

PUBLISHED:
  INLINE void fill(FLOATTYPE1 fill_value);
  INLINE void set(FLOATTYPE1 x, FLOATTYPE1 y);

  INLINE FLOATTYPE1 dot(const FLOATNAME(LVecBase2) &other) const;

  INLINE bool operator < (const FLOATNAME(LVecBase2) &other) const;
  INLINE bool operator == (const FLOATNAME(LVecBase2) &other) const;
  INLINE bool operator != (const FLOATNAME(LVecBase2) &other) const;

  INLINE int compare_to(const FLOATNAME(LVecBase2) &other) const;
  INLINE int compare_to(const FLOATNAME(LVecBase2) &other,
		        FLOATTYPE1 threshold) const;

  INLINE FLOATNAME(LVecBase2) operator - () const;

  INLINE FLOATNAME(LVecBase2)
  operator + (const FLOATNAME(LVecBase2) &other) const;
  INLINE FLOATNAME(LVecBase2)
  operator - (const FLOATNAME(LVecBase2) &other) const;

  INLINE FLOATNAME(LVecBase2) operator * (FLOATTYPE1 scalar) const;
  INLINE FLOATNAME(LVecBase2) operator / (FLOATTYPE1 scalar) const;

  INLINE void operator += (const FLOATNAME(LVecBase2) &other);
  INLINE void operator -= (const FLOATNAME(LVecBase2) &other);

  INLINE void operator *= (FLOATTYPE1 scalar);
  INLINE void operator /= (FLOATTYPE1 scalar);

  INLINE bool almost_equal(const FLOATNAME(LVecBase2) &other, 
			   FLOATTYPE1 threshold) const;
  INLINE bool almost_equal(const FLOATNAME(LVecBase2) &other) const;

  INLINE void output(ostream &out) const;

private:
  FLOATTYPE1 _data[2];

public:
  INLINE void write_datagram(Datagram &destination) const;
  INLINE void read_datagram(DatagramIterator &source);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();
 
private:
  static TypeHandle _type_handle;
};


INLINE ostream &operator << (ostream &out, const FLOATNAME(LVecBase2) &vec) {
  vec.output(out);
  return out;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2)::
FLOATNAME(LVecBase2)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2)::
FLOATNAME(LVecBase2)(const FLOATNAME(LVecBase2) &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2) &FLOATNAME(LVecBase2)::
operator = (const FLOATNAME(LVecBase2) &copy) {
  set(copy[0], copy[1]);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Fill Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2) &FLOATNAME(LVecBase2)::
operator = (FLOATTYPE1 fill_value) {
  fill(fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2)::
FLOATNAME(LVecBase2)(FLOATTYPE1 fill_value) {
  fill(fill_value);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2)::
FLOATNAME(LVecBase2)(FLOATTYPE1 x, FLOATTYPE1 y) {
  set(x, y);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::zero Named Constructor
//       Access: Public
//  Description: Returns a zero-length vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2) FLOATNAME(LVecBase2)::
zero() {
  return FLOATNAME(LVecBase2)(0.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::unit_x Named Constructor
//       Access: Public
//  Description: Returns a unit X vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2) FLOATNAME(LVecBase2)::
unit_x() {
  return FLOATNAME(LVecBase2)(1.0, 0.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::unit_y Named Constructor
//       Access: Public
//  Description: Returns a unit Y vector.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2) FLOATNAME(LVecBase2)::
unit_y() {
  return FLOATNAME(LVecBase2)(0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2)::
~FLOATNAME(LVecBase2)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Indexing Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1 FLOATNAME(LVecBase2)::
operator [](int i) const {
  nassertr(i >= 0 && i < 2, 0);
  return _data[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::Indexing Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1 &FLOATNAME(LVecBase2)::
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

INLINE bool FLOATNAME(LVecBase2)::
is_nan() const {
  return cnan(_data[0]) || cnan(_data[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::get_cell
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1 FLOATNAME(LVecBase2)::
get_cell(int i) const {
  nassertr(i >= 0 && i < 2, 0);
  return _data[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::get_x
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1 FLOATNAME(LVecBase2)::
get_x() const {
  return _data[0];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::get_y
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1 FLOATNAME(LVecBase2)::
get_y() const {
  return _data[1];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::set_cell
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LVecBase2)::
set_cell(int i, FLOATTYPE1 value) {
  nassertv(i >= 0 && i < 2);
  _data[i] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::set_x
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LVecBase2)::
set_x(FLOATTYPE1 value) {
  _data[0] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::set_y
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LVecBase2)::
set_y(FLOATTYPE1 value) {
  _data[1] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::get_data
//       Access: Public
//  Description: Returns the address of the first of the two data
//               elements in the vector.  The next element
//               occupies the next position consecutively in memory.
////////////////////////////////////////////////////////////////////

INLINE const FLOATTYPE1 *FLOATNAME(LVecBase2)::
get_data() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::get_num_components
//       Access: Public
//  Description: Returns the number of elements in the vector, two.
////////////////////////////////////////////////////////////////////

INLINE int FLOATNAME(LVecBase2)::
get_num_components() const {
  return 2;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2)::iterator FLOATNAME(LVecBase2)::
begin() {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2)::iterator FLOATNAME(LVecBase2)::
end() {
  return begin() + get_num_components();
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2)::const_iterator FLOATNAME(LVecBase2)::
begin() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2)::const_iterator FLOATNAME(LVecBase2)::
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

INLINE void FLOATNAME(LVecBase2)::
fill(FLOATTYPE1 fill_value) {
  _data[0] = fill_value;
  _data[1] = fill_value;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::set
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LVecBase2)::
set(FLOATTYPE1 x, FLOATTYPE1 y) {
  _data[0] = x;
  _data[1] = y;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::dot
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATTYPE1 FLOATNAME(LVecBase2)::
dot(const FLOATNAME(LVecBase2) &other) const {
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

INLINE bool FLOATNAME(LVecBase2)::
operator < (const FLOATNAME(LVecBase2) &other) const {
  return (compare_to(other) < 0);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::operator ==
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE bool FLOATNAME(LVecBase2)::
operator == (const FLOATNAME(LVecBase2) &other) const {
  return (_data[0] == other[0] &&
	  _data[1] == other[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::operator !=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE bool FLOATNAME(LVecBase2)::
operator != (const FLOATNAME(LVecBase2) &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::compare_to
//       Access: Public
//  Description: This flavor of compare_to uses a default threshold
//               value based on the numeric type.
////////////////////////////////////////////////////////////////////

INLINE int FLOATNAME(LVecBase2)::
compare_to(const FLOATNAME(LVecBase2) &other) const {
  return compare_to(other, NEARLY_ZERO(FLOATTYPE1));
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

INLINE int FLOATNAME(LVecBase2)::
compare_to(const FLOATNAME(LVecBase2) &other, FLOATTYPE1 threshold) const {
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

INLINE FLOATNAME(LVecBase2) FLOATNAME(LVecBase2)::
operator - () const {
  return FLOATNAME(LVecBase2)(-_data[0], -_data[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::vector + vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2) FLOATNAME(LVecBase2)::
operator + (const FLOATNAME(LVecBase2) &other) const {
  return FLOATNAME(LVecBase2)(_data[0] + other[0],
			    _data[1] + other[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::vector - vector
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2) FLOATNAME(LVecBase2)::
operator - (const FLOATNAME(LVecBase2) &other) const {
  return FLOATNAME(LVecBase2)(_data[0] - other[0],
			    _data[1] - other[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::vector * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2) FLOATNAME(LVecBase2)::
operator * (FLOATTYPE1 scalar) const {
  return FLOATNAME(LVecBase2)(_data[0] * scalar,
			    _data[1] * scalar);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::vector / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE FLOATNAME(LVecBase2) FLOATNAME(LVecBase2)::
operator / (FLOATTYPE1 scalar) const {
  return FLOATNAME(LVecBase2)(_data[0] / scalar,
			    _data[1] / scalar);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::operator +=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LVecBase2)::
operator += (const FLOATNAME(LVecBase2) &other) {
  _data[0] += other[0];
  _data[1] += other[1];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::operator -=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LVecBase2)::
operator -= (const FLOATNAME(LVecBase2) &other) {
  _data[0] -= other[0];
  _data[1] -= other[1];
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::operator *=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LVecBase2)::
operator *= (FLOATTYPE1 scalar) {
  _data[0] *= scalar;
  _data[1] *= scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::operator /=
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LVecBase2)::
operator /= (FLOATTYPE1 scalar) {
  _data[0] /= scalar;
  _data[1] /= scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::almost_equal
//       Access: Public
//  Description: Returns true if two vectors are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////

INLINE bool FLOATNAME(LVecBase2)::
almost_equal(const FLOATNAME(LVecBase2) &other, FLOATTYPE1 threshold) const {
  return (IS_THRESHOLD_EQUAL(_data[0], other[0], threshold) &&
	  IS_THRESHOLD_EQUAL(_data[1], other[1], threshold));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::almost_equal
//       Access: Public
//  Description: Returns true if two vectors are memberwise equal
//               within a default tolerance based on the numeric type.
////////////////////////////////////////////////////////////////////

INLINE bool FLOATNAME(LVecBase2)::
almost_equal(const FLOATNAME(LVecBase2) &other) const {
  return almost_equal(other, NEARLY_ZERO(FLOATTYPE1));
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LVecBase2)::
output(ostream &out) const {
  out << MAYBE_ZERO(_data[0]) << " " 
      << MAYBE_ZERO(_data[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::write_datagram
//       Access: Public
//  Description: Function to write itself into a datagram
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LVecBase2)::
write_datagram(Datagram &destination) const {
  destination.add_float32(_data[0]);
  destination.add_float32(_data[1]);
}

////////////////////////////////////////////////////////////////////
//     Function: LVecBase2::read_datagram
//       Access: Public
//  Description: Function to read itself from a datagramIterator
////////////////////////////////////////////////////////////////////

INLINE void FLOATNAME(LVecBase2)::
read_datagram(DatagramIterator &source) {
  _data[0] = source.get_float32();
  _data[1] = source.get_float32();
}



