// Filename: lmatrix4_src.I
// Created by:  drose (15Jan99)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::ident_mat
//       Access: Public, Static
//  Description: Returns an identity matrix.
//
//               This function definition must appear first, since
//               some inline functions below take advantage of it.
////////////////////////////////////////////////////////////////////
INLINE const FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
ident_mat() {
  return _ident_mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4)::
FLOATNAME(LMatrix4)() { 
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4):: 
FLOATNAME(LMatrix4)(const FLOATNAME(LMatrix4) &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4):: 
operator = (const FLOATNAME(LMatrix4) &copy) {
  set(copy(0, 0), copy(0, 1), copy(0, 2), copy(0, 3),
      copy(1, 0), copy(1, 1), copy(1, 2), copy(1, 3),
      copy(2, 0), copy(2, 1), copy(2, 2), copy(2, 3),
      copy(3, 0), copy(3, 1), copy(3, 2), copy(3, 3));
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Fill Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator = (FLOATTYPE fill_value) {
  fill(fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4)::
FLOATNAME(LMatrix4)(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02, FLOATTYPE e03,
		    FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12, FLOATTYPE e13,
		    FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22, FLOATTYPE e23,
		    FLOATTYPE e30, FLOATTYPE e31, FLOATTYPE e32, FLOATTYPE e33) {
  set(e00, e01, e02, e03,
      e10, e11, e12, e13,
      e20, e21, e22, e23,
      e30, e31, e32, e33);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Constructor, upper 3x3
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4)::
FLOATNAME(LMatrix4)(const FLOATNAME(LMatrix3) &upper3) {
  set(upper3(0, 0), upper3(0, 1), upper3(0, 2), 0.0,
      upper3(1, 0), upper3(1, 1), upper3(1, 2), 0.0,
      upper3(2, 0), upper3(2, 1), upper3(2, 2), 0.0,
      0.0, 0.0, 0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Constructor, upper 3x3 plus translation
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4)::
FLOATNAME(LMatrix4)(const FLOATNAME(LMatrix3) &upper3,
	 const FLOATNAME(LVecBase3) &trans) {
  set(upper3(0, 0), upper3(0, 1), upper3(0, 2), 0.0,
      upper3(1, 0), upper3(1, 1), upper3(1, 2), 0.0,
      upper3(2, 0), upper3(2, 1), upper3(2, 2), 0.0,
      trans[0], trans[1], trans[2], 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::fill
//       Access: Public
//  Description: Sets each element of the matrix to the indicated
//               fill_value.  This is of questionable value, but is
//               sometimes useful when initializing to zero.
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LMatrix4)::
fill(FLOATTYPE fill_value) {
  set(fill_value, fill_value, fill_value, fill_value,
      fill_value, fill_value, fill_value, fill_value,
      fill_value, fill_value, fill_value, fill_value,
      fill_value, fill_value, fill_value, fill_value);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::set
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LMatrix4)::
set(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02, FLOATTYPE e03,
    FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12, FLOATTYPE e13,
    FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22, FLOATTYPE e23,
    FLOATTYPE e30, FLOATTYPE e31, FLOATTYPE e32, FLOATTYPE e33) {
  (*this)(0, 0) = e00;
  (*this)(0, 1) = e01;
  (*this)(0, 2) = e02;
  (*this)(0, 3) = e03;

  (*this)(1, 0) = e10;
  (*this)(1, 1) = e11;
  (*this)(1, 2) = e12;
  (*this)(1, 3) = e13;

  (*this)(2, 0) = e20;
  (*this)(2, 1) = e21;
  (*this)(2, 2) = e22;
  (*this)(2, 3) = e23;

  (*this)(3, 0) = e30;
  (*this)(3, 1) = e31;
  (*this)(3, 2) = e32;
  (*this)(3, 3) = e33;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::set_upper_3
//       Access: Public
//  Description: Sets the upper 3x3 submatrix.
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LMatrix4)::
set_upper_3(const FLOATNAME(LMatrix3) &upper3) {
  (*this)(0, 0) = upper3(0, 0);
  (*this)(0, 1) = upper3(0, 1);
  (*this)(0, 2) = upper3(0, 2);

  (*this)(1, 0) = upper3(1, 0);
  (*this)(1, 1) = upper3(1, 1);
  (*this)(1, 2) = upper3(1, 2);

  (*this)(2, 0) = upper3(2, 0);
  (*this)(2, 1) = upper3(2, 1);
  (*this)(2, 2) = upper3(2, 2);
}
    
////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_upper_3
//       Access: Public
//  Description: Retrieves the upper 3x3 submatrix.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3) FLOATNAME(LMatrix4)::
get_upper_3() const {
  return FLOATNAME(LMatrix3)
    ((*this)(0, 0), (*this)(0, 1), (*this)(0, 2),
     (*this)(1, 0), (*this)(1, 1), (*this)(1, 2),
     (*this)(2, 0), (*this)(2, 1), (*this)(2, 2));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::set_row
//       Access: Public
//  Description: Replaces the indicated row of the matrix.
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LMatrix4)::
set_row(int row, const FLOATNAME(LVecBase4) &v) {
  (*this)(row, 0) = v[0];
  (*this)(row, 1) = v[1];
  (*this)(row, 2) = v[2];
  (*this)(row, 3) = v[3];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::set_col
//       Access: Public
//  Description: Replaces the indicated column of the matrix.
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LMatrix4)::
set_col(int col, const FLOATNAME(LVecBase4) &v) {
  (*this)(0, col) = v[0];
  (*this)(1, col) = v[1];
  (*this)(2, col) = v[2];
  (*this)(3, col) = v[3];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::set_row
//       Access: Public
//  Description: Replaces the indicated row of the matrix with the
//               indicated 3-component vector, ignoring the last
//               column.
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LMatrix4)::
set_row(int row, const FLOATNAME(LVecBase3) &v) {
  (*this)(row, 0) = v[0];
  (*this)(row, 1) = v[1];
  (*this)(row, 2) = v[2];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::set_col
//       Access: Public
//  Description: Replaces the indicated column of the matrix with the
//               indicated 3-component vector, ignoring the last
//               row.
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LMatrix4)::
set_col(int col, const FLOATNAME(LVecBase3) &v) {
  (*this)(0, col) = v[0];
  (*this)(1, col) = v[1];
  (*this)(2, col) = v[2];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_row
//       Access: Public
//  Description: Retrieves the indicated row of the matrix as a
//               4-component vector.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4) FLOATNAME(LMatrix4)::
get_row(int row) const {
  return FLOATNAME(LVecBase4)((*this)(row, 0),
			      (*this)(row, 1),
			      (*this)(row, 2),
			      (*this)(row, 3));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_col
//       Access: Public
//  Description: Retrieves the indicated column of the matrix as a
//               4-component vector.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4) FLOATNAME(LMatrix4)::
get_col(int col) const {
  return FLOATNAME(LVecBase4)((*this)(0, col),
			      (*this)(1, col),
			      (*this)(2, col),
			      (*this)(3, col));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_row3
//       Access: Public
//  Description: Retrieves the row column of the matrix as a
//               3-component vector, ignoring the last column.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3) FLOATNAME(LMatrix4)::
get_row3(int row) const {
  return FLOATNAME(LVecBase3)((*this)(row, 0),
			      (*this)(row, 1),
			      (*this)(row, 2));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_col3
//       Access: Public
//  Description: Retrieves the indicated column of the matrix as a
//               3-component vector, ignoring the last row.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3) FLOATNAME(LMatrix4)::
get_col3(int col) const {
  return FLOATNAME(LVecBase3)((*this)(0, col),
			      (*this)(1, col),
			      (*this)(2, col));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Indexing operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE &FLOATNAME(LMatrix4)::
operator () (int row, int col) {
  //  nassertr(row >= 0 && row < 4, _data[0]);
  //  nassertr(col >= 0 && col < 4, _data[0]);
  return _data[row * 4 + col];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Indexing operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LMatrix4)::
operator () (int row, int col) const {
  //  nassertr(row >= 0 && row < 4, 0.0);
  //  nassertr(col >= 0 && col < 4, 0.0);
  return _data[row * 4 + col];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::is_nan
//       Access: Public
//  Description: Returns true if any component of the matrix is
//               not-a-number, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LMatrix4)::
is_nan() const {
  return 
    cnan(_data[0]) || cnan(_data[1]) || cnan(_data[2]) || cnan(_data[3]) ||
    cnan(_data[4]) || cnan(_data[5]) || cnan(_data[6]) || cnan(_data[7]) || 
    cnan(_data[8]) || cnan(_data[9]) || cnan(_data[10]) || cnan(_data[11]) || 
    cnan(_data[12]) || cnan(_data[13]) || cnan(_data[14]) || cnan(_data[15]);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_cell
//       Access: Public
//  Description: Returns a particular element of the matrix.
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LMatrix4)::
get_cell(int row, int col) const {
//  nassertr(row >= 0 && row < 4, 0.0);
//  nassertr(col >= 0 && col < 4, 0.0);
  return _data[row * 4 + col];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::set_cell
//       Access: Public
//  Description: Changes a particular element of the matrix.
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LMatrix4)::
set_cell(int row, int col, FLOATTYPE value) {
//  nassertv(row >= 0 && row < 4);
//  nassertv(col >= 0 && col < 4);
  _data[row * 4 + col] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_data
//       Access: Public
//  Description: Returns the address of the first of the nine data
//               elements in the matrix.  The remaining elements
//               occupy the next eight positions in row-major order.
////////////////////////////////////////////////////////////////////
INLINE const FLOATTYPE *FLOATNAME(LMatrix4)::
get_data() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_num_components
//       Access: Public
//  Description: Returns the number of elements in the matrix, 16.
////////////////////////////////////////////////////////////////////
INLINE int FLOATNAME(LMatrix4)::
get_num_components() const {
  return 16;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4)::iterator FLOATNAME(LMatrix4)::
begin() {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4)::iterator FLOATNAME(LMatrix4)::
end() {
  return begin() + 16;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4)::const_iterator FLOATNAME(LMatrix4)::
begin() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4)::const_iterator FLOATNAME(LMatrix4)::
end() const {
  return begin() + 16;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Ordering Operator
//       Access: Public
//  Description: This performs a lexicographical comparison.  It's of
//               questionable mathematical meaning, but sometimes has
//               a practical purpose for sorting unique vectors,
//               especially in an STL container.  Also see
//               compare_to().
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LMatrix4)::
operator < (const FLOATNAME(LMatrix4) &other) const {
  return compare_to(other) < 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Equality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LMatrix4)::
operator == (const FLOATNAME(LMatrix4) &other) const {
  return compare_to(other) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LMatrix4)::
operator != (const FLOATNAME(LMatrix4) &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::compare_to
//       Access: Public
//  Description: This flavor of compare_to uses a default threshold
//               value based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE int FLOATNAME(LMatrix4)::
compare_to(const FLOATNAME(LMatrix4) &other) const {
  return compare_to(other, NEARLY_ZERO(FLOATTYPE));
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::xform
//       Access: Public
//  Description: 4-component vector or point times matrix.  This is a
//               fully general operation.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase4) FLOATNAME(LMatrix4)::
xform(const FLOATNAME(LVecBase4) &v) const {
  return FLOATNAME(LVecBase4)(v.dot(get_col(0)),
			      v.dot(get_col(1)),
			      v.dot(get_col(2)),
			      v.dot(get_col(3)));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::xform_point
//       Access: Public
//  Description: The matrix transforms a 3-component point (including
//               translation component) and returns the result.  This
//               assumes the matrix is an affine transform.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3) FLOATNAME(LMatrix4)::
xform_point(const FLOATNAME(LVecBase3) &v) const {
  return FLOATNAME(LVecBase3)(v.dot(get_col3(0)) + (*this)(3, 0),
			      v.dot(get_col3(1)) + (*this)(3, 1),
			      v.dot(get_col3(2)) + (*this)(3, 2));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::xform_vec
//       Access: Public
//  Description: The matrix transforms a 3-component vector (without
//               translation component) and returns the result.  This
//               assumes the matrix is an affine transform.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3) FLOATNAME(LMatrix4)::
xform_vec(const FLOATNAME(LVecBase3) &v) const {
  return FLOATNAME(LVecBase3)(v.dot(get_col3(0)),
			      v.dot(get_col3(1)),
			      v.dot(get_col3(2)));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::mult_cel
//       Access: Private
//  Description: Returns one cell of the result of a matrix-matrix
//               multiplication operation.
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LMatrix4)::
mult_cel(const FLOATNAME(LMatrix4) &other, int row, int col) const {
  return get_row(row).dot(other.get_col(col));
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix * matrix
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
operator * (const FLOATNAME(LMatrix4) &other) const {
  FLOATNAME(LMatrix4) t;

  t(0, 0) = mult_cel(other, 0, 0);
  t(0, 1) = mult_cel(other, 0, 1);
  t(0, 2) = mult_cel(other, 0, 2);
  t(0, 3) = mult_cel(other, 0, 3);

  t(1, 0) = mult_cel(other, 1, 0);
  t(1, 1) = mult_cel(other, 1, 1);
  t(1, 2) = mult_cel(other, 1, 2);
  t(1, 3) = mult_cel(other, 1, 3);

  t(2, 0) = mult_cel(other, 2, 0);
  t(2, 1) = mult_cel(other, 2, 1);
  t(2, 2) = mult_cel(other, 2, 2);
  t(2, 3) = mult_cel(other, 2, 3);

  t(3, 0) = mult_cel(other, 3, 0);
  t(3, 1) = mult_cel(other, 3, 1);
  t(3, 2) = mult_cel(other, 3, 2);
  t(3, 3) = mult_cel(other, 3, 3);

  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
operator * (FLOATTYPE scalar) const {
  FLOATNAME(LMatrix4) t;

  t(0, 0) = (*this)(0, 0) * scalar;
  t(0, 1) = (*this)(0, 1) * scalar;
  t(0, 2) = (*this)(0, 2) * scalar;
  t(0, 3) = (*this)(0, 3) * scalar;

  t(1, 0) = (*this)(1, 0) * scalar;
  t(1, 1) = (*this)(1, 1) * scalar;
  t(1, 2) = (*this)(1, 2) * scalar;
  t(1, 3) = (*this)(1, 3) * scalar;

  t(2, 0) = (*this)(2, 0) * scalar;
  t(2, 1) = (*this)(2, 1) * scalar;
  t(2, 2) = (*this)(2, 2) * scalar;
  t(2, 3) = (*this)(2, 3) * scalar;

  t(3, 0) = (*this)(3, 0) * scalar;
  t(3, 1) = (*this)(3, 1) * scalar;
  t(3, 2) = (*this)(3, 2) * scalar;
  t(3, 3) = (*this)(3, 3) * scalar;

  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
operator / (FLOATTYPE scalar) const {
  FLOATNAME(LMatrix4) t;

  t(0, 0) = (*this)(0, 0) / scalar;
  t(0, 1) = (*this)(0, 1) / scalar;
  t(0, 2) = (*this)(0, 2) / scalar;
  t(0, 3) = (*this)(0, 3) / scalar;

  t(1, 0) = (*this)(1, 0) / scalar;
  t(1, 1) = (*this)(1, 1) / scalar;
  t(1, 2) = (*this)(1, 2) / scalar;
  t(1, 3) = (*this)(1, 3) / scalar;

  t(2, 0) = (*this)(2, 0) / scalar;
  t(2, 1) = (*this)(2, 1) / scalar;
  t(2, 2) = (*this)(2, 2) / scalar;
  t(2, 3) = (*this)(2, 3) / scalar;

  t(3, 0) = (*this)(3, 0) / scalar;
  t(3, 1) = (*this)(3, 1) / scalar;
  t(3, 2) = (*this)(3, 2) / scalar;
  t(3, 3) = (*this)(3, 3) / scalar;

  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix += matrix
//       Access: Public
//  Description: Performs a memberwise addition between two matrices.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator += (const FLOATNAME(LMatrix4) &other) {
  (*this)(0, 0) += other(0, 0);
  (*this)(0, 1) += other(0, 1);
  (*this)(0, 2) += other(0, 2);
  (*this)(0, 3) += other(0, 3);

  (*this)(1, 0) += other(1, 0);
  (*this)(1, 1) += other(1, 1);
  (*this)(1, 2) += other(1, 2);
  (*this)(1, 3) += other(1, 3);

  (*this)(2, 0) += other(2, 0);
  (*this)(2, 1) += other(2, 1);
  (*this)(2, 2) += other(2, 2);
  (*this)(2, 3) += other(2, 3);

  (*this)(3, 0) += other(3, 0);
  (*this)(3, 1) += other(3, 1);
  (*this)(3, 2) += other(3, 2);
  (*this)(3, 3) += other(3, 3);

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix -= matrix
//       Access: Public
//  Description: Performs a memberwise addition between two matrices.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator -= (const FLOATNAME(LMatrix4) &other) {
  (*this)(0, 0) -= other(0, 0);
  (*this)(0, 1) -= other(0, 1);
  (*this)(0, 2) -= other(0, 2);
  (*this)(0, 3) -= other(0, 3);

  (*this)(1, 0) -= other(1, 0);
  (*this)(1, 1) -= other(1, 1);
  (*this)(1, 2) -= other(1, 2);
  (*this)(1, 3) -= other(1, 3);

  (*this)(2, 0) -= other(2, 0);
  (*this)(2, 1) -= other(2, 1);
  (*this)(2, 2) -= other(2, 2);
  (*this)(2, 3) -= other(2, 3);

  (*this)(3, 0) -= other(3, 0);
  (*this)(3, 1) -= other(3, 1);
  (*this)(3, 2) -= other(3, 2);
  (*this)(3, 3) -= other(3, 3);

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix *= matrix
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator *= (const FLOATNAME(LMatrix4) &other) {
  (*this) = (*this) * other;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix *= scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator *= (FLOATTYPE scalar) {
  (*this)(0, 0) *= scalar;
  (*this)(0, 1) *= scalar;
  (*this)(0, 2) *= scalar;
  (*this)(0, 3) *= scalar;

  (*this)(1, 0) *= scalar;
  (*this)(1, 1) *= scalar;
  (*this)(1, 2) *= scalar;
  (*this)(1, 3) *= scalar;

  (*this)(2, 0) *= scalar;
  (*this)(2, 1) *= scalar;
  (*this)(2, 2) *= scalar;
  (*this)(2, 3) *= scalar;

  (*this)(3, 0) *= scalar;
  (*this)(3, 1) *= scalar;
  (*this)(3, 2) *= scalar;
  (*this)(3, 3) *= scalar;

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix /= scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator /= (FLOATTYPE scalar) {
  (*this)(0, 0) /= scalar;
  (*this)(0, 1) /= scalar;
  (*this)(0, 2) /= scalar;
  (*this)(0, 3) /= scalar;

  (*this)(1, 0) /= scalar;
  (*this)(1, 1) /= scalar;
  (*this)(1, 2) /= scalar;
  (*this)(1, 3) /= scalar;

  (*this)(2, 0) /= scalar;
  (*this)(2, 1) /= scalar;
  (*this)(2, 2) /= scalar;
  (*this)(2, 3) /= scalar;

  (*this)(3, 0) /= scalar;
  (*this)(3, 1) /= scalar;
  (*this)(3, 2) /= scalar;
  (*this)(3, 3) /= scalar;

  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::transpose_from
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LMatrix4)::
transpose_from(const FLOATNAME(LMatrix4) &other) {
  (*this)(0, 0) = other(0, 0);
  (*this)(0, 1) = other(1, 0);
  (*this)(0, 2) = other(2, 0);
  (*this)(0, 3) = other(3, 0);

  (*this)(1, 0) = other(0, 1);
  (*this)(1, 1) = other(1, 1);
  (*this)(1, 2) = other(2, 1);
  (*this)(1, 3) = other(3, 1);

  (*this)(2, 0) = other(0, 2);
  (*this)(2, 1) = other(1, 2);
  (*this)(2, 2) = other(2, 2);
  (*this)(2, 3) = other(3, 2);

  (*this)(3, 0) = other(0, 3);
  (*this)(3, 1) = other(1, 3);
  (*this)(3, 2) = other(2, 3);
  (*this)(3, 3) = other(3, 3);
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::transpose_in_place
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LMatrix4)::
transpose_in_place() {
  FLOATNAME(LMatrix4) temp = (*this);
  transpose_from(temp);
}



////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::invert_from
//       Access: Public
//  Description: Computes the inverse of the other matrix, and stores
//               the result in this matrix.  This is a fully general
//               operation and makes no assumptions about the type of
//               transform represented by the matrix.
//
//               The other matrix must be a different object than this
//               matrix.  However, if you need to invert a matrix in
//               place, see invert_in_place.
//
//               The return value is true if the matrix was
//               successfully inverted, false if the was a
//               singularity.
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LMatrix4)::
invert_from(const FLOATNAME(LMatrix4) &other) {
  if (IS_NEARLY_EQUAL(other(3, 0), 0.0) &&
      IS_NEARLY_EQUAL(other(3, 1), 0.0) &&
      IS_NEARLY_EQUAL(other(3, 2), 0.0) &&
      IS_NEARLY_EQUAL(other(3, 3), 1.0)) {
    return invert_affine_from(other);
  }

  (*this) = other;
   
  int index[4];

  if (!decompose_mat(index)) {
    linmath_cat.warning()
      << "Tried to invert singular LMatrix4.\n";
    return false;
  }
  
  FLOATNAME(LMatrix4) inv = FLOATNAME(LMatrix4)::ident_mat();
  int row;
  
  for (row = 0; row < 4; row++) {
    back_sub_mat(index, inv, row);
  }
  
  transpose_from(inv);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::invert_affine_from
//       Access: Public
//  Description: Performs an invert of the indicated matrix, storing
//               the result in this matrix.  The calculation is only
//               correct of the other matrix represents an affine
//               transform.
//
//               The other matrix must be a different object than this
//               matrix.  However, if you need to invert a matrix in
//               place, see invert_in_place.
//
//               The return value is true if the matrix was
//               successfully inverted, false if the was a
//               singularity.
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LMatrix4)::
invert_affine_from(const FLOATNAME(LMatrix4) &other) {
  FLOATNAME(LMatrix3) rot;
  rot.invert_from(other.get_upper_3());

  set_upper_3(rot);
  set_col(3, FLOATNAME(LVecBase4)(0.0, 0.0, 0.0, 1.0));

  // compute -C*inv(A)
  for (int i = 0; i < 3; i++) {
    (*this)(3, i) = 0.0;
    for (int j = 0; j < 3; j++) {
      (*this)(3, i) -= other(3, j) * (*this)(j, i);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::invert_in_place
//       Access: Public
//  Description: Inverts the current matrix.  Returns true if the
//               inverse is successful, false if the matrix was
//               singular.
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LMatrix4)::
invert_in_place() {
  FLOATNAME(LMatrix4) temp = (*this);
  return invert_from(temp);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::translate_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               translation.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
translate_mat(const FLOATNAME(LVecBase3) &trans) {
  return FLOATNAME(LMatrix4)(1.0, 0.0, 0.0, 0.0,
			   0.0, 1.0, 0.0, 0.0,
			   0.0, 0.0, 1.0, 0.0,
			   trans[0], trans[1], trans[2], 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::translate_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               translation.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
translate_mat(FLOATTYPE tx, FLOATTYPE ty, FLOATTYPE tz) {
  return FLOATNAME(LMatrix4)(1.0, 0.0, 0.0, 0.0,
			   0.0, 1.0, 0.0, 0.0,
			   0.0, 0.0, 1.0, 0.0,
			   tx, ty, tz, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::rotate_mat
//       Access: Public, Static
//  Description: Returns a matrix that rotates by the given angle in
//               degrees counterclockwise about the indicated vector.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
rotate_mat(FLOATTYPE angle, FLOATNAME(LVecBase3) axis,
	   CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }
  FLOATNAME(LMatrix4) mat;

  if (!is_right_handed(cs)) {
    // In a left-handed coordinate system, counterclockwise is the
    // other direction.
    angle = -angle;
  }

  // Normalize the axis.
  FLOATTYPE length2 = axis.dot(axis);
  // Cannot rotate about a zero-length axis.
  nassertr(length2 != 0.0, ident_mat());
  FLOATTYPE recip_sqrt_length2=1.0f/csqrt(length2);
  axis *= recip_sqrt_length2;

  double angle_rad=deg_2_rad(angle);
  double s,c;
  csincos(angle_rad,&s,&c);
  double t = 1.0 - c;

  mat(0, 0) = t * axis[0] * axis[0] + c;
  mat(0, 1) = t * axis[0] * axis[1] + s * axis[2];
  mat(0, 2) = t * axis[0] * axis[2] - s * axis[1];
  mat(0, 3) = 0.0;

  mat(1, 0) = t * axis[1] * axis[0] - s * axis[2];
  mat(1, 1) = t * axis[1] * axis[1] + c;
  mat(1, 2) = t * axis[1] * axis[2] + s * axis[0];
  mat(1, 3) = 0.0;

  mat(2, 0) = t * axis[2] * axis[0] + s * axis[1];
  mat(2, 1) = t * axis[2] * axis[1] - s * axis[0];
  mat(2, 2) = t * axis[2] * axis[2] + c;
  mat(2, 3) = 0.0;

  mat(3, 0) = 0.0;
  mat(3, 1) = 0.0;
  mat(3, 2) = 0.0;
  mat(3, 3) = 1.0;

  return mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale in each of the three axes.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
scale_mat(const FLOATNAME(LVecBase3) &scale) {
  return FLOATNAME(LMatrix4)(scale[0], 0.0, 0.0, 0.0,
			   0.0, scale[1], 0.0, 0.0,
			   0.0, 0.0, scale[2], 0.0,
			   0.0, 0.0, 0.0, 1.0);
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale in each of the three axes.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
scale_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz) {
  return FLOATNAME(LMatrix4)(sx, 0.0, 0.0, 0.0,
			   0.0, sy, 0.0, 0.0,
			   0.0, 0.0, sz, 0.0,
			   0.0, 0.0, 0.0, 1.0);
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               uniform scale.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
scale_mat(FLOATTYPE scale) {
  return FLOATNAME(LMatrix4)(scale, 0.0, 0.0, 0.0,
			   0.0, scale, 0.0, 0.0,
			   0.0, 0.0, scale, 0.0,
			   0.0, 0.0, 0.0, 1.0);
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix::y_to_z_up_mat
//       Access: Public, Static
//  Description: Returns a matrix that transforms from the Y-up
//               coordinate system to the Z-up coordinate system.
////////////////////////////////////////////////////////////////////
INLINE const FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
y_to_z_up_mat() {
  return _y_to_z_up_mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::z_to_y_up_mat
//       Access: Public, Static
//  Description: Returns a matrix that transforms from the Y-up
//               coordinate system to the Z-up coordinate system.
////////////////////////////////////////////////////////////////////
INLINE const FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
z_to_y_up_mat() {
  return _z_to_y_up_mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::almost_equal
//       Access: Public
//  Description: Returns true if two matrices are memberwise equal
//               within a default tolerance based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LMatrix4)::
almost_equal(const FLOATNAME(LMatrix4) &other) const {
  return almost_equal(other, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LMatrix4)::
output(ostream &out) const {
  out << "[ " 
      << MAYBE_ZERO((*this)(0, 0)) << " "
      << MAYBE_ZERO((*this)(0, 1)) << " " 
      << MAYBE_ZERO((*this)(0, 2)) << " "
      << MAYBE_ZERO((*this)(0, 3))
      << " ] [ "
      << MAYBE_ZERO((*this)(1, 0)) << " "
      << MAYBE_ZERO((*this)(1, 1)) << " " 
      << MAYBE_ZERO((*this)(1, 2)) << " "
      << MAYBE_ZERO((*this)(1, 3))
      << " ] [ "
      << MAYBE_ZERO((*this)(2, 0)) << " "
      << MAYBE_ZERO((*this)(2, 1)) << " " 
      << MAYBE_ZERO((*this)(2, 2)) << " "
      << MAYBE_ZERO((*this)(2, 3))
      << " ] [ "
      << MAYBE_ZERO((*this)(3, 0)) << " "
      << MAYBE_ZERO((*this)(3, 1)) << " " 
      << MAYBE_ZERO((*this)(3, 2)) << " "
      << MAYBE_ZERO((*this)(3, 3))
      << " ]";
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LMatrix4)::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << MAYBE_ZERO((*this)(0, 0)) << " "
    << MAYBE_ZERO((*this)(0, 1)) << " " 
    << MAYBE_ZERO((*this)(0, 2)) << " "
    << MAYBE_ZERO((*this)(0, 3))
    << "\n";
  indent(out, indent_level)
    << MAYBE_ZERO((*this)(1, 0)) << " "
    << MAYBE_ZERO((*this)(1, 1)) << " " 
    << MAYBE_ZERO((*this)(1, 2)) << " "
    << MAYBE_ZERO((*this)(1, 3))
    << "\n";
  indent(out, indent_level)
    << MAYBE_ZERO((*this)(2, 0)) << " "
    << MAYBE_ZERO((*this)(2, 1)) << " " 
    << MAYBE_ZERO((*this)(2, 2)) << " "
    << MAYBE_ZERO((*this)(2, 3))
    << "\n";
  indent(out, indent_level)
    << MAYBE_ZERO((*this)(3, 0)) << " "
    << MAYBE_ZERO((*this)(3, 1)) << " " 
    << MAYBE_ZERO((*this)(3, 2)) << " "
    << MAYBE_ZERO((*this)(3, 3))
    << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::transpose
//  Description: Transposes the given matrix and returns it.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4)
transpose(const FLOATNAME(LMatrix4) &a) {
  FLOATNAME(LMatrix4) result;
  result.transpose_from(a);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::invert
//  Description: Inverts the given matrix and returns it.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix4)
invert(const FLOATNAME(LMatrix4) &a) {
  FLOATNAME(LMatrix4) result;
  bool nonsingular = result.invert_from(a);
  nassertr(nonsingular, FLOATNAME(LMatrix4)::ident_mat());
  return result;
}
