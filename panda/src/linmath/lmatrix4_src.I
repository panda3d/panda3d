// Filename: lmatrix4_src.I
// Created by:  drose (15Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Row::Constructor
//       Access: Private
//  Description: Defines a row-level index accessor to the matrix.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4)::Row::
Row(FLOATTYPE *row) : _row(row) {
}
    
////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Row::operator []
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix4)::Row::
operator [](int i) const {
  nassertr(i >= 0 && i < 4, 0.0);
  return _row[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Row::operator []
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE &FLOATNAME(LMatrix4)::Row::
operator [](int i) {
  nassertr(i >= 0 && i < 4, _row[0]);
  return _row[i];
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Row::__setitem__
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix4)::Row::
__setitem__(int i, FLOATTYPE v) {
  nassertv(i >= 0 && i < 4);
  _row[i] = v;
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Row::size
//       Access: Public, Static
//  Description: Returns 4: the number of columns of a LMatrix4.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LMatrix4)::Row::
size() {
  return 4;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::CRow::Constructor
//       Access: Private
//  Description: Defines a row-level constant accessor to the matrix.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4)::CRow::
CRow(const FLOATTYPE *row) : _row(row) {
}
    
////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::CRow::operator []
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix4)::CRow::
operator [](int i) const {
  nassertr(i >= 0 && i < 4, 0.0);
  return _row[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::CRow::size
//       Access: Public, Static
//  Description: Returns 4: the number of columns of a LMatrix4.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LMatrix4)::CRow::
size() {
  return 4;
}

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
//     Function: LMatrix4::ones_mat
//       Access: Public, Static
//  Description: Returns an matrix filled with ones.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
ones_mat() {
  return _ones_mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::zeros_mat
//       Access: Public, Static
//  Description: Returns an matrix filled with zeros.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
zeros_mat() {
  return _zeros_mat;
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
  TAU_PROFILE("LMatrix4::LMatrix4(const LMatrix4 &)", " ", TAU_USER);
  
  memcpy(_m.data, copy._m.data, sizeof(_m.data));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator = (const FLOATNAME(LMatrix4) &copy) {
  TAU_PROFILE("void LMatrix4::operator = (const LMatrix4 &)", " ", TAU_USER);

  memcpy(_m.data, copy._m.data, sizeof(_m.data));
  return *this;
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
  TAU_PROFILE("LMatrix4::LMatrix4(FLOATTYPE, ...)", " ", TAU_USER);

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
  TAU_PROFILE("void LMatrix4::LMatrix4(const LMatrix3 &)", " ", TAU_USER);

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
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Constructor, upper 3x3 plus translation
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4)::
FLOATNAME(LMatrix4)(const FLOATNAME(LMatrix3) &upper3,
         const FLOATNAME(LVecBase3) &trans) {
  TAU_PROFILE("void LMatrix4::LMatrix4(upper3, const LVecBase3 &)", " ", TAU_USER);

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
  TAU_PROFILE("void LMatrix4::fill(FLOATTYPE)", " ", TAU_USER);
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
  TAU_PROFILE("void LMatrix4::set(FLOATTYPE, ...)", " ", TAU_USER);

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
  TAU_PROFILE("void LMatrix4::set_upper_3(const LMatrix3 &)", " ", TAU_USER);
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
  TAU_PROFILE("LMatrix3 LMatrix4::get_upper_3()", " ", TAU_USER);
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
//     Function: LMatrix4::Indexing Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4)::CRow FLOATNAME(LMatrix4)::
operator [](int i) const {
  nassertr(i >= 0 && i < 4, CRow(&_m.data[0]));
  return CRow(&_m.data[i * 4]);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Indexing Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4)::Row FLOATNAME(LMatrix4)::
operator [](int i) {
  nassertr(i >= 0 && i < 4, Row(&_m.data[0]));
  return Row(&_m.data[i * 4]);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::size
//       Access: Public, Static
//  Description: Returns 4: the number of rows of a LMatrix4.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LMatrix4)::
size() {
  return 4;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::is_nan
//       Access: Public
//  Description: Returns true if any component of the matrix is
//               not-a-number, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
is_nan() const {
  TAU_PROFILE("bool LMatrix4::is_nan()", " ", TAU_USER);
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

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_hash
//       Access: Public
//  Description: Returns a suitable hash for phash_map.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH size_t FLOATNAME(LMatrix4)::
get_hash() const {
  return add_hash(0);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::get_hash
//       Access: Public
//  Description: Returns a suitable hash for phash_map.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH size_t FLOATNAME(LMatrix4)::
get_hash(FLOATTYPE threshold) const {
  return add_hash(0, threshold);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::add_hash
//       Access: Public
//  Description: Adds the vector into the running hash.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH size_t FLOATNAME(LMatrix4)::
add_hash(size_t hash) const {
  return add_hash(hash, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::add_hash
//       Access: Public
//  Description: Adds the vector into the running hash.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH size_t FLOATNAME(LMatrix4)::
add_hash(size_t hash, FLOATTYPE threshold) const {
  TAU_PROFILE("size_t LMatrix4::add_hash(size_t, FLOATTYPE)", " ", TAU_USER);
  float_hash fhasher(threshold);

  hash = fhasher.add_hash(hash, _m.m._00);
  hash = fhasher.add_hash(hash, _m.m._01);
  hash = fhasher.add_hash(hash, _m.m._02);
  hash = fhasher.add_hash(hash, _m.m._03);

  hash = fhasher.add_hash(hash, _m.m._10);
  hash = fhasher.add_hash(hash, _m.m._11);
  hash = fhasher.add_hash(hash, _m.m._12);
  hash = fhasher.add_hash(hash, _m.m._13);

  hash = fhasher.add_hash(hash, _m.m._20);
  hash = fhasher.add_hash(hash, _m.m._21);
  hash = fhasher.add_hash(hash, _m.m._22);
  hash = fhasher.add_hash(hash, _m.m._23);

  hash = fhasher.add_hash(hash, _m.m._30);
  hash = fhasher.add_hash(hash, _m.m._31);
  hash = fhasher.add_hash(hash, _m.m._32);
  hash = fhasher.add_hash(hash, _m.m._33);

  return hash;
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
  TAU_PROFILE("LVecBase3 LMatrix4::xform(const LVecBase3 &)", " ", TAU_USER);
  FLOATNAME(LVecBase4) v_res;
  
  VECTOR4_MATRIX4_PRODUCT(v_res, v,(*this));
  return v_res;
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
  TAU_PROFILE("LVecBase3 LMatrix4::xform_point(const LVecBase3 &)", " ", TAU_USER);
  FLOATNAME(LVecBase3) v_res;

  // v._v.v._3 == 1.0f for this case
  
  v_res._v.v._0 = v._v.v._0*_m.m._00 + v._v.v._1*_m.m._10 + v._v.v._2*_m.m._20 + _m.m._30;
  v_res._v.v._1 = v._v.v._0*_m.m._01 + v._v.v._1*_m.m._11 + v._v.v._2*_m.m._21 + _m.m._31;
  v_res._v.v._2 = v._v.v._0*_m.m._02 + v._v.v._1*_m.m._12 + v._v.v._2*_m.m._22 + _m.m._32;
  
  return v_res;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::xform_vec
//       Access: Public
//  Description: The matrix transforms a 3-component vector (without
//               translation component) and returns the result.  This
//               assumes the matrix is an orthonormal transform.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix4)::
xform_vec(const FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix4::xform_vec(const LVecBase3 &)", " ", TAU_USER);
  FLOATNAME(LVecBase3) v_res;
  
  // v._v.v._3 == 0.0f for this case
  
  v_res._v.v._0 = v._v.v._0*_m.m._00 + v._v.v._1*_m.m._10 + v._v.v._2*_m.m._20;
  v_res._v.v._1 = v._v.v._0*_m.m._01 + v._v.v._1*_m.m._11 + v._v.v._2*_m.m._21;
  v_res._v.v._2 = v._v.v._0*_m.m._02 + v._v.v._1*_m.m._12 + v._v.v._2*_m.m._22;
  
  return v_res;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::xform_vec_general
//       Access: Public
//  Description: The matrix transforms a 3-component vector (without
//               translation component) and returns the result, as a
//               fully general operation.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix4)::
xform_vec_general(const FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix4::xform_vec_general(const LVecBase3 &)", " ", TAU_USER);
  FLOATNAME(LMatrix3) i;
  i.invert_transpose_from(*this);

  return i.xform(v);
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
  TAU_PROFILE("LMatrix4 LMatrix4::operator *(const LMatrix4 &)", " ", TAU_USER);
  FLOATNAME(LMatrix4) t;
  t.multiply(*this, other);
  return t;
}

// this = other1 * other2
INLINE_LINMATH void FLOATNAME(LMatrix4)::
multiply(const FLOATNAME(LMatrix4) &other1, const FLOATNAME(LMatrix4) &other2) {
  TAU_PROFILE("LMatrix4 multiply(const LMatrix4 &, const LMatrix4 &)", " ", TAU_USER);
  // faster than operator * since it writes result in place, avoiding extra copying
  // this will fail if you try to mat.multiply(mat,other_mat)

  nassertv((&other1 != this) && (&other2 != this));
  MATRIX4_PRODUCT((*this),other1,other2);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix * scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
operator * (FLOATTYPE scalar) const {
  TAU_PROFILE("LMatrix4 operator *(const LMatrix4 &, FLOATTYPE)", " ", TAU_USER);
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
  FLOATTYPE recip_scalar = 1.0f/scalar;
  return (*this) * recip_scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix += matrix
//       Access: Public
//  Description: Performs a memberwise addition between two matrices.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator += (const FLOATNAME(LMatrix4) &other) {
  TAU_PROFILE("LMatrix4 LMatrix4::operator +=(const LMatrix4 &)", " ", TAU_USER);
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
  TAU_PROFILE("LMatrix4 LMatrix4::operator -=(const LMatrix4 &)", " ", TAU_USER);
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
  TAU_PROFILE("LMatrix4 LMatrix4::operator *=(const LMatrix4 &)", " ", TAU_USER);
  FLOATNAME(LMatrix4) temp = *this;
  multiply(temp, other);

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::matrix *= scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator *= (FLOATTYPE scalar) {
  TAU_PROFILE("LMatrix4 LMatrix4::operator *=(FLOATTYPE)", " ", TAU_USER);
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
  TAU_PROFILE("LMatrix4 LMatrix4::operator /=(FLOATTYPE)", " ", TAU_USER);
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
  TAU_PROFILE("LMatrix4 LMatrix4::transpose_from(const LMatrix4 &other)", " ", TAU_USER);
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
  TAU_PROFILE("void LMatrix4::transpose_in_place()", " ", TAU_USER);

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
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
invert_from(const FLOATNAME(LMatrix4) &other) {
  TAU_PROFILE("bool LMatrix4::invert_from(const LMatrix4 &)", " ", TAU_USER);
  if (IS_NEARLY_EQUAL(other._m.m._03, 0.0f) &&
      IS_NEARLY_EQUAL(other._m.m._13, 0.0f) &&
      IS_NEARLY_EQUAL(other._m.m._23, 0.0f) &&
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
  TAU_PROFILE("bool LMatrix4::invert_affine_from(const LMatrix4 &)", " ", TAU_USER);
  FLOATNAME(LMatrix3) rot;

  // probably could use transpose here
  if (!rot.invert_from(other.get_upper_3())) {
    return false;
  }

  set_upper_3(rot);

  _m.m._03 = 0.0f;
  _m.m._13 = 0.0f;
  _m.m._23 = 0.0f;
  _m.m._33 = 1.0f;

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
  TAU_PROFILE("bool LMatrix4::invert_in_place()", " ", TAU_USER);
  FLOATNAME(LMatrix4) temp = (*this);
  return invert_from(temp);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::set_translate_mat
//       Access: Public
//  Description: Fills mat with a matrix that applies the indicated
//               translation.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_translate_mat(const FLOATNAME(LVecBase3) &trans) {
  set(1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      trans._v.v._0, trans._v.v._1, trans._v.v._2, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::set_scale_mat
//       Access: Public
//  Description: Fills mat with a matrix that applies the indicated
//               scale in each of the three axes.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_scale_mat(const FLOATNAME(LVecBase3) &scale) {
  set(scale._v.v._0, 0.0f, 0.0f, 0.0f,
      0.0f, scale._v.v._1, 0.0f, 0.0f,
      0.0f, 0.0f, scale._v.v._2, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::set_shear_mat
//       Access: Public
//  Description: Fills mat with a matrix that applies the indicated
//               shear in each of the three planes.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_shear_mat(const FLOATNAME(LVecBase3) &shear, CoordinateSystem cs) {
  set_scale_shear_mat(FLOATNAME(LVecBase3)(1.0f, 1.0f, 1.0f),
                      shear, cs);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::set_scale_shear_mat
//       Access: Public, Static
//  Description: Fills mat with a matrix that applies the indicated
//               scale and shear.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_scale_shear_mat(const FLOATNAME(LVecBase3) &scale,
                    const FLOATNAME(LVecBase3) &shear,
                    CoordinateSystem cs) {
  FLOATNAME(LMatrix3) m3;
  m3.set_scale_shear_mat(scale, shear, cs);
  set_upper_3(m3);

  _m.m._03 = 0.0f;
  _m.m._13 = 0.0f;
  _m.m._23 = 0.0f;
  _m.m._33 = 1.0f;

  _m.m._30 = 0.0f;
  _m.m._31 = 0.0f;
  _m.m._32 = 0.0f;
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
rotate_mat(FLOATTYPE angle, const FLOATNAME(LVecBase3) &axis,
           CoordinateSystem cs) {
  FLOATNAME(LMatrix4) mat;
  mat.set_rotate_mat(angle, axis, cs);
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
  TAU_PROFILE("LMatrix4 LMatrix4::rotate_mat_normaxis(FLOATTYPE, const LVecBase3 &, cs)", " ", TAU_USER);

  FLOATNAME(LMatrix4) mat;
  mat.set_rotate_mat_normaxis(angle, axis, cs);
  return mat;
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
  FLOATNAME(LMatrix4) mat;
  mat.set_shear_mat(shear, cs);
  return mat;
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
  FLOATNAME(LMatrix4) mat;
  mat.set_shear_mat(FLOATNAME(LVecBase3)(shxy, shxz, shyz), cs);
  return mat;
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
  FLOATNAME(LMatrix4) mat;
  mat.set_scale_shear_mat(scale, shear, cs);
  return mat;
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
  FLOATNAME(LMatrix4) mat;
  mat.set_scale_shear_mat(FLOATNAME(LVecBase3)(sx, sy, sz),
                          FLOATNAME(LVecBase3)(shxy, shxz, shyz), cs);
  return mat;
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
  TAU_PROFILE("LMatrix4 invert(const LMatrix4 &)", " ", TAU_USER);
  FLOATNAME(LMatrix4) result;
  bool nonsingular = result.invert_from(a);
#ifndef NDEBUG
  if (!nonsingular) {
    nassert_raise("Attempt to compute inverse of singular matrix!");
    return FLOATNAME(LMatrix4)::ident_mat();
  }
#endif
  return result;
}

