// Filename: lmatrix.I
// Created by:  drose (15Jan99)
// 
////////////////////////////////////////////////////////////////////

#include <pandabase.h>

#include "deg_2_rad.h"
#include "nearly_zero.h"
#include "config_linmath.h"

#include <indent.h>

#include <math.h>

template<class NumType>
TypeHandle LMatrix4<NumType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix4<NumType>::
LMatrix4() { 
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix4<NumType>:: 
LMatrix4(const LMatrix4<NumType> &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix4<NumType> &LMatrix4<NumType>:: 
operator = (const LMatrix4<NumType> &copy) {
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
template<class NumType>
INLINE LMatrix4<NumType> &LMatrix4<NumType>::
operator = (NumType fill_value) {
  fill(fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix4<NumType>::
LMatrix4(NumType e00, NumType e01, NumType e02, NumType e03,
	 NumType e10, NumType e11, NumType e12, NumType e13,
	 NumType e20, NumType e21, NumType e22, NumType e23,
	 NumType e30, NumType e31, NumType e32, NumType e33) {
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
template<class NumType>
LMatrix4<NumType>::
LMatrix4(const LMatrix3<NumType> &upper3) {
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
template<class NumType>
LMatrix4<NumType>::
LMatrix4(const LMatrix3<NumType> &upper3,
	 const LVecBase3<NumType> &trans) {
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
template<class NumType>
void LMatrix4<NumType>::
fill(NumType fill_value) {
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
template<class NumType>
INLINE void LMatrix4<NumType>::
set(NumType e00, NumType e01, NumType e02, NumType e03,
    NumType e10, NumType e11, NumType e12, NumType e13,
    NumType e20, NumType e21, NumType e22, NumType e23,
    NumType e30, NumType e31, NumType e32, NumType e33) {
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
template<class NumType>
INLINE void LMatrix4<NumType>::
set_upper_3(const LMatrix3<NumType> &upper3) {
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
template<class NumType>
INLINE LMatrix3<NumType> LMatrix4<NumType>::
get_upper_3() const {
  return LMatrix3<NumType>
    ((*this)(0, 0), (*this)(0, 1), (*this)(0, 2),
     (*this)(1, 0), (*this)(1, 1), (*this)(1, 2),
     (*this)(2, 0), (*this)(2, 1), (*this)(2, 2));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::set_row
//       Access: Public
//  Description: Replaces the indicated row of the matrix.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LMatrix4<NumType>::
set_row(int row, const LVecBase4<NumType> &v) {
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
template<class NumType>
INLINE void LMatrix4<NumType>::
set_col(int col, const LVecBase4<NumType> &v) {
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
template<class NumType>
INLINE void LMatrix4<NumType>::
set_row(int row, const LVecBase3<NumType> &v) {
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
template<class NumType>
INLINE void LMatrix4<NumType>::
set_col(int col, const LVecBase3<NumType> &v) {
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
template<class NumType>
INLINE LVecBase4<NumType> LMatrix4<NumType>::
get_row(int row) const {
  return LVecBase4<NumType>((*this)(row, 0),
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
template<class NumType>
INLINE LVecBase4<NumType> LMatrix4<NumType>::
get_col(int col) const {
  return LVecBase4<NumType>((*this)(0, col),
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
template<class NumType>
INLINE LVecBase3<NumType> LMatrix4<NumType>::
get_row3(int row) const {
  return LVecBase3<NumType>((*this)(row, 0),
			    (*this)(row, 1),
			    (*this)(row, 2));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_col3
//       Access: Public
//  Description: Retrieves the indicated column of the matrix as a
//               3-component vector, ignoring the last row.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase3<NumType> LMatrix4<NumType>::
get_col3(int col) const {
  return LVecBase3<NumType>((*this)(0, col),
			    (*this)(1, col),
			    (*this)(2, col));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Indexing operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType &LMatrix4<NumType>::
operator () (int row, int col) {
  nassertr(row >= 0 && row < 4, _data[0]);
  nassertr(col >= 0 && col < 4, _data[0]);
  return _data[row * 4 + col];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Indexing operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LMatrix4<NumType>::
operator () (int row, int col) const {
  nassertr(row >= 0 && row < 4, 0.0);
  nassertr(col >= 0 && col < 4, 0.0);
  return _data[row * 4 + col];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::is_nan
//       Access: Public
//  Description: Returns true if any component of the matrix is
//               not-a-number, false otherwise.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LMatrix4<NumType>::
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
template<class NumType>
INLINE NumType LMatrix4<NumType>::
get_cell(int row, int col) const {
  nassertr(row >= 0 && row < 4, 0.0);
  nassertr(col >= 0 && col < 4, 0.0);
  return _data[row * 4 + col];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::set_cell
//       Access: Public
//  Description: Changes a particular element of the matrix.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LMatrix4<NumType>::
set_cell(int row, int col, NumType value) {
  nassertv(row >= 0 && row < 4);
  nassertv(col >= 0 && col < 4);
  _data[row * 4 + col] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_data
//       Access: Public
//  Description: Returns the address of the first of the nine data
//               elements in the matrix.  The remaining elements
//               occupy the next eight positions in row-major order.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE const NumType *LMatrix4<NumType>::
get_data() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_num_components
//       Access: Public
//  Description: Returns the number of elements in the matrix, 16.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE int LMatrix4<NumType>::
get_num_components() const {
  return 16;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix4<NumType>::iterator LMatrix4<NumType>::
begin() {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix4<NumType>::iterator LMatrix4<NumType>::
end() {
  return begin() + 16;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix4<NumType>::const_iterator LMatrix4<NumType>::
begin() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix4<NumType>::const_iterator LMatrix4<NumType>::
end() const {
  return begin() + 16;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Equality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
bool LMatrix4<NumType>::
operator == (const LMatrix4<NumType> &other) const {
  return ((*this)(0, 0) == other(0, 0) &&
	  (*this)(0, 1) == other(0, 1) &&
	  (*this)(0, 2) == other(0, 2) &&
	  (*this)(0, 3) == other(0, 3) &&
	  (*this)(1, 0) == other(1, 0) &&
	  (*this)(1, 1) == other(1, 1) &&
	  (*this)(1, 2) == other(1, 2) &&
	  (*this)(1, 3) == other(1, 3) &&
	  (*this)(2, 0) == other(2, 0) &&
	  (*this)(2, 1) == other(2, 1) &&
	  (*this)(2, 2) == other(2, 2) &&
	  (*this)(2, 3) == other(2, 3) &&
	  (*this)(3, 0) == other(3, 0) &&
	  (*this)(3, 1) == other(3, 1) &&
	  (*this)(3, 2) == other(3, 2) &&
	  (*this)(3, 3) == other(3, 3));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LMatrix4<NumType>::
operator != (const LMatrix4<NumType> &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::compare_to
//       Access: Public
//  Description: This flavor of compare_to uses a default threshold
//               value based on the numeric type.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE int LMatrix4<NumType>::
compare_to(const LMatrix4<NumType> &other) const {
  return compare_to(other, NEARLY_ZERO(NumType));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::compare_to
//       Access: Public
//  Description: Sorts matrices lexicographically, componentwise.
//               Returns a number less than 0 if this matrix sorts
//               before the other one, greater than zero if it sorts
//               after, 0 if they are equivalent (within the indicated
//               tolerance).
////////////////////////////////////////////////////////////////////
template<class NumType>
int LMatrix4<NumType>::
compare_to(const LMatrix4<NumType> &other, NumType threshold) const {
  for (int i = 0; i < 16; i++) {
    if (!IS_THRESHOLD_EQUAL(_data[i], other._data[i], threshold)) {
      return (_data[i] < other._data[i]) ? -1 : 1;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::xform
//       Access: Public
//  Description: 4-component vector or point times matrix.  This is a
//               fully general operation.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase4<NumType> LMatrix4<NumType>::
xform(const LVecBase4<NumType> &v) const {
  return LVecBase4<NumType>(v.dot(get_col(0)),
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
template<class NumType>
INLINE LVecBase3<NumType> LMatrix4<NumType>::
xform_point(const LVecBase3<NumType> &v) const {
  return LVecBase3<NumType>(v.dot(get_col3(0)) + (*this)(3, 0),
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
template<class NumType>
INLINE LVecBase3<NumType> LMatrix4<NumType>::
xform_vec(const LVecBase3<NumType> &v) const {
  return LVecBase3<NumType>(v.dot(get_col3(0)),
			    v.dot(get_col3(1)),
			    v.dot(get_col3(2)));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::mult_cel
//       Access: Private
//  Description: Returns one cell of the result of a matrix-matrix
//               multiplication operation.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LMatrix4<NumType>::
mult_cel(const LMatrix4<NumType> &other, int row, int col) const {
  return get_row(row).dot(other.get_col(col));
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix * matrix
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix4<NumType> LMatrix4<NumType>::
operator * (const LMatrix4<NumType> &other) const {
  LMatrix4<NumType> t;

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
template<class NumType>
LMatrix4<NumType> LMatrix4<NumType>::
operator * (NumType scalar) const {
  LMatrix4<NumType> t;

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
template<class NumType>
LMatrix4<NumType> LMatrix4<NumType>::
operator / (NumType scalar) const {
  LMatrix4<NumType> t;

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
template<class NumType>
LMatrix4<NumType> &LMatrix4<NumType>::
operator += (const LMatrix4<NumType> &other) {
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
template<class NumType>
LMatrix4<NumType> &LMatrix4<NumType>::
operator -= (const LMatrix4<NumType> &other) {
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
template<class NumType>
INLINE LMatrix4<NumType> &LMatrix4<NumType>::
operator *= (const LMatrix4<NumType> &other) {
  (*this) = (*this) * other;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix *= scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix4<NumType> &LMatrix4<NumType>::
operator *= (NumType scalar) {
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
template<class NumType>
LMatrix4<NumType> &LMatrix4<NumType>::
operator /= (NumType scalar) {
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
template<class NumType>
void LMatrix4<NumType>::
transpose_from(const LMatrix4<NumType> &other) {
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
template<class NumType>
INLINE void LMatrix4<NumType>::
transpose_in_place() {
  LMatrix4<NumType> temp = (*this);
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
template<class NumType>
bool LMatrix4<NumType>::
invert_from(const LMatrix4<NumType> &other) {
  if (IS_NEARLY_EQUAL(other(3, 0), 0.0) &&
      IS_NEARLY_EQUAL(other(3, 1), 0.0) &&
      IS_NEARLY_EQUAL(other(3, 2), 0.0) &&
      IS_NEARLY_EQUAL(other(3, 3), 1.0)) {
    return invert_affine_from(other);
  }

  (*this) = other;
   
  int index[4];

  if (!decompose_mat(index)) {
    nout << "Tried to invert singular LMatrix4.\n";
    return false;
  }
  
  LMatrix4<NumType> inv = LMatrix4<NumType>::ident_mat();
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
template<class NumType>
bool LMatrix4<NumType>::
invert_affine_from(const LMatrix4<NumType> &other) {
  LMatrix3<NumType> rot;
  rot.invert_from(other.get_upper_3());

  set_upper_3(rot);
  set_col(3, LVecBase4<NumType>(0.0, 0.0, 0.0, 1.0));

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
template<class NumType>
INLINE bool LMatrix4<NumType>::
invert_in_place() {
  LMatrix4<NumType> temp = (*this);
  return invert_from(temp);
}






////////////////////////////////////////////////////////////////////
//     Function: LMatrix::ident_mat
//       Access: Public, Static
//  Description: Returns an identity matrix.
////////////////////////////////////////////////////////////////////
template<class NumType>
const LMatrix4<NumType> &LMatrix4<NumType>::
ident_mat() {
  static LMatrix4<NumType> mat(1.0, 0.0, 0.0, 0.0,
			       0.0, 1.0, 0.0, 0.0,
			       0.0, 0.0, 1.0, 0.0,
			       0.0, 0.0, 0.0, 1.0);
  return mat;
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix::translate_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               translation.
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix4<NumType> LMatrix4<NumType>::
translate_mat(const LVecBase3<NumType> &trans) {
  return LMatrix4<NumType>(1.0, 0.0, 0.0, 0.0,
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
template<class NumType>
LMatrix4<NumType> LMatrix4<NumType>::
translate_mat(NumType tx, NumType ty, NumType tz) {
  return LMatrix4<NumType>(1.0, 0.0, 0.0, 0.0,
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
template<class NumType>
LMatrix4<NumType> LMatrix4<NumType>::
rotate_mat(NumType angle, LVecBase3<NumType> axis,
	   CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }
  LMatrix4<NumType> mat;

  if (!is_right_handed(cs)) {
    // In a left-handed coordinate system, counterclockwise is the
    // other direction.
    angle = -angle;
  }

  // Normalize the axis.
  NumType length = axis.dot(axis);
  nassertr(length != 0.0, ident_mat());
  axis /= length;
    
  double s = sin(deg_2_rad(angle));
  double c = cos(deg_2_rad(angle));
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
template<class NumType>
LMatrix4<NumType> LMatrix4<NumType>::
scale_mat(const LVecBase3<NumType> &scale) {
  return LMatrix4<NumType>(scale[0], 0.0, 0.0, 0.0,
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
template<class NumType>
LMatrix4<NumType> LMatrix4<NumType>::
scale_mat(NumType sx, NumType sy, NumType sz) {
  return LMatrix4<NumType>(sx, 0.0, 0.0, 0.0,
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
template<class NumType>
LMatrix4<NumType> LMatrix4<NumType>::
scale_mat(NumType scale) {
  return LMatrix4<NumType>(scale, 0.0, 0.0, 0.0,
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
template<class NumType>
const LMatrix4<NumType> &LMatrix4<NumType>::
y_to_z_up_mat() {
  static LMatrix4<NumType> mat(1.0, 0.0, 0.0, 0.0,
			       0.0, 0.0, 1.0, 0.0,
			       0.0,-1.0, 0.0, 0.0,
			       0.0, 0.0, 0.0, 1.0);
  return mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::z_to_y_up_mat
//       Access: Public, Static
//  Description: Returns a matrix that transforms from the Y-up
//               coordinate system to the Z-up coordinate system.
////////////////////////////////////////////////////////////////////
template<class NumType>
const LMatrix4<NumType> &LMatrix4<NumType>::
z_to_y_up_mat() {
  static LMatrix4<NumType> mat(1.0, 0.0, 0.0, 0.0,
			       0.0, 0.0,-1.0, 0.0,
			       0.0, 1.0, 0.0, 0.0,
			       0.0, 0.0, 0.0, 1.0);
  return mat;
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix::convert_mat
//       Access: Public, Static
//  Description: Returns a matrix that transforms from the indicated
//               coordinate system to the indicated coordinate system.
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix4<NumType> LMatrix4<NumType>::
convert_mat(CoordinateSystem from, CoordinateSystem to) {
  if (from == CS_default) {
    from = default_coordinate_system;
  }
  if (to == CS_default) {
    to = default_coordinate_system;
  }
  switch (from) {
  case CS_zup_left:
    switch (to) {
    case CS_zup_left: return ident_mat();
    case CS_yup_left: return z_to_y_up_mat();
    case CS_zup_right: return scale_mat(1.0, -1.0, 1.0);
    case CS_yup_right: return scale_mat(1.0, -1.0, 1.0) * z_to_y_up_mat();
    default: break;
    }
    break;

  case CS_yup_left:
    switch (to) {
    case CS_zup_left: return y_to_z_up_mat();
    case CS_yup_left: return ident_mat();
    case CS_zup_right: return scale_mat(1.0, 1.0, -1.0) * y_to_z_up_mat();
    case CS_yup_right: return scale_mat(1.0, 1.0, -1.0);
    default: break;
    }
    break;

  case CS_zup_right:
    switch (to) {
    case CS_zup_left: return scale_mat(1.0, -1.0, 1.0);
    case CS_yup_left: return scale_mat(1.0, -1.0, 1.0) * z_to_y_up_mat();
    case CS_zup_right: return ident_mat();
    case CS_yup_right: return z_to_y_up_mat();
    default: break;
    }
    break;

  case CS_yup_right:
    switch (to) {
    case CS_zup_left: return scale_mat(1.0, 1.0, -1.0) * y_to_z_up_mat();
    case CS_yup_left: return scale_mat(1.0, 1.0, -1.0);
    case CS_zup_right: return y_to_z_up_mat();
    case CS_yup_right: return ident_mat();
    default: break;
    }
    break;

  default:
    break;
  }

  linmath_cat.error()
    << "Invalid coordinate system value!\n";
  return ident_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::almost_equal
//       Access: Public
//  Description: Returns true if two matrices are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////
template<class NumType>
bool LMatrix4<NumType>::
almost_equal(const LMatrix4<NumType> &other, NumType threshold) const {
  return (IS_THRESHOLD_EQUAL((*this)(0, 0), other(0, 0), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(0, 1), other(0, 1), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(0, 2), other(0, 2), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(0, 3), other(0, 3), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(1, 0), other(1, 0), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(1, 1), other(1, 1), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(1, 2), other(1, 2), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(1, 3), other(1, 3), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(2, 0), other(2, 0), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(2, 1), other(2, 1), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(2, 2), other(2, 2), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(2, 3), other(2, 3), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(3, 0), other(3, 0), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(3, 1), other(3, 1), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(3, 2), other(3, 2), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(3, 3), other(3, 3), threshold));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::almost_equal
//       Access: Public
//  Description: Returns true if two matrices are memberwise equal
//               within a default tolerance based on the numeric type.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LMatrix4<NumType>::
almost_equal(const LMatrix4<NumType> &other) const {
  return almost_equal(other, NEARLY_ZERO(NumType));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LMatrix4<NumType>::
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
template<class NumType>
INLINE void LMatrix4<NumType>::
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
//     Function: LMatrix4::decompose_mat
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
bool LMatrix4<NumType>::
decompose_mat(int index[4]) {
  int i, j, k;
  NumType vv[4];
  for (i = 0; i < 4; i++) {
    NumType big = 0.0;
    for (j = 0; j < 4; j++) {
      NumType temp = fabs((*this)(i,j));
      if (temp > big) {
	big = temp;
      }
    }

    if (IS_NEARLY_ZERO(big)) {
      return false;
    }
    vv[i] = 1.0 / big;
  }

  for (j = 0; j < 4; j++) {
    for (i = 0; i < j; i++) {
      NumType sum = (*this)(i,j);
      for (k = 0; k < i; k++) {
	sum -= (*this)(i,k) * (*this)(k,j);
      }
      (*this)(i,j) = sum;
    }

    NumType big = 0.0;
    int imax = -1;
    for (i = j; i < 4; i++) {
      NumType sum = (*this)(i,j);
      for (k = 0; k < j; k++) {
	sum -= (*this)(i,k) * (*this)(k,j);
      }
      (*this)(i,j) = sum;

      NumType dum = vv[i] * fabs(sum);
      if (dum >= big) {
	big = dum;
	imax = i;
      }
    }
    nassertr(imax >= 0, false);
    if (j != imax) {
      for (k = 0; k < 4; k++) {
	NumType dum = (*this)(imax,k);
	(*this)(imax,k) = (*this)(j,k);
	(*this)(j,k) = dum;
      }
      vv[imax] = vv[j];
    }
    index[j] = imax;

    if ((*this)(j,j) == 0.0) {
      (*this)(j,j) = NEARLY_ZERO(NumType);
    }

    if (j != 4 - 1) {
      NumType dum = 1.0 / (*this)(j,j);
      for (i = j + 1; i < 4; i++) {
	(*this)(i,j) *= dum;
      }
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::back_sub_mat
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
bool LMatrix4<NumType>::
back_sub_mat(int index[4], LMatrix4<NumType> &inv, int row) const {
  int ii = -1;
  int i, j;
  for (i = 0; i < 4; i++) {
    int ip = index[i];
    NumType sum = inv(row, ip);
    inv(row, ip) = inv(row, i);
    if (ii >= 0) {
      for (j = ii; j <= i - 1; j++) {
	sum -= (*this)(i,j) * inv(row, j);
      }
    } else if (sum) {
      ii = i;
    }

    inv(row, i) = sum;
  }

  for (i = 4 - 1; i >= 0; i--) {
    NumType sum = inv(row, i);
    for (j = i + 1; j < 4; j++) {
      sum -= (*this)(i,j) * inv(row, j);
    }
    inv(row, i) = sum / (*this)(i,i);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void LMatrix4<NumType>::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    do_init_type(NumType);
    string name =
      "LMatrix4<" + get_type_handle(NumType).get_name() + ">";
    register_type(_type_handle, name);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::write_datagram
//  Description: Writes the matrix to the datagram
////////////////////////////////////////////////////////////////////
template<class NumType>
void LMatrix4<NumType>::
write_datagram(Datagram &destination) const
{
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      destination.add_float64(get_cell(i,j));
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::read_datagram
//  Description: Reads itself out of the datagram
////////////////////////////////////////////////////////////////////
template<class NumType>
void LMatrix4<NumType>::
read_datagram(DatagramIterator &scan) 
{
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      set_cell(i, j, scan.get_float64());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::transpose
//  Description: Transposes the given matrix and returns it.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix4<NumType>
transpose(const LMatrix4<NumType> &a) {
  LMatrix4<NumType> result;
  result.transpose_from(a);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::invert
//  Description: Inverts the given matrix and returns it.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix4<NumType>
invert(const LMatrix4<NumType> &a) {
  LMatrix4<NumType> result;
  bool nonsingular = result.invert_from(a);
  nassertr(nonsingular, LMatrix4<NumType>::ident_mat());
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a matrix from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////
template<class NumType, class NumType2>
INLINE LMatrix4<NumType2> 
lcast_to(NumType2 *, const LMatrix4<NumType> &source) {
  return LMatrix4<NumType2>
    (source(0, 0), source(0, 1), source(0, 2), source(0, 3),
     source(1, 0), source(1, 1), source(1, 2), source(1, 3),
     source(2, 0), source(2, 1), source(2, 2), source(2, 3),
     source(3, 0), source(3, 1), source(3, 2), source(3, 3));
}
