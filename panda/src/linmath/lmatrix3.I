// Filename: lmatrix3.I
// Created by:  drose (29Jan99)
// 
////////////////////////////////////////////////////////////////////

#include <pandabase.h>

#include "deg_2_rad.h"
#include "nearly_zero.h"

#include <notify.h>
#include <indent.h>

#include <math.h>

template<class NumType>
TypeHandle LMatrix3<NumType>::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix3<NumType>:: 
LMatrix3() {
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix3<NumType>:: 
LMatrix3(const LMatrix3<NumType> &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> &LMatrix3<NumType>:: 
operator = (const LMatrix3<NumType> &copy) {
  set(copy(0, 0), copy(0, 1), copy(0, 2),
      copy(1, 0), copy(1, 1), copy(1, 2),
      copy(2, 0), copy(2, 1), copy(2, 2));
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Fill Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix3<NumType> &LMatrix3<NumType>::
operator = (NumType fill_value) {
  fill(fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix3<NumType>:: 
LMatrix3(NumType e00, NumType e01, NumType e02,
	 NumType e10, NumType e11, NumType e12,
	 NumType e20, NumType e21, NumType e22) {
  set(e00, e01, e02,
      e10, e11, e12,
      e20, e21, e22);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::fill
//       Access: Public
//  Description: Sets each element of the matrix to the indicated
//               fill_value.  This is of questionable value, but is
//               sometimes useful when initializing to zero.
////////////////////////////////////////////////////////////////////
template<class NumType>
void LMatrix3<NumType>::
fill(NumType fill_value) {
  set(fill_value, fill_value, fill_value,
      fill_value, fill_value, fill_value,
      fill_value, fill_value, fill_value);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::set
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LMatrix3<NumType>::
set(NumType e00, NumType e01, NumType e02,
    NumType e10, NumType e11, NumType e12,
    NumType e20, NumType e21, NumType e22) {
  (*this)(0, 0) = e00;
  (*this)(0, 1) = e01;
  (*this)(0, 2) = e02;
  (*this)(1, 0) = e10;
  (*this)(1, 1) = e11;
  (*this)(1, 2) = e12;
  (*this)(2, 0) = e20;
  (*this)(2, 1) = e21;
  (*this)(2, 2) = e22;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::set_row
//       Access: Public
//  Description: Replaces the indicated row of the matrix from a
//               three-component vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LMatrix3<NumType>::
set_row(int row, const LVecBase3<NumType> &v) {
  (*this)(row, 0) = v[0];
  (*this)(row, 1) = v[1];
  (*this)(row, 2) = v[2];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::set_column
//       Access: Public
//  Description: Replaces the indicated column of the matrix from a
//               three-component vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LMatrix3<NumType>::
set_col(int col, const LVecBase3<NumType> &v) {
  (*this)(0, col) = v[0];
  (*this)(1, col) = v[1];
  (*this)(2, col) = v[2];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::set_row
//       Access: Public
//  Description: Replaces the indicated row of the matrix from a
//               two-component vector, ignoring the last column.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LMatrix3<NumType>::
set_row(int row, const LVecBase2<NumType> &v) {
  (*this)(row, 0) = v[0];
  (*this)(row, 1) = v[1];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::set_column
//       Access: Public
//  Description: Replaces the indicated column of the matrix from a
//               two-component vector, ignoring the last row.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LMatrix3<NumType>::
set_col(int col, const LVecBase2<NumType> &v) {
  (*this)(0, col) = v[0];
  (*this)(1, col) = v[1];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_row
//       Access: Public
//  Description: Returns the indicated row of the matrix as a
//               three-component vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase3<NumType> LMatrix3<NumType>::
get_row(int row) const {
  return LVecBase3<NumType>((*this)(row, 0), (*this)(row, 1), (*this)(row, 2));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_col
//       Access: Public
//  Description: Returns the indicated column of the matrix as a
//               three-component vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase3<NumType> LMatrix3<NumType>::
get_col(int col) const {
  return LVecBase3<NumType>((*this)(0, col), (*this)(1, col), (*this)(2, col));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_row2
//       Access: Public
//  Description: Returns the indicated row of the matrix as a
//               two-component vector, ignoring the last column.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> LMatrix3<NumType>::
get_row2(int row) const {
  return LVecBase2<NumType>((*this)(row, 0), (*this)(row, 1));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_col2
//       Access: Public
//  Description: Returns the indicated column of the matrix as a
//               two-component vector, ignoring the last row.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> LMatrix3<NumType>::
get_col2(int col) const {
  return LVecBase2<NumType>((*this)(0, col), (*this)(1, col));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Indexing operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType &LMatrix3<NumType>::
operator () (int row, int col) {
  nassertr(row >= 0 && row < 3, _data[0]);
  nassertr(col >= 0 && col < 3, _data[0]);
  return _data[row * 3 + col];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Indexing operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LMatrix3<NumType>::
operator () (int row, int col) const {
  nassertr(row >= 0 && row < 3, 0.0);
  nassertr(col >= 0 && col < 3, 0.0);
  return _data[row * 3 + col];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::is_nan
//       Access: Public
//  Description: Returns true if any component of the matrix is
//               not-a-number, false otherwise.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LMatrix3<NumType>::
is_nan() const {
  return 
    cnan(_data[0]) || cnan(_data[1]) || cnan(_data[2]) ||
    cnan(_data[3]) || cnan(_data[4]) || cnan(_data[5]) ||
    cnan(_data[6]) || cnan(_data[7]) || cnan(_data[8]);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_cell
//       Access: Public
//  Description: Returns a particular element of the matrix.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LMatrix3<NumType>::
get_cell(int row, int col) const {
  nassertr(row >= 0 && row < 3, 0.0);
  nassertr(col >= 0 && col < 3, 0.0);
  return _data[row * 3 + col];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::set_cell
//       Access: Public
//  Description: Changes a particular element of the matrix.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LMatrix3<NumType>::
set_cell(int row, int col, NumType value) {
  nassertv(row >= 0 && row < 3);
  nassertv(col >= 0 && col < 3);
  _data[row * 3 + col] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_data
//       Access: Public
//  Description: Returns the address of the first of the nine data
//               elements in the matrix.  The remaining elements
//               occupy the next eight positions in row-major order.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE const NumType *LMatrix3<NumType>::
get_data() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_num_components
//       Access: Public
//  Description: Returns the number of elements in the matrix, nine.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE int LMatrix3<NumType>::
get_num_components() const {
  return 9;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix3<NumType>::iterator LMatrix3<NumType>::
begin() {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix3<NumType>::iterator LMatrix3<NumType>::
end() {
  return begin() + get_num_components();
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix3<NumType>::const_iterator LMatrix3<NumType>::
begin() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix3<NumType>::const_iterator LMatrix3<NumType>::
end() const {
  return begin() + get_num_components();
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Equality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
bool LMatrix3<NumType>::
operator == (const LMatrix3<NumType> &other) const {
  return ((*this)(0, 0) == other(0, 0) &&
	  (*this)(0, 1) == other(0, 1) &&
	  (*this)(0, 2) == other(0, 2) &&
	  (*this)(1, 0) == other(1, 0) &&
	  (*this)(1, 1) == other(1, 1) &&
	  (*this)(1, 2) == other(1, 2) &&
	  (*this)(2, 0) == other(2, 0) &&
	  (*this)(2, 1) == other(2, 1) &&
	  (*this)(2, 2) == other(2, 2));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LMatrix3<NumType>::
operator != (const LMatrix3<NumType> &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::compare_to
//       Access: Public
//  Description: This flavor of compare_to uses a default threshold
//               value based on the numeric type.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE int LMatrix3<NumType>::
compare_to(const LMatrix3<NumType> &other) const {
  return compare_to(other, NEARLY_ZERO(NumType));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::compare_to
//       Access: Public
//  Description: Sorts matrices lexicographically, componentwise.
//               Returns a number less than 0 if this matrix sorts
//               before the other one, greater than zero if it sorts
//               after, 0 if they are equivalent (within the indicated
//               tolerance).
////////////////////////////////////////////////////////////////////
template<class NumType>
int LMatrix3<NumType>::
compare_to(const LMatrix3<NumType> &other, NumType threshold) const {
  for (int i = 0; i < 9; i++) {
    if (!IS_THRESHOLD_EQUAL(_data[i], other._data[i], threshold)) {
      return (_data[i] < other._data[i]) ? -1 : 1;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::xform
//       Access: Public
//  Description: 3-component vector or point times matrix.  This is a
//               fully general operation.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase3<NumType> LMatrix3<NumType>::
xform(const LVecBase3<NumType> &v) const {
  return LVecBase3<NumType>(v.dot(get_col(0)),
			    v.dot(get_col(1)),
			    v.dot(get_col(2)));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::xform_point
//       Access: Public
//  Description: The matrix transforms a 2-component point (including
//               translation component) and returns the result.  This
//               assumes the matrix is an affine transform.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> LMatrix3<NumType>::
xform_point(const LVecBase2<NumType> &v) const {
  return LVecBase2<NumType>(v.dot(get_col2(0)) + (*this)(2, 0),
			    v.dot(get_col2(1)) + (*this)(2, 1));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::xform_vec
//       Access: Public
//  Description: The matrix transforms a 2-component vector (without
//               translation component) and returns the result.  This
//               assumes the matrix is an affine transform.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LVecBase2<NumType> LMatrix3<NumType>::
xform_vec(const LVecBase2<NumType> &v) const {
  return LVecBase2<NumType>(v.dot(get_col2(0)),
			    v.dot(get_col2(1)));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::mult_cel
//       Access: Private
//  Description: Returns one cell of the result of a matrix-matrix
//               multiplication operation.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LMatrix3<NumType>::
mult_cel(const LMatrix3<NumType> &other, int row, int col) const {
  return get_row(row).dot(other.get_col(col));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix * matrix
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> LMatrix3<NumType>::
operator * (const LMatrix3<NumType> &other) const {
  LMatrix3<NumType> t;

  t(0, 0) = mult_cel(other, 0, 0);
  t(0, 1) = mult_cel(other, 0, 1);
  t(0, 2) = mult_cel(other, 0, 2);

  t(1, 0) = mult_cel(other, 1, 0);
  t(1, 1) = mult_cel(other, 1, 1);
  t(1, 2) = mult_cel(other, 1, 2);

  t(2, 0) = mult_cel(other, 2, 0);
  t(2, 1) = mult_cel(other, 2, 1);
  t(2, 2) = mult_cel(other, 2, 2);

  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix * scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> LMatrix3<NumType>::
operator * (NumType scalar) const {
  LMatrix3<NumType> t;

  t(0, 0) = (*this)(0, 0) * scalar;
  t(0, 1) = (*this)(0, 1) * scalar;
  t(0, 2) = (*this)(0, 2) * scalar;

  t(1, 0) = (*this)(1, 0) * scalar;
  t(1, 1) = (*this)(1, 1) * scalar;
  t(1, 2) = (*this)(1, 2) * scalar;

  t(2, 0) = (*this)(2, 0) * scalar;
  t(2, 1) = (*this)(2, 1) * scalar;
  t(2, 2) = (*this)(2, 2) * scalar;

  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix / scalar
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> LMatrix3<NumType>::
operator / (NumType scalar) const {
  LMatrix3<NumType> t;

  t(0, 0) = (*this)(0, 0) / scalar;
  t(0, 1) = (*this)(0, 1) / scalar;
  t(0, 2) = (*this)(0, 2) / scalar;

  t(1, 0) = (*this)(1, 0) / scalar;
  t(1, 1) = (*this)(1, 1) / scalar;
  t(1, 2) = (*this)(1, 2) / scalar;

  t(2, 0) = (*this)(2, 0) / scalar;
  t(2, 1) = (*this)(2, 1) / scalar;
  t(2, 2) = (*this)(2, 2) / scalar;

  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix += matrix
//       Access: Public
//  Description: Performs a memberwise addition between two matrices.
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> &LMatrix3<NumType>::
operator += (const LMatrix3<NumType> &other) {
  (*this)(0, 0) += other(0, 0);
  (*this)(0, 1) += other(0, 1);
  (*this)(0, 2) += other(0, 2);

  (*this)(1, 0) += other(1, 0);
  (*this)(1, 1) += other(1, 1);
  (*this)(1, 2) += other(1, 2);

  (*this)(2, 0) += other(2, 0);
  (*this)(2, 1) += other(2, 1);
  (*this)(2, 2) += other(2, 2);

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix -= matrix
//       Access: Public
//  Description: Performs a memberwise subtraction between two matrices.
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> &LMatrix3<NumType>::
operator -= (const LMatrix3<NumType> &other) {
  (*this)(0, 0) -= other(0, 0);
  (*this)(0, 1) -= other(0, 1);
  (*this)(0, 2) -= other(0, 2);

  (*this)(1, 0) -= other(1, 0);
  (*this)(1, 1) -= other(1, 1);
  (*this)(1, 2) -= other(1, 2);

  (*this)(2, 0) -= other(2, 0);
  (*this)(2, 1) -= other(2, 1);
  (*this)(2, 2) -= other(2, 2);

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix *= matrix
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix3<NumType> &LMatrix3<NumType>::
operator *= (const LMatrix3<NumType> &other) {
  (*this) = (*this) * other;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix *= scalar
//       Access: Public
//  Description: Performs a memberwise scale.
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> &LMatrix3<NumType>::
operator *= (NumType scalar) {
  (*this)(0, 0) *= scalar;
  (*this)(0, 1) *= scalar;
  (*this)(0, 2) *= scalar;

  (*this)(1, 0) *= scalar;
  (*this)(1, 1) *= scalar;
  (*this)(1, 2) *= scalar;

  (*this)(2, 0) *= scalar;
  (*this)(2, 1) *= scalar;
  (*this)(2, 2) *= scalar;

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix /= scalar
//       Access: Public
//  Description: Performs a memberwise scale.
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> &LMatrix3<NumType>::
operator /= (NumType scalar) {
  (*this)(0, 0) /= scalar;
  (*this)(0, 1) /= scalar;
  (*this)(0, 2) /= scalar;

  (*this)(1, 0) /= scalar;
  (*this)(1, 1) /= scalar;
  (*this)(1, 2) /= scalar;

  (*this)(2, 0) /= scalar;
  (*this)(2, 1) /= scalar;
  (*this)(2, 2) /= scalar;

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::transpose_from
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
void LMatrix3<NumType>::
transpose_from(const LMatrix3<NumType> &other) {
  (*this)(0, 0) = other(0, 0);
  (*this)(0, 1) = other(1, 0);
  (*this)(0, 2) = other(2, 0);

  (*this)(1, 0) = other(0, 1);
  (*this)(1, 1) = other(1, 1);
  (*this)(1, 2) = other(2, 1);

  (*this)(2, 0) = other(0, 2);
  (*this)(2, 1) = other(1, 2);
  (*this)(2, 2) = other(2, 2);
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::transpose_in_place
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LMatrix3<NumType>::
transpose_in_place() {
  LMatrix3<NumType> temp = (*this);
  transpose_from(temp);
}

// Matrix inversion code from Numerical Recipes in C.

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::det2
//       Access: Private, Static
//  Description: Returns the determinant of a 2x2 matrix.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LMatrix3<NumType>::
det2(NumType e00, NumType e01, NumType e10, NumType e11) const {
  return (e00 * e11 - e10 * e01);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::determinant
//       Access: Public
//  Description: Returns the determinant of the matrix.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE NumType LMatrix3<NumType>::
determinant() const {
  return 
    (*this)(0,0) * det2((*this)(1,1),(*this)(1,2),(*this)(2,1),(*this)(2,2))
    -(*this)(0,1) * det2((*this)(1,0),(*this)(1,2),(*this)(2,0),(*this)(2,2))
    +(*this)(0,2) * det2((*this)(1,0),(*this)(1,1),(*this)(2,0),(*this)(2,1));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::invert_from
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
bool LMatrix3<NumType>::
invert_from(const LMatrix3<NumType> &other) {
  NumType d = other.determinant();

  if (IS_NEARLY_ZERO(d)) {
    linmath_cat.warning()
      << "Tried to invert singular LMatrix3.\n";
    (*this) = ident_mat();
    return false;
  }

  d = 1.0 / d;
  (*this)(0,0) =  d * det2(other(1,1), other(1,2), other(2,1), other(2,2));
  (*this)(1,0) = -d * det2(other(1,0), other(1,2), other(2,0), other(2,2));
  (*this)(2,0) =  d * det2(other(1,0), other(1,1), other(2,0), other(2,1));

  (*this)(0,1) = -d * det2(other(0,1), other(0,2), other(2,1), other(2,2));
  (*this)(1,1) =  d * det2(other(0,0), other(0,2), other(2,0), other(2,2));
  (*this)(2,1) = -d * det2(other(0,0), other(0,1), other(2,0), other(2,1));

  (*this)(0,2) =  d * det2(other(0,1), other(0,2), other(1,1), other(1,2));
  (*this)(1,2) = -d * det2(other(0,0), other(0,2), other(1,0), other(1,2));
  (*this)(2,2) =  d * det2(other(0,0), other(0,1), other(1,0), other(1,1));

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::invert_in_place
//       Access: Public
//  Description: Inverts the current matrix.  Returns true if the
//               inverse is successful, false if the matrix was
//               singular.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LMatrix3<NumType>::
invert_in_place() {
  LMatrix3<NumType> temp = (*this);
  return invert_from(temp);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::ident_mat
//       Access: Public, Static
//  Description: Returns an identity matrix.
////////////////////////////////////////////////////////////////////
template<class NumType>
const LMatrix3<NumType> &LMatrix3<NumType>::
ident_mat() {
  static LMatrix3<NumType> mat(1.0, 0.0, 0.0,
			       0.0, 1.0, 0.0,
			       0.0, 0.0, 1.0);
  return mat;
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix::translate_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               translation.
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> LMatrix3<NumType>::
translate_mat(const LVecBase2<NumType> &trans) {
  return LMatrix3<NumType>(1.0, 0.0, 0.0,
			   0.0, 1.0, 0.0,
			   trans[0], trans[1], 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::translate_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               translation.
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> LMatrix3<NumType>::
translate_mat(NumType tx, NumType ty) {
  return LMatrix3<NumType>(1.0, 0.0, 0.0,
			   0.0, 1.0, 0.0,
			   tx, ty, 1.0);
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix::rotate_mat
//       Access: Public, Static
//  Description: Returns a matrix that rotates by the given angle in
//               degrees counterclockwise.
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> LMatrix3<NumType>::
rotate_mat(NumType angle) {
  double angle_rad=deg_2_rad(angle);
  double s,c;
  csincos(angle_rad,&s,&c);
  return LMatrix3<NumType>(  c,    s,  0.0,
			    -s,    c,  0.0,
			   0.0,  0.0,  1.0);
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale in each of the two axes.
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> LMatrix3<NumType>::
scale_mat(const LVecBase2<NumType> &scale) {
  return LMatrix3<NumType>(scale[0], 0.0, 0.0,
			   0.0, scale[1], 0.0,
			   0.0, 0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale in each of the two axes.
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> LMatrix3<NumType>::
scale_mat(NumType sx, NumType sy) {
  return LMatrix3<NumType>(sx, 0.0, 0.0,
			   0.0, sy, 0.0,
			   0.0, 0.0, 1.0);
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix::rotate_mat
//       Access: Public, Static
//  Description: Returns a matrix that rotates by the given angle in
//               degrees counterclockwise about the indicated vector.
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> LMatrix3<NumType>::
rotate_mat(NumType angle, LVecBase3<NumType> axis,
	   CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }
  LMatrix3<NumType> mat;

  if (!is_right_handed(cs)) {
    // In a left-handed coordinate system, counterclockwise is the
    // other direction.
    angle = -angle;
  }

  // Normalize the axis.
  NumType length = axis.dot(axis);
  nassertr(length != 0.0, ident_mat());
  NumType recip_length=1.0f/length;
  axis *= recip_length;

  double angle_rad=deg_2_rad(angle);
  double s,c;
  csincos(angle_rad,&s,&c);
  double t = 1.0 - c;

  mat(0, 0) = t * axis[0] * axis[0] + c;
  mat(0, 1) = t * axis[0] * axis[1] + s * axis[2];
  mat(0, 2) = t * axis[0] * axis[2] - s * axis[1];

  mat(1, 0) = t * axis[1] * axis[0] - s * axis[2];
  mat(1, 1) = t * axis[1] * axis[1] + c;
  mat(1, 2) = t * axis[1] * axis[2] + s * axis[0];

  mat(2, 0) = t * axis[2] * axis[0] + s * axis[1];
  mat(2, 1) = t * axis[2] * axis[1] - s * axis[0];
  mat(2, 2) = t * axis[2] * axis[2] + c;

  return mat;
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale in each of the three axes.
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> LMatrix3<NumType>::
scale_mat(const LVecBase3<NumType> &scale) {
  return LMatrix3<NumType>(scale[0], 0.0, 0.0,
			   0.0, scale[1], 0.0,
			   0.0, 0.0, scale[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale in each of the three axes.
////////////////////////////////////////////////////////////////////
template<class NumType>
LMatrix3<NumType> LMatrix3<NumType>::
scale_mat(NumType sx, NumType sy, NumType sz) {
  return LMatrix3<NumType>(sx, 0.0, 0.0,
			   0.0, sy, 0.0,
			   0.0, 0.0, sz);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::almost_equal
//       Access: Public
//  Description: Returns true if two matrices are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////
template<class NumType>
bool LMatrix3<NumType>::
almost_equal(const LMatrix3<NumType> &other, NumType threshold) const {
  return (IS_THRESHOLD_EQUAL((*this)(0, 0), other(0, 0), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(0, 1), other(0, 1), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(0, 2), other(0, 2), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(1, 0), other(1, 0), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(1, 1), other(1, 1), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(1, 2), other(1, 2), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(2, 0), other(2, 0), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(2, 1), other(2, 1), threshold) &&
	  IS_THRESHOLD_EQUAL((*this)(2, 2), other(2, 2), threshold));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::almost_equal
//       Access: Public
//  Description: Returns true if two matrices are memberwise equal
//               within a default tolerance based on the numeric type.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool LMatrix3<NumType>::
almost_equal(const LMatrix3<NumType> &other) const {
  return almost_equal(other, NEARLY_ZERO(NumType));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LMatrix3<NumType>::
output(ostream &out) const {
  out << "[ " 
      << MAYBE_ZERO((*this)(0, 0)) << " "
      << MAYBE_ZERO((*this)(0, 1)) << " " 
      << MAYBE_ZERO((*this)(0, 2))
      << " ] [ "
      << MAYBE_ZERO((*this)(1, 0)) << " "
      << MAYBE_ZERO((*this)(1, 1)) << " " 
      << MAYBE_ZERO((*this)(1, 2))
      << " ] [ "
      << MAYBE_ZERO((*this)(2, 0)) << " "
      << MAYBE_ZERO((*this)(2, 1)) << " " 
      << MAYBE_ZERO((*this)(2, 2))
      << " ]";
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void LMatrix3<NumType>::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) 
    << MAYBE_ZERO((*this)(0, 0)) << " "
    << MAYBE_ZERO((*this)(0, 1)) << " " 
    << MAYBE_ZERO((*this)(0, 2))
    << "\n";
  indent(out, indent_level)
    << MAYBE_ZERO((*this)(1, 0)) << " "
    << MAYBE_ZERO((*this)(1, 1)) << " " 
    << MAYBE_ZERO((*this)(1, 2))
    << "\n";
  indent(out, indent_level)
    << MAYBE_ZERO((*this)(2, 0)) << " "
    << MAYBE_ZERO((*this)(2, 1)) << " " 
    << MAYBE_ZERO((*this)(2, 2))
    << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::init_type
//       Access: Public, Static
//  Description: 
////////////////////////////////////////////////////////////////////
template<class NumType>
void LMatrix3<NumType>::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    do_init_type(NumType);
    string name =
      "LMatrix3<" + get_type_handle(NumType).get_name() + ">";
    register_type(_type_handle, name);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::write_datagram
//  Description: Writes the matrix to the datagram
////////////////////////////////////////////////////////////////////
template<class NumType>
void LMatrix3<NumType>::
write_datagram(Datagram &destination) const
{
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      destination.add_float32(get_cell(i,j));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::read_datagram
//  Description: Reads itself out of the datagram
////////////////////////////////////////////////////////////////////
template<class NumType>
void LMatrix3<NumType>::
read_datagram(DatagramIterator &scan) 
{
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      set_cell(i, j, scan.get_float32());
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::transpose
//  Description: Transposes the given matrix and returns it.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix3<NumType>
transpose(const LMatrix3<NumType> &a) {
  LMatrix3<NumType> result;
  result.transpose_from(a);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::invert
//  Description: Inverts the given matrix and returns it.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE LMatrix3<NumType>
invert(const LMatrix3<NumType> &a) {
  LMatrix3<NumType> result;
  bool nonsingular = result.invert_from(a);
  nassertr(nonsingular, LMatrix3<NumType>::ident_mat());
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: lcast_to
//  Description: Converts a matrix from one numeric representation to
//               another one.  This is usually invoked using the macro
//               LCAST.
////////////////////////////////////////////////////////////////////
template<class NumType, class NumType2>
INLINE LMatrix3<NumType2> 
lcast_to(NumType2 *, const LMatrix3<NumType> &source) {
  return LMatrix3<NumType2>
    (source(0, 0), source(0, 1), source(0, 2),
     source(1, 0), source(1, 1), source(1, 2),
     source(2, 0), source(2, 1), source(2, 2));
}
