// Filename: lmatrix4_src.I
// Created by:  drose (15Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
INLINE_LINMATH const FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
ident_mat() {
  return _ident_mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4)::
FLOATNAME(LMatrix4)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4)::
FLOATNAME(LMatrix4)(const FLOATNAME(LMatrix4) &copy) {
  memcpy(_m.data,copy._m.data,sizeof(_m.data));
//  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator = (const FLOATNAME(LMatrix4) &copy) {
  //xxx888
  memcpy(_m.data,copy._m.data,sizeof(_m.data));
//  set(copy(0, 0), copy(0, 1), copy(0, 2), copy(0, 3),
//      copy(1, 0), copy(1, 1), copy(1, 2), copy(1, 3),
//      copy(2, 0), copy(2, 1), copy(2, 2), copy(2, 3),
//      copy(3, 0), copy(3, 1), copy(3, 2), copy(3, 3));
  return *this;
  //xxx000
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Fill Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator = (FLOATTYPE fill_value) {
  fill(fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4)::
FLOATNAME(LMatrix4)(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02, FLOATTYPE e03,
                    FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12, FLOATTYPE e13,
                    FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22, FLOATTYPE e23,
                    FLOATTYPE e30, FLOATTYPE e31, FLOATTYPE e32, FLOATTYPE e33) {

  _m.m._00 = e00;
  _m.m._01 = e01;
  _m.m._02 = e02;
  _m.m._03 = e03;

  _m.m._10 = e10;
  _m.m._11 = e11;
  _m.m._12 = e12;
  _m.m._13 = e13;

  _m.m._20 = e20;
  _m.m._21 = e21;
  _m.m._22 = e22;
  _m.m._23 = e23;

  _m.m._30 = e30;
  _m.m._31 = e31;
  _m.m._32 = e32;
  _m.m._33 = e33;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Constructor, upper 3x3
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4)::
FLOATNAME(LMatrix4)(const FLOATNAME(LMatrix3) &upper3) {

  _m.m._00 = upper3._m.m._00;
  _m.m._01 = upper3._m.m._01;
  _m.m._02 = upper3._m.m._02;
  _m.m._03 = 0.0f;

  _m.m._10 = upper3._m.m._10;
  _m.m._11 = upper3._m.m._11;
  _m.m._12 = upper3._m.m._12;
  _m.m._13 = 0.0f;

  _m.m._20 = upper3._m.m._20;
  _m.m._21 = upper3._m.m._21;
  _m.m._22 = upper3._m.m._22;
  _m.m._23 = 0.0f;

  _m.m._30 = 0.0f;
  _m.m._31 = 0.0f;
  _m.m._32 = 0.0f;
  _m.m._33 = 1.0f;

//  set(upper3(0, 0), upper3(0, 1), upper3(0, 2), 0.0f,
//      upper3(1, 0), upper3(1, 1), upper3(1, 2), 0.0f,
//      upper3(2, 0), upper3(2, 1), upper3(2, 2), 0.0f,
//      0.0f, 0.0f, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Constructor, upper 3x3 plus translation
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4)::
FLOATNAME(LMatrix4)(const FLOATNAME(LMatrix3) &upper3,
         const FLOATNAME(LVecBase3) &trans) {

  _m.m._00 = upper3._m.m._00;
  _m.m._01 = upper3._m.m._01;
  _m.m._02 = upper3._m.m._02;
  _m.m._03 = 0.0f;

  _m.m._10 = upper3._m.m._10;
  _m.m._11 = upper3._m.m._11;
  _m.m._12 = upper3._m.m._12;
  _m.m._13 = 0.0f;

  _m.m._20 = upper3._m.m._20;
  _m.m._21 = upper3._m.m._21;
  _m.m._22 = upper3._m.m._22;
  _m.m._23 = 0.0f;

  _m.m._30 = trans._v.v._0;
  _m.m._31 = trans._v.v._1;
  _m.m._32 = trans._v.v._2;
  _m.m._33 = 1.0f;

//  set(upper3(0, 0), upper3(0, 1), upper3(0, 2), 0.0f,
//      upper3(1, 0), upper3(1, 1), upper3(1, 2), 0.0f,
//      upper3(2, 0), upper3(2, 1), upper3(2, 2), 0.0f,
//      trans[0], trans[1], trans[2], 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::fill
//       Access: Public
//  Description: Sets each element of the matrix to the indicated
//               fill_value.  This is of questionable value, but is
//               sometimes useful when initializing to zero.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix4)::
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
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02, FLOATTYPE e03,
    FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12, FLOATTYPE e13,
    FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22, FLOATTYPE e23,
    FLOATTYPE e30, FLOATTYPE e31, FLOATTYPE e32, FLOATTYPE e33) {

  _m.m._00 = e00;
  _m.m._01 = e01;
  _m.m._02 = e02;
  _m.m._03 = e03;

  _m.m._10 = e10;
  _m.m._11 = e11;
  _m.m._12 = e12;
  _m.m._13 = e13;

  _m.m._20 = e20;
  _m.m._21 = e21;
  _m.m._22 = e22;
  _m.m._23 = e23;

  _m.m._30 = e30;
  _m.m._31 = e31;
  _m.m._32 = e32;
  _m.m._33 = e33;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::set_upper_3
//       Access: Public
//  Description: Sets the upper 3x3 submatrix.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_upper_3(const FLOATNAME(LMatrix3) &upper3) {
  _m.m._00 = upper3(0, 0);
  _m.m._01 = upper3(0, 1);
  _m.m._02 = upper3(0, 2);

  _m.m._10 = upper3(1, 0);
  _m.m._11 = upper3(1, 1);
  _m.m._12 = upper3(1, 2);

  _m.m._20 = upper3(2, 0);
  _m.m._21 = upper3(2, 1);
  _m.m._22 = upper3(2, 2);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_upper_3
//       Access: Public
//  Description: Retrieves the upper 3x3 submatrix.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix4)::
get_upper_3() const {
  return FLOATNAME(LMatrix3)
    (_m.m._00, _m.m._01, _m.m._02,
     _m.m._10, _m.m._11, _m.m._12,
     _m.m._20, _m.m._21, _m.m._22);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::set_row
//       Access: Public
//  Description: Replaces the indicated row of the matrix.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_row(int row, const FLOATNAME(LVecBase4) &v) {
  (*this)(row, 0) = v._v.v._0;
  (*this)(row, 1) = v._v.v._1;
  (*this)(row, 2) = v._v.v._2;
  (*this)(row, 3) = v._v.v._3;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::set_col
//       Access: Public
//  Description: Replaces the indicated column of the matrix.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_col(int col, const FLOATNAME(LVecBase4) &v) {
  (*this)(0, col) = v._v.v._0;
  (*this)(1, col) = v._v.v._1;
  (*this)(2, col) = v._v.v._2;
  (*this)(3, col) = v._v.v._3;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::set_row
//       Access: Public
//  Description: Replaces the indicated row of the matrix with the
//               indicated 3-component vector, ignoring the last
//               column.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_row(int row, const FLOATNAME(LVecBase3) &v) {
  (*this)(row, 0) = v._v.v._0;
  (*this)(row, 1) = v._v.v._1;
  (*this)(row, 2) = v._v.v._2;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::set_col
//       Access: Public
//  Description: Replaces the indicated column of the matrix with the
//               indicated 3-component vector, ignoring the last
//               row.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_col(int col, const FLOATNAME(LVecBase3) &v) {
  (*this)(0, col) = v._v.v._0;
  (*this)(1, col) = v._v.v._1;
  (*this)(2, col) = v._v.v._2;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_row
//       Access: Public
//  Description: Retrieves the indicated row of the matrix as a
//               4-component vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LMatrix4)::
get_row(int row) const {
  return FLOATNAME(LVecBase4)((*this)(row, 0),
                              (*this)(row, 1),
                              (*this)(row, 2),
                              (*this)(row, 3));
}

INLINE_LINMATH void FLOATNAME(LMatrix4)::
get_row(FLOATNAME(LVecBase4) &result_vec,int row) const {
  result_vec._v.v._0 = (*this)(row, 0);
  result_vec._v.v._1 = (*this)(row, 1);
  result_vec._v.v._2 = (*this)(row, 2);
  result_vec._v.v._3 = (*this)(row, 3);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_col
//       Access: Public
//  Description: Retrieves the indicated column of the matrix as a
//               4-component vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LMatrix4)::
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
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix4)::
get_row3(int row) const {
  return FLOATNAME(LVecBase3)((*this)(row, 0),
                              (*this)(row, 1),
                              (*this)(row, 2));
}

INLINE_LINMATH void FLOATNAME(LMatrix4)::
get_row3(FLOATNAME(LVecBase3) &result_vec,int row) const {
  result_vec._v.v._0 = (*this)(row, 0);
  result_vec._v.v._1 = (*this)(row, 1);
  result_vec._v.v._2 = (*this)(row, 2);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_col3
//       Access: Public
//  Description: Retrieves the indicated column of the matrix as a
//               3-component vector, ignoring the last row.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix4)::
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
INLINE_LINMATH FLOATTYPE &FLOATNAME(LMatrix4)::
operator () (int row, int col) {
  nassertr(row >= 0 && row < 4 && col >= 0 && col < 4, _m.data[0]);
  return _m.data[row * 4 + col];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Indexing operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix4)::
operator () (int row, int col) const {
  nassertr(row >= 0 && row < 4 && col >= 0 && col < 4, 0.0f);
  return _m.data[row * 4 + col];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::is_nan
//       Access: Public
//  Description: Returns true if any component of the matrix is
//               not-a-number, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
is_nan() const {
  return
    cnan(_m.data[0]) || cnan(_m.data[1]) || cnan(_m.data[2]) || cnan(_m.data[3]) ||
    cnan(_m.data[4]) || cnan(_m.data[5]) || cnan(_m.data[6]) || cnan(_m.data[7]) ||
    cnan(_m.data[8]) || cnan(_m.data[9]) || cnan(_m.data[10]) || cnan(_m.data[11]) ||
    cnan(_m.data[12]) || cnan(_m.data[13]) || cnan(_m.data[14]) || cnan(_m.data[15]);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_cell
//       Access: Public
//  Description: Returns a particular element of the matrix.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix4)::
get_cell(int row, int col) const {
  nassertr(row >= 0 && row < 4 && col >= 0 && col < 4, 0.0f);
  return _m.data[row * 4 + col];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::set_cell
//       Access: Public
//  Description: Changes a particular element of the matrix.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_cell(int row, int col, FLOATTYPE value) {
  nassertv(row >= 0 && row < 4 && col >= 0 && col < 4);
  _m.data[row * 4 + col] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_data
//       Access: Public
//  Description: Returns the address of the first of the nine data
//               elements in the matrix.  The remaining elements
//               occupy the next eight positions in row-major order.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATTYPE *FLOATNAME(LMatrix4)::
get_data() const {
  return _m.data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_num_components
//       Access: Public
//  Description: Returns the number of elements in the matrix, 16.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LMatrix4)::
get_num_components() const {
  return 16;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4)::iterator FLOATNAME(LMatrix4)::
begin() {
  return _m.data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4)::iterator FLOATNAME(LMatrix4)::
end() {
  return begin() + 16;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4)::const_iterator FLOATNAME(LMatrix4)::
begin() const {
  return _m.data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4)::const_iterator FLOATNAME(LMatrix4)::
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
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
operator < (const FLOATNAME(LMatrix4) &other) const {
  return compare_to(other) < 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Equality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
operator == (const FLOATNAME(LMatrix4) &other) const {
  return compare_to(other) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Inequality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
operator != (const FLOATNAME(LMatrix4) &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::compare_to
//       Access: Public
//  Description: This flavor of compare_to uses a default threshold
//               value based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LMatrix4)::
compare_to(const FLOATNAME(LMatrix4) &other) const {
  return compare_to(other, NEARLY_ZERO(FLOATTYPE));
}

#define VECTOR4_MATRIX4_PRODUCT(v_res, v, mat)       \
v_res._v.v._0 = v._v.v._0*mat._m.m._00 + v._v.v._1*mat._m.m._10 + v._v.v._2*mat._m.m._20 + v._v.v._3*mat._m.m._30;   \
v_res._v.v._1 = v._v.v._0*mat._m.m._01 + v._v.v._1*mat._m.m._11 + v._v.v._2*mat._m.m._21 + v._v.v._3*mat._m.m._31;   \
v_res._v.v._2 = v._v.v._0*mat._m.m._02 + v._v.v._1*mat._m.m._12 + v._v.v._2*mat._m.m._22 + v._v.v._3*mat._m.m._32;   \
v_res._v.v._3 = v._v.v._0*mat._m.m._03 + v._v.v._1*mat._m.m._13 + v._v.v._2*mat._m.m._23 + v._v.v._3*mat._m.m._33;

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::xform
//       Access: Public
//  Description: 4-component vector or point times matrix.  This is a
//               fully general operation.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LMatrix4)::
xform(const FLOATNAME(LVecBase4) &v) const {
    FLOATNAME(LVecBase4) v_res;

        VECTOR4_MATRIX4_PRODUCT(v_res, v,(*this));
        return v_res;

/*  return FLOATNAME(LVecBase4)(v.dot(get_col(0)),
                              v.dot(get_col(1)),
                              v.dot(get_col(2)),
                              v.dot(get_col(3)));
*/
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::xform_point
//       Access: Public
//  Description: The matrix transforms a 3-component point (including
//               translation component) and returns the result.  This
//               assumes the matrix is an affine transform.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix4)::
xform_point(const FLOATNAME(LVecBase3) &v) const {

        FLOATNAME(LVecBase3) v_res;

        // v._v.v._3 == 1.0f for this case

        v_res._v.v._0 = v._v.v._0*_m.m._00 + v._v.v._1*_m.m._10 + v._v.v._2*_m.m._20 + _m.m._30;
        v_res._v.v._1 = v._v.v._0*_m.m._01 + v._v.v._1*_m.m._11 + v._v.v._2*_m.m._21 + _m.m._31;
        v_res._v.v._2 = v._v.v._0*_m.m._02 + v._v.v._1*_m.m._12 + v._v.v._2*_m.m._22 + _m.m._32;

        return v_res;

/*
  return FLOATNAME(LVecBase3)(v.dot(get_col3(0)) + _m.m._30,
                              v.dot(get_col3(1)) + _m.m._31,
                              v.dot(get_col3(2)) + _m.m._32);
*/
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::xform_vec
//       Access: Public
//  Description: The matrix transforms a 3-component vector (without
//               translation component) and returns the result.  This
//               assumes the matrix is an affine transform.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix4)::
xform_vec(const FLOATNAME(LVecBase3) &v) const {
        FLOATNAME(LVecBase3) v_res;

        // v._v.v._3 == 0.0f for this case

        v_res._v.v._0 = v._v.v._0*_m.m._00 + v._v.v._1*_m.m._10 + v._v.v._2*_m.m._20;
        v_res._v.v._1 = v._v.v._0*_m.m._01 + v._v.v._1*_m.m._11 + v._v.v._2*_m.m._21;
        v_res._v.v._2 = v._v.v._0*_m.m._02 + v._v.v._1*_m.m._12 + v._v.v._2*_m.m._22;

        return v_res;
/*
  return FLOATNAME(LVecBase3)(v.dot(get_col3(0)),
                              v.dot(get_col3(1)),
                              v.dot(get_col3(2)));
*/
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::mult_cel
//       Access: Private
//  Description: Returns one cell of the result of a matrix-matrix
//               multiplication operation.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix4)::
mult_cel(const FLOATNAME(LMatrix4) &other, int row, int col) const {
  return get_row(row).dot(other.get_col(col));
}

#define MATRIX4_PRODUCT(res, a, b)                                          \
res._m.m._00 = a._m.m._00*b._m.m._00 + a._m.m._01*b._m.m._10 + a._m.m._02*b._m.m._20 + a._m.m._03*b._m.m._30;   \
res._m.m._01 = a._m.m._00*b._m.m._01 + a._m.m._01*b._m.m._11 + a._m.m._02*b._m.m._21 + a._m.m._03*b._m.m._31;   \
res._m.m._02 = a._m.m._00*b._m.m._02 + a._m.m._01*b._m.m._12 + a._m.m._02*b._m.m._22 + a._m.m._03*b._m.m._32;   \
res._m.m._03 = a._m.m._00*b._m.m._03 + a._m.m._01*b._m.m._13 + a._m.m._02*b._m.m._23 + a._m.m._03*b._m.m._33;   \
                                                                   \
res._m.m._10 = a._m.m._10*b._m.m._00 + a._m.m._11*b._m.m._10 + a._m.m._12*b._m.m._20 + a._m.m._13*b._m.m._30;   \
res._m.m._11 = a._m.m._10*b._m.m._01 + a._m.m._11*b._m.m._11 + a._m.m._12*b._m.m._21 + a._m.m._13*b._m.m._31;   \
res._m.m._12 = a._m.m._10*b._m.m._02 + a._m.m._11*b._m.m._12 + a._m.m._12*b._m.m._22 + a._m.m._13*b._m.m._32;   \
res._m.m._13 = a._m.m._10*b._m.m._03 + a._m.m._11*b._m.m._13 + a._m.m._12*b._m.m._23 + a._m.m._13*b._m.m._33;   \
                                                                   \
res._m.m._20 = a._m.m._20*b._m.m._00 + a._m.m._21*b._m.m._10 + a._m.m._22*b._m.m._20 + a._m.m._23*b._m.m._30;   \
res._m.m._21 = a._m.m._20*b._m.m._01 + a._m.m._21*b._m.m._11 + a._m.m._22*b._m.m._21 + a._m.m._23*b._m.m._31;   \
res._m.m._22 = a._m.m._20*b._m.m._02 + a._m.m._21*b._m.m._12 + a._m.m._22*b._m.m._22 + a._m.m._23*b._m.m._32;   \
res._m.m._23 = a._m.m._20*b._m.m._03 + a._m.m._21*b._m.m._13 + a._m.m._22*b._m.m._23 + a._m.m._23*b._m.m._33;   \
                                                                   \
res._m.m._30 = a._m.m._30*b._m.m._00 + a._m.m._31*b._m.m._10 + a._m.m._32*b._m.m._20 + a._m.m._33*b._m.m._30;   \
res._m.m._31 = a._m.m._30*b._m.m._01 + a._m.m._31*b._m.m._11 + a._m.m._32*b._m.m._21 + a._m.m._33*b._m.m._31;   \
res._m.m._32 = a._m.m._30*b._m.m._02 + a._m.m._31*b._m.m._12 + a._m.m._32*b._m.m._22 + a._m.m._33*b._m.m._32;   \
res._m.m._33 = a._m.m._30*b._m.m._03 + a._m.m._31*b._m.m._13 + a._m.m._32*b._m.m._23 + a._m.m._33*b._m.m._33;

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix * matrix
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
operator * (const FLOATNAME(LMatrix4) &other) const {
        FLOATNAME(LMatrix4) t;

        MATRIX4_PRODUCT(t,(*this),other);
/*
typedef union {
        struct {
           FLOATTYPE  _m.m._11, _m.m._12, _m.m._13, _m.m._14;
           FLOATTYPE  _m.m._21, _m.m._22, _m.m._23, _m.m._24;
           FLOATTYPE  _m.m._31, _m.m._32, _m.m._33, _m.m._34;
           FLOATTYPE  _m.m._41, _m.m._42, _m.m._43, _m.m._44;
        };

                FLOATTYPE m[4][4];
    } MYMATRIX4;

        FLOATNAME(LMatrix4) t;

        MYMATRIX4 *result_ptr=(MYMATRIX4 *)t.get_m.data();
        MYMATRIX4 *mat1_ptr=(MYMATRIX4 *)this->get_m.data();
        MYMATRIX4 *mat2_ptr=(MYMATRIX4 *)other.get_m.data();

        MATRIX4_PRODUCT(result_ptr,mat1_ptr,mat2_ptr);
*/

/*

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
*/
  return t;
}

// this = other1 * other2
INLINE_LINMATH void FLOATNAME(LMatrix4)::
multiply(const FLOATNAME(LMatrix4) &other1, const FLOATNAME(LMatrix4) &other2) {
// faster than operator * since it writes result in place, avoiding extra copying
// this will fail if you try to mat.multiply(mat,other_mat)

  #ifdef _DEBUG
     assert((&other1 != this) && (&other2 != this));
  #endif

  MATRIX4_PRODUCT((*this),other1,other2);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix * scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
operator * (FLOATTYPE scalar) const {
  FLOATNAME(LMatrix4) t;

  t._m.m._00 = _m.m._00 * scalar;
  t._m.m._01 = _m.m._01 * scalar;
  t._m.m._02 = _m.m._02 * scalar;
  t._m.m._03 = _m.m._03 * scalar;

  t._m.m._10 = _m.m._10 * scalar;
  t._m.m._11 = _m.m._11 * scalar;
  t._m.m._12 = _m.m._12 * scalar;
  t._m.m._13 = _m.m._13 * scalar;

  t._m.m._20 = _m.m._20 * scalar;
  t._m.m._21 = _m.m._21 * scalar;
  t._m.m._22 = _m.m._22 * scalar;
  t._m.m._23 = _m.m._23 * scalar;

  t._m.m._30 = _m.m._30 * scalar;
  t._m.m._31 = _m.m._31 * scalar;
  t._m.m._32 = _m.m._32 * scalar;
  t._m.m._33 = _m.m._33 * scalar;

  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix / scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
operator / (FLOATTYPE scalar) const {
  FLOATNAME(LMatrix4) t;
  FLOATTYPE recip_scalar = 1.0f/scalar;

  t._m.m._00 = _m.m._00 * recip_scalar;
  t._m.m._01 = _m.m._01 * recip_scalar;
  t._m.m._02 = _m.m._02 * recip_scalar;
  t._m.m._03 = _m.m._03 * recip_scalar;

  t._m.m._10 = _m.m._10 * recip_scalar;
  t._m.m._11 = _m.m._11 * recip_scalar;
  t._m.m._12 = _m.m._12 * recip_scalar;
  t._m.m._13 = _m.m._13 * recip_scalar;

  t._m.m._20 = _m.m._20 * recip_scalar;
  t._m.m._21 = _m.m._21 * recip_scalar;
  t._m.m._22 = _m.m._22 * recip_scalar;
  t._m.m._23 = _m.m._23 * recip_scalar;

  t._m.m._30 = _m.m._30 * recip_scalar;
  t._m.m._31 = _m.m._31 * recip_scalar;
  t._m.m._32 = _m.m._32 * recip_scalar;
  t._m.m._33 = _m.m._33 * recip_scalar;

  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix += matrix
//       Access: Public
//  Description: Performs a memberwise addition between two matrices.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator += (const FLOATNAME(LMatrix4) &other) {
  _m.m._00 += other._m.m._00;
  _m.m._01 += other._m.m._01;
  _m.m._02 += other._m.m._02;
  _m.m._03 += other._m.m._03;

  _m.m._10 += other._m.m._10;
  _m.m._11 += other._m.m._11;
  _m.m._12 += other._m.m._12;
  _m.m._13 += other._m.m._13;

  _m.m._20 += other._m.m._20;
  _m.m._21 += other._m.m._21;
  _m.m._22 += other._m.m._22;
  _m.m._23 += other._m.m._23;

  _m.m._30 += other._m.m._30;
  _m.m._31 += other._m.m._31;
  _m.m._32 += other._m.m._32;
  _m.m._33 += other._m.m._33;

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix -= matrix
//       Access: Public
//  Description: Performs a memberwise addition between two matrices.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator -= (const FLOATNAME(LMatrix4) &other) {
  _m.m._00 -= other._m.m._00;
  _m.m._01 -= other._m.m._01;
  _m.m._02 -= other._m.m._02;
  _m.m._03 -= other._m.m._03;

  _m.m._10 -= other._m.m._10;
  _m.m._11 -= other._m.m._11;
  _m.m._12 -= other._m.m._12;
  _m.m._13 -= other._m.m._13;

  _m.m._20 -= other._m.m._20;
  _m.m._21 -= other._m.m._21;
  _m.m._22 -= other._m.m._22;
  _m.m._23 -= other._m.m._23;

  _m.m._30 -= other._m.m._30;
  _m.m._31 -= other._m.m._31;
  _m.m._32 -= other._m.m._32;
  _m.m._33 -= other._m.m._33;

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix *= matrix
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator *= (const FLOATNAME(LMatrix4) &other) {
  (*this) = (*this) * other;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix *= scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator *= (FLOATTYPE scalar) {
  _m.m._00 *= scalar;
  _m.m._01 *= scalar;
  _m.m._02 *= scalar;
  _m.m._03 *= scalar;

  _m.m._10 *= scalar;
  _m.m._11 *= scalar;
  _m.m._12 *= scalar;
  _m.m._13 *= scalar;

  _m.m._20 *= scalar;
  _m.m._21 *= scalar;
  _m.m._22 *= scalar;
  _m.m._23 *= scalar;

  _m.m._30 *= scalar;
  _m.m._31 *= scalar;
  _m.m._32 *= scalar;
  _m.m._33 *= scalar;

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix /= scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator /= (FLOATTYPE scalar) {
  FLOATTYPE recip_scalar = 1.0f/scalar;
  _m.m._00 *= recip_scalar;
  _m.m._01 *= recip_scalar;
  _m.m._02 *= recip_scalar;
  _m.m._03 *= recip_scalar;

  _m.m._10 *= recip_scalar;
  _m.m._11 *= recip_scalar;
  _m.m._12 *= recip_scalar;
  _m.m._13 *= recip_scalar;

  _m.m._20 *= recip_scalar;
  _m.m._21 *= recip_scalar;
  _m.m._22 *= recip_scalar;
  _m.m._23 *= recip_scalar;

  _m.m._30 *= recip_scalar;
  _m.m._31 *= recip_scalar;
  _m.m._32 *= recip_scalar;
  _m.m._33 *= recip_scalar;

  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::transpose_from
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix4)::
transpose_from(const FLOATNAME(LMatrix4) &other) {
  _m.m._00 = other._m.m._00;
  _m.m._01 = other._m.m._10;
  _m.m._02 = other._m.m._20;
  _m.m._03 = other._m.m._30;

  _m.m._10 = other._m.m._01;
  _m.m._11 = other._m.m._11;
  _m.m._12 = other._m.m._21;
  _m.m._13 = other._m.m._31;

  _m.m._20 = other._m.m._02;
  _m.m._21 = other._m.m._12;
  _m.m._22 = other._m.m._22;
  _m.m._23 = other._m.m._32;

  _m.m._30 = other._m.m._03;
  _m.m._31 = other._m.m._13;
  _m.m._32 = other._m.m._23;
  _m.m._33 = other._m.m._33;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::transpose_in_place
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix4)::
transpose_in_place() {
//  FLOATNAME(LMatrix4) temp = (*this);
//  transpose_from(temp);

  #define SWAP__(x,y) { FLOATTYPE temp = (x);  (x) = (y);  (y) = temp;}
  SWAP__(_m.m._01,_m.m._10);
  SWAP__(_m.m._02,_m.m._20);
  SWAP__(_m.m._03,_m.m._30);
  SWAP__(_m.m._12,_m.m._21);
  SWAP__(_m.m._13,_m.m._31);
  SWAP__(_m.m._23,_m.m._32);
  #undef SWAP__
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

// bugbug: we could optimize this for rotation/scale/translation matrices
//         (transpose upper 3x3 and take negative of translation component)
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
invert_from(const FLOATNAME(LMatrix4) &other) {
  if (IS_NEARLY_EQUAL(other._m.m._30, 0.0f) &&
      IS_NEARLY_EQUAL(other._m.m._31, 0.0f) &&
      IS_NEARLY_EQUAL(other._m.m._32, 0.0f) &&
      IS_NEARLY_EQUAL(other._m.m._33, 1.0f)) {
    return invert_affine_from(other);
  }

  (*this) = other;

  int index[4];

  if (!decompose_mat(index)) {
#ifdef NDEBUG
    linmath_cat.warning() << "Tried to invert singular LMatrix4.\n";
#endif
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

// bugbug: we could optimize this for rotation/scale/translation matrices
//         (transpose upper 3x3 and take negative of translation component)
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
invert_affine_from(const FLOATNAME(LMatrix4) &other) {
  FLOATNAME(LMatrix3) rot;

  // probably could use transpose here
  if (!rot.invert_from(other.get_upper_3())) {
    return false;
  }

  set_upper_3(rot);
//  set_col(3, FLOATNAME(LVecBase4)(0.0f, 0.0f, 0.0f, 1.0f));

  _m.m._03 = 0.0f;
  _m.m._13 = 0.0f;
  _m.m._23 = 0.0f;
  _m.m._33 = 1.0f;

  // compute -C*inv(A)
/*
  for (int i = 0; i < 3; i++) {
    (*this)(3, i) = 0.0f;
    for (int j = 0; j < 3; j++) {
      (*this)(3, i) -= other(3, j) * (*this)(j, i);
    }
  }
*/

    _m.m._30 = -(other._m.m._30 * _m.m._00 +
               other._m.m._31 * _m.m._10 +
               other._m.m._32 * _m.m._20);

    _m.m._31 = -(other._m.m._30 * _m.m._01 +
                   other._m.m._31 * _m.m._11 +
               other._m.m._32 * _m.m._21);

    _m.m._32 = -(other._m.m._30 * _m.m._02 +
                   other._m.m._31 * _m.m._12 +
               other._m.m._32 * _m.m._22);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::invert_in_place
//       Access: Public
//  Description: Inverts the current matrix.  Returns true if the
//               inverse is successful, false if the matrix was
//               singular.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
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
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
translate_mat(const FLOATNAME(LVecBase3) &trans) {
  return FLOATNAME(LMatrix4)(1.0f, 0.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, 0.0f,
                           trans._v.v._0, trans._v.v._1, trans._v.v._2, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::translate_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               translation.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
translate_mat(FLOATTYPE tx, FLOATTYPE ty, FLOATTYPE tz) {
  return FLOATNAME(LMatrix4)(1.0f, 0.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, 0.0f,
                           tx, ty, tz, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::rotate_mat
//       Access: Public, Static
//  Description: Returns a matrix that rotates by the given angle in
//               degrees counterclockwise about the indicated vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
rotate_mat(FLOATTYPE angle, FLOATNAME(LVecBase3) axis,
           CoordinateSystem cs) {

  if (cs == CS_default) {
    cs = default_coordinate_system;
  }
  FLOATNAME(LMatrix4) mat;

  if(IS_LEFT_HANDED_COORDSYSTEM(cs)) {
    // In a left-handed coordinate system, counterclockwise is the
    // other direction.
    angle = -angle;
  }

  FLOATTYPE axis_0 = axis._v.v._0;
  FLOATTYPE axis_1 = axis._v.v._1;
  FLOATTYPE axis_2 = axis._v.v._2;

  // Normalize the axis.

/*
  // hack check for prenormalization, only works for simple unit vecs,
  // which is what we usually pass in anyway. screws up if you happen to
  // pass in something like (.5,.5,0).  need to add flag parameter so caller
  // can request normalization if needed

  if((cabs(axis_0)+cabs(axis_1)+cabs(axis_2)) != 1.0f) {
*/

          FLOATTYPE length_sq = axis_0 * axis_0 + axis_1 * axis_1 + axis_2 * axis_2;
          #ifdef _DEBUG
              nassertr(length_sq != 0.0f, ident_mat());
          #endif
          FLOATTYPE recip_length = 1.0f/csqrt(length_sq);

          axis_0 *= recip_length;
          axis_1 *= recip_length;
          axis_2 *= recip_length;
//  }

  FLOATTYPE angle_rad=deg_2_rad(angle);
  FLOATTYPE s,c;
  csincos(angle_rad,&s,&c);
  FLOATTYPE t = 1.0f - c;

  FLOATTYPE t0,t1,t2,s0,s1,s2;

  t0 = t * axis_0;
  t1 = t * axis_1;
  t2 = t * axis_2;
  s0 = s * axis_0;
  s1 = s * axis_1;
  s2 = s * axis_2;

  mat._m.m._00 = t0 * axis_0 + c;
  mat._m.m._01 = t0 * axis_1 + s2;
  mat._m.m._02 = t0 * axis_2 - s1;

  mat._m.m._10 = t1 * axis_0 - s2;
  mat._m.m._11 = t1 * axis_1 + c;
  mat._m.m._12 = t1 * axis_2 + s0;

  mat._m.m._20 = t2 * axis_0 + s1;
  mat._m.m._21 = t2 * axis_1 - s0;
  mat._m.m._22 = t2 * axis_2 + c;

  mat._m.m._03 = 0.0f;
  mat._m.m._13 = 0.0f;
  mat._m.m._23 = 0.0f;

  mat._m.m._30 = 0.0f;
  mat._m.m._31 = 0.0f;
  mat._m.m._32 = 0.0f;
  mat._m.m._33 = 1.0f;

  return mat;
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix::rotate_mat_normaxis
//       Access: Public, Static
//  Description: Returns a matrix that rotates by the given angle in
//               degrees counterclockwise about the indicated vector.
//               Assumes axis has been prenormalized.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
rotate_mat_normaxis(FLOATTYPE angle, const FLOATNAME(LVecBase3) &axis,
           CoordinateSystem cs) {

  FLOATNAME(LMatrix4) mat;
  rotate_mat_normaxis(angle,axis,mat,cs);
  return mat;
}

INLINE_LINMATH void FLOATNAME(LMatrix4)::
rotate_mat_normaxis(FLOATTYPE angle, const FLOATNAME(LVecBase3) &axis,
                                        FLOATNAME(LMatrix4) &result_mat, CoordinateSystem cs) {

  if (cs == CS_default) {
    cs = default_coordinate_system;
  }

  if(IS_LEFT_HANDED_COORDSYSTEM(cs)) {
    // In a left-handed coordinate system, counterclockwise is the
    // other direction.
    angle = -angle;
  }

  FLOATTYPE axis_0 = axis._v.v._0;
  FLOATTYPE axis_1 = axis._v.v._1;
  FLOATTYPE axis_2 = axis._v.v._2;

  FLOATTYPE angle_rad=deg_2_rad(angle);
  FLOATTYPE s,c;
  csincos(angle_rad,&s,&c);
  FLOATTYPE t = 1.0f - c;

  FLOATTYPE t0,t1,t2,s0,s1,s2;

  t0 = t * axis_0;
  t1 = t * axis_1;
  t2 = t * axis_2;
  s0 = s * axis_0;
  s1 = s * axis_1;
  s2 = s * axis_2;

  result_mat._m.m._00 = t0 * axis_0 + c;
  result_mat._m.m._01 = t0 * axis_1 + s2;
  result_mat._m.m._02 = t0 * axis_2 - s1;

  result_mat._m.m._10 = t1 * axis_0 - s2;
  result_mat._m.m._11 = t1 * axis_1 + c;
  result_mat._m.m._12 = t1 * axis_2 + s0;

  result_mat._m.m._20 = t2 * axis_0 + s1;
  result_mat._m.m._21 = t2 * axis_1 - s0;
  result_mat._m.m._22 = t2 * axis_2 + c;

  result_mat._m.m._03 = 0.0f;
  result_mat._m.m._13 = 0.0f;
  result_mat._m.m._23 = 0.0f;

  result_mat._m.m._30 = 0.0f;
  result_mat._m.m._31 = 0.0f;
  result_mat._m.m._32 = 0.0f;
  result_mat._m.m._33 = 1.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale in each of the three axes.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
scale_mat(const FLOATNAME(LVecBase3) &scale) {
  return FLOATNAME(LMatrix4)(scale._v.v._0, 0.0f, 0.0f, 0.0f,
                             0.0f, scale._v.v._1, 0.0f, 0.0f,
                             0.0f, 0.0f, scale._v.v._2, 0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f);
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale in each of the three axes.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
scale_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz) {
  return FLOATNAME(LMatrix4)(sx, 0.0f, 0.0f, 0.0f,
                             0.0f, sy, 0.0f, 0.0f,
                             0.0f, 0.0f, sz, 0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               uniform scale.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
scale_mat(FLOATTYPE scale) {
  return FLOATNAME(LMatrix4)(scale, 0.0f, 0.0f, 0.0f,
                             0.0f, scale, 0.0f, 0.0f,
                             0.0f, 0.0f, scale, 0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::shear_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               shear in each of the three planes.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
shear_mat(const FLOATNAME(LVecBase3) &shear, CoordinateSystem cs) {
  return scale_shear_mat(FLOATNAME(LVecBase3)(1.0f, 1.0f, 1.0f),
                         shear, cs);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::shear_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               shear in each of the three planes.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
shear_mat(FLOATTYPE shxy, FLOATTYPE shxz, FLOATTYPE shyz, 
          CoordinateSystem cs) {
  return scale_shear_mat(1.0f, 1.0f, 1.0f,
                         shxy, shxz, shyz, cs);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_shear_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale and shear.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
scale_shear_mat(const FLOATNAME(LVecBase3) &scale,
                const FLOATNAME(LVecBase3) &shear,
                CoordinateSystem cs) {
  return FLOATNAME(LMatrix4)(FLOATNAME(LMatrix3)::scale_shear_mat(scale, shear, cs));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_shear_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale and shear.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
scale_shear_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz,
                FLOATTYPE shxy, FLOATTYPE shxz, FLOATTYPE shyz,
                CoordinateSystem cs) {
  return scale_shear_mat(FLOATNAME(LVecBase3)(sx, sy, sz),
                         FLOATNAME(LVecBase3)(shxy, shxz, shyz),
                         cs);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::y_to_z_up_mat
//       Access: Public, Static
//  Description: Returns a matrix that transforms from the Y-up
//               coordinate system to the Z-up coordinate system.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
y_to_z_up_mat() {
  return _y_to_z_up_mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::z_to_y_up_mat
//       Access: Public, Static
//  Description: Returns a matrix that transforms from the Y-up
//               coordinate system to the Z-up coordinate system.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
z_to_y_up_mat() {
  return _z_to_y_up_mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::almost_equal
//       Access: Public
//  Description: Returns true if two matrices are memberwise equal
//               within a default tolerance based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
almost_equal(const FLOATNAME(LMatrix4) &other) const {
  return almost_equal(other, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::generate_hash
//       Access: Public
//  Description: Adds the vector to the indicated hash generator.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix4)::
generate_hash(ChecksumHashGenerator &hashgen) const {
  generate_hash(hashgen, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: transpose
//  Description: Transposes the given matrix and returns it.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4)
transpose(const FLOATNAME(LMatrix4) &a) {
  FLOATNAME(LMatrix4) result;
  result.transpose_from(a);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: invert
//  Description: Inverts the given matrix and returns it.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4)
invert(const FLOATNAME(LMatrix4) &a) {
  FLOATNAME(LMatrix4) result;
  bool nonsingular = result.invert_from(a);
  nassertr(nonsingular, FLOATNAME(LMatrix4)::ident_mat());
  return result;
}

