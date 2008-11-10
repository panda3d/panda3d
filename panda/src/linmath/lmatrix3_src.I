// Filename: lmatrix3_src.I
// Created by:  drose (29Jan99)
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
//     Function: LMatrix3::Row::Constructor
//       Access: Private
//  Description: Defines a row-level index accessor to the matrix.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3)::Row::
Row(FLOATTYPE *row) : _row(row) {
}
    
////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Row::operator []
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix3)::Row::
operator [](int i) const {
  nassertr(i >= 0 && i < 3, 0.0);
  return _row[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Row::operator []
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE &FLOATNAME(LMatrix3)::Row::
operator [](int i) {
  nassertr(i >= 0 && i < 3, _row[0]);
  return _row[i];
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Row::__setitem__
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix3)::Row::
__setitem__(int i, FLOATTYPE v) {
  nassertv(i >= 0 && i < 3);
  _row[i] = v;
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Row::size
//       Access: Public, Static
//  Description: Returns 3: the number of columns of a LMatrix3.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LMatrix3)::Row::
size() {
  return 3;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::CRow::Constructor
//       Access: Private
//  Description: Defines a row-level constant accessor to the matrix.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3)::CRow::
CRow(const FLOATTYPE *row) : _row(row) {
}
    
////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::CRow::operator []
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix3)::CRow::
operator [](int i) const {
  nassertr(i >= 0 && i < 3, 0.0);
  return _row[i];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::CRow::size
//       Access: Public, Static
//  Description: Returns 3: the number of columns of a LMatrix3.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LMatrix3)::CRow::
size() {
  return 3;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::ident_mat
//       Access: Public, Static
//  Description: Returns an identity matrix.
//
//               This function definition must appear first, since
//               some inline functions below take advantage of it.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
ident_mat() {
  return _ident_mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3)::
FLOATNAME(LMatrix3)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3)::
FLOATNAME(LMatrix3)(const FLOATNAME(LMatrix3) &copy) {
  TAU_PROFILE("LMatrix3::LMatrix3(const LMatrix3 &)", " ", TAU_USER);
  
  memcpy(_m.data, copy._m.data, sizeof(_m.data));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator = (const FLOATNAME(LMatrix3) &copy) {
  TAU_PROFILE("void LMatrix3::operator = (const LMatrix3 &)", " ", TAU_USER);

  memcpy(_m.data, copy._m.data, sizeof(_m.data));
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Fill Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator = (FLOATTYPE fill_value) {
  fill(fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3)::
FLOATNAME(LMatrix3)(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02,
                    FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12,
                    FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22) {
  TAU_PROFILE("LMatrix3::LMatrix3(FLOATTYPE, ...)", " ", TAU_USER);
  _m.m._00 = e00;
  _m.m._01 = e01;
  _m.m._02 = e02;
  _m.m._10 = e10;
  _m.m._11 = e11;
  _m.m._12 = e12;
  _m.m._20 = e20;
  _m.m._21 = e21;
  _m.m._22 = e22;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::set
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02,
    FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12,
    FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22) {
  TAU_PROFILE("void LMatrix3::set(FLOATTYPE, ...)", " ", TAU_USER);
  _m.m._00 = e00;
  _m.m._01 = e01;
  _m.m._02 = e02;
  _m.m._10 = e10;
  _m.m._11 = e11;
  _m.m._12 = e12;
  _m.m._20 = e20;
  _m.m._21 = e21;
  _m.m._22 = e22;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::set_row
//       Access: Public
//  Description: Replaces the indicated row of the matrix from a
//               three-component vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_row(int row, const FLOATNAME(LVecBase3) &v) {
  (*this)(row, 0) = v._v.v._0;
  (*this)(row, 1) = v._v.v._1;
  (*this)(row, 2) = v._v.v._2;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::set_column
//       Access: Public
//  Description: Replaces the indicated column of the matrix from a
//               three-component vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_col(int col, const FLOATNAME(LVecBase3) &v) {
  (*this)(0, col) = v._v.v._0;
  (*this)(1, col) = v._v.v._1;
  (*this)(2, col) = v._v.v._2;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::set_row
//       Access: Public
//  Description: Replaces the indicated row of the matrix from a
//               two-component vector, ignoring the last column.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_row(int row, const FLOATNAME(LVecBase2) &v) {
  (*this)(row, 0) = v._v.v._0;
  (*this)(row, 1) = v._v.v._1;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::set_column
//       Access: Public
//  Description: Replaces the indicated column of the matrix from a
//               two-component vector, ignoring the last row.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_col(int col, const FLOATNAME(LVecBase2) &v) {
  (*this)(0, col) = v._v.v._0;
  (*this)(1, col) = v._v.v._1;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_row
//       Access: Public
//  Description: Returns the indicated row of the matrix as a
//               three-component vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix3)::
get_row(int row) const {
  return FLOATNAME(LVecBase3)((*this)(row, 0), (*this)(row, 1), (*this)(row, 2));
}

INLINE_LINMATH void FLOATNAME(LMatrix3)::
get_row(FLOATNAME(LVecBase3) &result_vec,int row) const {
  result_vec._v.v._0 = (*this)(row, 0);
  result_vec._v.v._1 = (*this)(row, 1);
  result_vec._v.v._2 = (*this)(row, 2);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_col
//       Access: Public
//  Description: Returns the indicated column of the matrix as a
//               three-component vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix3)::
get_col(int col) const {
  return FLOATNAME(LVecBase3)((*this)(0, col), (*this)(1, col), (*this)(2, col));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_row2
//       Access: Public
//  Description: Returns the indicated row of the matrix as a
//               two-component vector, ignoring the last column.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LMatrix3)::
get_row2(int row) const {
  return FLOATNAME(LVecBase2)((*this)(row, 0), (*this)(row, 1));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_col2
//       Access: Public
//  Description: Returns the indicated column of the matrix as a
//               two-component vector, ignoring the last row.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LMatrix3)::
get_col2(int col) const {
  return FLOATNAME(LVecBase2)((*this)(0, col), (*this)(1, col));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Indexing operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE &FLOATNAME(LMatrix3)::
operator () (int row, int col) {
  nassertr(row >= 0 && row < 3 && col >= 0 && col < 3, _m.data[0]);
  return _m.data[row * 3 + col];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Indexing operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix3)::
operator () (int row, int col) const {
  nassertr(row >= 0 && row < 3 && col >= 0 && col < 3, 0.0);
  return _m.data[row * 3 + col];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Indexing Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3)::CRow FLOATNAME(LMatrix3)::
operator [](int i) const {
  nassertr(i >= 0 && i < 3, CRow(&_m.data[0]));
  return CRow(&_m.data[i * 3]);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Indexing Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3)::Row FLOATNAME(LMatrix3)::
operator [](int i) {
  nassertr(i >= 0 && i < 3, Row(&_m.data[0]));
  return Row(&_m.data[i * 3]);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::size
//       Access: Public, Static
//  Description: Returns 3: the number of rows of a LMatrix3.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LMatrix3)::
size() {
  return 3;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::is_nan
//       Access: Public
//  Description: Returns true if any component of the matrix is
//               not-a-number, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
is_nan() const {
  TAU_PROFILE("bool LMatrix3::is_nan()", " ", TAU_USER);
  return
    cnan(_m.data[0]) || cnan(_m.data[1]) || cnan(_m.data[2]) ||
    cnan(_m.data[3]) || cnan(_m.data[4]) || cnan(_m.data[5]) ||
    cnan(_m.data[6]) || cnan(_m.data[7]) || cnan(_m.data[8]);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_cell
//       Access: Public
//  Description: Returns a particular element of the matrix.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix3)::
get_cell(int row, int col) const {
  nassertr(row >= 0 && row < 3 && col >= 0 && col < 3, 0.0);
  return _m.data[row * 3 + col];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::set_cell
//       Access: Public
//  Description: Changes a particular element of the matrix.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_cell(int row, int col, FLOATTYPE value) {
  nassertv(row >= 0 && row < 3 && col >= 0 && col < 3);
  _m.data[row * 3 + col] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_data
//       Access: Public
//  Description: Returns the address of the first of the nine data
//               elements in the matrix.  The remaining elements
//               occupy the next eight positions in row-major order.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH const FLOATTYPE *FLOATNAME(LMatrix3)::
get_data() const {
  return _m.data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_num_components
//       Access: Public
//  Description: Returns the number of elements in the matrix, nine.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LMatrix3)::
get_num_components() const {
  return 9;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3)::iterator FLOATNAME(LMatrix3)::
begin() {
  return _m.data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3)::iterator FLOATNAME(LMatrix3)::
end() {
  return begin() + get_num_components();
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3)::const_iterator FLOATNAME(LMatrix3)::
begin() const {
  return _m.data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3)::const_iterator FLOATNAME(LMatrix3)::
end() const {
  return begin() + get_num_components();
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Ordering Operator
//       Access: Public
//  Description: This performs a lexicographical comparison.  It's of
//               questionable mathematical meaning, but sometimes has
//               a practical purpose for sorting unique vectors,
//               especially in an STL container.  Also see
//               compare_to().
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
operator < (const FLOATNAME(LMatrix3) &other) const {
  return compare_to(other) < 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Equality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
operator == (const FLOATNAME(LMatrix3) &other) const {
  return compare_to(other) == 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Inequality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
operator != (const FLOATNAME(LMatrix3) &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::compare_to
//       Access: Public
//  Description: This flavor of compare_to uses a default threshold
//               value based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH int FLOATNAME(LMatrix3)::
compare_to(const FLOATNAME(LMatrix3) &other) const {
  return compare_to(other, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_hash
//       Access: Public
//  Description: Returns a suitable hash for phash_map.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH size_t FLOATNAME(LMatrix3)::
get_hash() const {
  return add_hash(0);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_hash
//       Access: Public
//  Description: Returns a suitable hash for phash_map.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH size_t FLOATNAME(LMatrix3)::
get_hash(FLOATTYPE threshold) const {
  return add_hash(0, threshold);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::add_hash
//       Access: Public
//  Description: Adds the vector into the running hash.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH size_t FLOATNAME(LMatrix3)::
add_hash(size_t hash) const {
  return add_hash(hash, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::add_hash
//       Access: Public
//  Description: Adds the vector into the running hash.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH size_t FLOATNAME(LMatrix3)::
add_hash(size_t hash, FLOATTYPE threshold) const {
  TAU_PROFILE("size_t LMatrix3::add_hash(size_t, FLOATTYPE)", " ", TAU_USER);
  float_hash fhasher(threshold);

  hash = fhasher.add_hash(hash, _m.m._00);
  hash = fhasher.add_hash(hash, _m.m._01);
  hash = fhasher.add_hash(hash, _m.m._02);

  hash = fhasher.add_hash(hash, _m.m._10);
  hash = fhasher.add_hash(hash, _m.m._11);
  hash = fhasher.add_hash(hash, _m.m._12);

  hash = fhasher.add_hash(hash, _m.m._20);
  hash = fhasher.add_hash(hash, _m.m._21);
  hash = fhasher.add_hash(hash, _m.m._22);

  return hash;
}

#define VECTOR3_MATRIX3_PRODUCT(v_res, v, mat)                                              \
v_res._v.v._0 = v._v.v._0*mat._m.m._00 + v._v.v._1*mat._m.m._10 + v._v.v._2*mat._m.m._20;   \
v_res._v.v._1 = v._v.v._0*mat._m.m._01 + v._v.v._1*mat._m.m._11 + v._v.v._2*mat._m.m._21;   \
v_res._v.v._2 = v._v.v._0*mat._m.m._02 + v._v.v._1*mat._m.m._12 + v._v.v._2*mat._m.m._22;

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::xform
//       Access: Public
//  Description: 3-component vector or point times matrix.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix3)::
xform(const FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix3::xform(const LVecBase3 &)", " ", TAU_USER);
  FLOATNAME(LVecBase3) v_res;
  VECTOR3_MATRIX3_PRODUCT(v_res, v,(*this));
  
  return v_res;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::xform_point
//       Access: Public
//  Description: The matrix transforms a 2-component point (including
//               translation component) and returns the result.  This
//               assumes the matrix is an affine transform.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LMatrix3)::
xform_point(const FLOATNAME(LVecBase2) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix3::xform_point(const LVecBase3 &)", " ", TAU_USER);
  FLOATNAME(LVecBase2) v_res;

  // v._v.v._2 == 1.0f for this case
  
  v_res._v.v._0 = v._v.v._0*_m.m._00 + v._v.v._1*_m.m._10 + _m.m._20;
  v_res._v.v._1 = v._v.v._0*_m.m._01 + v._v.v._1*_m.m._11 + _m.m._21;
  
  return v_res;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::xform_vec
//       Access: Public
//  Description: The matrix transforms a 2-component vector (without
//               translation component) and returns the result.  This
//               assumes the matrix is an affine transform.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LMatrix3)::
xform_vec(const FLOATNAME(LVecBase2) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix3::xform_vec(const LVecBase3 &)", " ", TAU_USER);

  FLOATNAME(LVecBase2) v_res;

  // v._v.v._2 == 0.0f for this case
  
  v_res._v.v._0 = v._v.v._0*_m.m._00 + v._v.v._1*_m.m._10;
  v_res._v.v._1 = v._v.v._0*_m.m._01 + v._v.v._1*_m.m._11;
  
  return v_res;
  
//  return FLOATNAME(LVecBase2)(v.dot(get_col2(0)),
//                          v.dot(get_col2(1)));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::xform_vec
//       Access: Public
//  Description: The matrix transforms a 3-component vector and
//               returns the result.  This assumes the matrix is an
//               orthonormal transform.
//
//               In practice, this is the same computation as xform().
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix3)::
xform_vec(const FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix3::xform_vec(const LVecBase3 &)", " ", TAU_USER);
  FLOATNAME(LVecBase3) v_res;
  
  // v._v.v._3 == 0.0f for this case
  
  v_res._v.v._0 = v._v.v._0*_m.m._00 + v._v.v._1*_m.m._10 + v._v.v._2*_m.m._20;
  v_res._v.v._1 = v._v.v._0*_m.m._01 + v._v.v._1*_m.m._11 + v._v.v._2*_m.m._21;
  v_res._v.v._2 = v._v.v._0*_m.m._02 + v._v.v._1*_m.m._12 + v._v.v._2*_m.m._22;
  
  return v_res;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::xform_vec_general
//       Access: Public
//  Description: The matrix transforms a 3-component vector (without
//               translation component) and returns the result, as a
//               fully general operation.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix3)::
xform_vec_general(const FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix3::xform_vec_general(const LVecBase3 &)", " ", TAU_USER);
  FLOATNAME(LMatrix3) i;
  i.invert_transpose_from(*this);

  return i.xform(v);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::mult_cel
//       Access: Private
//  Description: Returns one cell of the result of a matrix-matrix
//               multiplication operation.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix3)::
mult_cel(const FLOATNAME(LMatrix3) &other, int row, int col) const {
  return get_row(row).dot(other.get_col(col));
}


#define MATRIX3_PRODUCT(res, a, b)                   \
res._m.m._00 = a._m.m._00*b._m.m._00 + a._m.m._01*b._m.m._10 + a._m.m._02*b._m.m._20;   \
res._m.m._01 = a._m.m._00*b._m.m._01 + a._m.m._01*b._m.m._11 + a._m.m._02*b._m.m._21;   \
res._m.m._02 = a._m.m._00*b._m.m._02 + a._m.m._01*b._m.m._12 + a._m.m._02*b._m.m._22;   \
res._m.m._10 = a._m.m._10*b._m.m._00 + a._m.m._11*b._m.m._10 + a._m.m._12*b._m.m._20;   \
res._m.m._11 = a._m.m._10*b._m.m._01 + a._m.m._11*b._m.m._11 + a._m.m._12*b._m.m._21;   \
res._m.m._12 = a._m.m._10*b._m.m._02 + a._m.m._11*b._m.m._12 + a._m.m._12*b._m.m._22;   \
res._m.m._20 = a._m.m._20*b._m.m._00 + a._m.m._21*b._m.m._10 + a._m.m._22*b._m.m._20;   \
res._m.m._21 = a._m.m._20*b._m.m._01 + a._m.m._21*b._m.m._11 + a._m.m._22*b._m.m._21;   \
res._m.m._22 = a._m.m._20*b._m.m._02 + a._m.m._21*b._m.m._12 + a._m.m._22*b._m.m._22;


////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix * matrix
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
operator * (const FLOATNAME(LMatrix3) &other) const {
  TAU_PROFILE("LMatrix3 LMatrix3::operator *(const LMatrix3 &)", " ", TAU_USER);
  FLOATNAME(LMatrix3) t;
  t.multiply(*this, other);
  return t;
}

// this = other1 * other2
INLINE_LINMATH void FLOATNAME(LMatrix3)::
multiply(const FLOATNAME(LMatrix3) &other1, const FLOATNAME(LMatrix3) &other2) {
  TAU_PROFILE("LMatrix3 multiply(const LMatrix3 &, const LMatrix3 &)", " ", TAU_USER);
  // faster than operator * since it writes result in place, avoiding extra copying
  // this will fail if you try to mat.multiply(mat,other_mat)

  nassertv((&other1 != this) && (&other2 != this));

  MATRIX3_PRODUCT((*this), other1, other2);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix * scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
operator * (FLOATTYPE scalar) const {
  TAU_PROFILE("LMatrix3 operator *(const LMatrix3 &, FLOATTYPE)", " ", TAU_USER);
  FLOATNAME(LMatrix3) t;

  t._m.m._00 = _m.m._00 * scalar;
  t._m.m._01 = _m.m._01 * scalar;
  t._m.m._02 = _m.m._02 * scalar;

  t._m.m._10 = _m.m._10 * scalar;
  t._m.m._11 = _m.m._11 * scalar;
  t._m.m._12 = _m.m._12 * scalar;

  t._m.m._20 = _m.m._20 * scalar;
  t._m.m._21 = _m.m._21 * scalar;
  t._m.m._22 = _m.m._22 * scalar;

  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix / scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
operator / (FLOATTYPE scalar) const {
  FLOATTYPE recip_scalar = 1.0f/scalar;
  return (*this) * recip_scalar;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix += matrix
//       Access: Public
//  Description: Performs a memberwise addition between two matrices.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator += (const FLOATNAME(LMatrix3) &other) {
  TAU_PROFILE("LMatrix3 LMatrix3::operator +=(const LMatrix3 &)", " ", TAU_USER);
  _m.m._00 += other._m.m._00;
  _m.m._01 += other._m.m._01;
  _m.m._02 += other._m.m._02;

  _m.m._10 += other._m.m._10;
  _m.m._11 += other._m.m._11;
  _m.m._12 += other._m.m._12;

  _m.m._20 += other._m.m._20;
  _m.m._21 += other._m.m._21;
  _m.m._22 += other._m.m._22;

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix -= matrix
//       Access: Public
//  Description: Performs a memberwise subtraction between two matrices.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator -= (const FLOATNAME(LMatrix3) &other) {
  TAU_PROFILE("LMatrix3 LMatrix3::operator -=(const LMatrix3 &)", " ", TAU_USER);
  _m.m._00 -= other._m.m._00;
  _m.m._01 -= other._m.m._01;
  _m.m._02 -= other._m.m._02;

  _m.m._10 -= other._m.m._10;
  _m.m._11 -= other._m.m._11;
  _m.m._12 -= other._m.m._12;

  _m.m._20 -= other._m.m._20;
  _m.m._21 -= other._m.m._21;
  _m.m._22 -= other._m.m._22;

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix *= matrix
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator *= (const FLOATNAME(LMatrix3) &other) {
  TAU_PROFILE("LMatrix3 LMatrix3::operator *=(const LMatrix3 &)", " ", TAU_USER);
  FLOATNAME(LMatrix3) temp = *this;
  multiply(temp, other);

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix *= scalar
//       Access: Public
//  Description: Performs a memberwise scale.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator *= (FLOATTYPE scalar) {
  TAU_PROFILE("LMatrix3 LMatrix3::operator *=(FLOATTYPE)", " ", TAU_USER);
  _m.m._00 *= scalar;
  _m.m._01 *= scalar;
  _m.m._02 *= scalar;

  _m.m._10 *= scalar;
  _m.m._11 *= scalar;
  _m.m._12 *= scalar;

  _m.m._20 *= scalar;
  _m.m._21 *= scalar;
  _m.m._22 *= scalar;

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix /= scalar
//       Access: Public
//  Description: Performs a memberwise scale.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator /= (FLOATTYPE scalar) {
  TAU_PROFILE("LMatrix3 LMatrix3::operator /=(FLOATTYPE)", " ", TAU_USER);
  FLOATTYPE recip_scalar = 1.0f/scalar;
  _m.m._00 *= recip_scalar;
  _m.m._01 *= recip_scalar;
  _m.m._02 *= recip_scalar;

  _m.m._10 *= recip_scalar;
  _m.m._11 *= recip_scalar;
  _m.m._12 *= recip_scalar;

  _m.m._20 *= recip_scalar;
  _m.m._21 *= recip_scalar;
  _m.m._22 *= recip_scalar;

  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::transpose_from
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix3)::
transpose_from(const FLOATNAME(LMatrix3) &other) {
  TAU_PROFILE("LMatrix3 LMatrix3::transpose_from(const LMatrix3 &other)", " ", TAU_USER);
  _m.m._00 = other._m.m._00;
  _m.m._01 = other._m.m._10;
  _m.m._02 = other._m.m._20;

  _m.m._10 = other._m.m._01;
  _m.m._11 = other._m.m._11;
  _m.m._12 = other._m.m._21;

  _m.m._20 = other._m.m._02;
  _m.m._21 = other._m.m._12;
  _m.m._22 = other._m.m._22;
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::transpose_in_place
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix3)::
transpose_in_place() {
  TAU_PROFILE("void LMatrix3::transpose_in_place()", " ", TAU_USER);

  #define SWAP__(x,y) { FLOATTYPE temp = (x);  (x) = (y);  (y) = temp;}
  SWAP__(_m.m._01,_m.m._10);
  SWAP__(_m.m._02,_m.m._20);
  SWAP__(_m.m._12,_m.m._21);
  #undef SWAP__
}

// Matrix inversion code from Numerical Recipes in C.

// dont trust compilers to inline these
#define DET2(E00,E01,E10,E11) ((E00)*(E11) - (E10)*(E01))
#define MATRIX3_DETERMINANT(mat)                                  \
   ( (mat)._00 * DET2((mat)._11,(mat)._12,(mat)._21,(mat)._22)    \
    -(mat)._01 * DET2((mat)._10,(mat)._12,(mat)._20,(mat)._22)    \
    +(mat)._02 * DET2((mat)._10,(mat)._11,(mat)._20,(mat)._21))


////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::det2
//       Access: Private, Static
//  Description: Returns the determinant of a 2x2 matrix.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix3)::
det2(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e10, FLOATTYPE e11) const {
  TAU_PROFILE("FLOATTYPE LMatrix3::det2()", " ", TAU_USER);
  return DET2(e00, e01, e10, e11);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::determinant
//       Access: Public
//  Description: Returns the determinant of the matrix.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix3)::
determinant() const {
  TAU_PROFILE("FLOATTYPE LMatrix3::determinant()", " ", TAU_USER);

 return MATRIX3_DETERMINANT(_m.m);
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
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
invert_from(const FLOATNAME(LMatrix3) &other) {
  TAU_PROFILE("bool LMatrix3::invert_from(const LMatrix3 &)", " ", TAU_USER);

  FLOATTYPE other_det = MATRIX3_DETERMINANT(other._m.m);

  // We throw the value out only if it's smaller than our "small"
  // threshold squared.  This helps reduce overly-sensitive
  // rejections.
  if (IS_THRESHOLD_ZERO(other_det, (NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE)))) {
    //    if (IS_NEARLY_ZERO(other_det)) {
#ifdef NOTIFY_DEBUG
    linmath_cat.warning() << "Tried to invert singular LMatrix3.\n";
#endif
    (*this) = ident_mat();
    return false;
  }

  other_det = 1.0f / other_det;
  _m.m._00 =  other_det * DET2(other._m.m._11, other._m.m._12, other._m.m._21, other._m.m._22);
  _m.m._10 = -other_det * DET2(other._m.m._10, other._m.m._12, other._m.m._20, other._m.m._22);
  _m.m._20 =  other_det * DET2(other._m.m._10, other._m.m._11, other._m.m._20, other._m.m._21);

  _m.m._01 = -other_det * DET2(other._m.m._01, other._m.m._02, other._m.m._21, other._m.m._22);
  _m.m._11 =  other_det * DET2(other._m.m._00, other._m.m._02, other._m.m._20, other._m.m._22);
  _m.m._21 = -other_det * DET2(other._m.m._00, other._m.m._01, other._m.m._20, other._m.m._21);

  _m.m._02 =  other_det * DET2(other._m.m._01, other._m.m._02, other._m.m._11, other._m.m._12);
  _m.m._12 = -other_det * DET2(other._m.m._00, other._m.m._02, other._m.m._10, other._m.m._12);
  _m.m._22 =  other_det * DET2(other._m.m._00, other._m.m._01, other._m.m._10, other._m.m._11);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::invert_in_place
//       Access: Public
//  Description: Inverts the current matrix.  Returns true if the
//               inverse is successful, false if the matrix was
//               singular.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
invert_in_place() {
  TAU_PROFILE("bool LMatrix3::invert_in_place()", " ", TAU_USER);
  FLOATNAME(LMatrix3) temp = (*this);
  return invert_from(temp);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::invert_transpose_from
//       Access: Public
//  Description: Simultaneously computes the inverse of the indicated
//               matrix, and then the transpose of that inverse.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
invert_transpose_from(const FLOATNAME(LMatrix3) &other) {
  TAU_PROFILE("bool LMatrix3::invert_transpose_from(const LMatrix3 &)", " ", TAU_USER);

  FLOATTYPE other_det = MATRIX3_DETERMINANT(other._m.m);

  if (IS_THRESHOLD_ZERO(other_det, (NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE)))) {
#ifdef NOTIFY_DEBUG
    linmath_cat.warning() << "Tried to invert singular LMatrix3.\n";
#endif
    (*this) = ident_mat();
    return false;
  }

  other_det = 1.0f / other_det;
  _m.m._00 =  other_det * DET2(other._m.m._11, other._m.m._12, other._m.m._21, other._m.m._22);
  _m.m._01 = -other_det * DET2(other._m.m._10, other._m.m._12, other._m.m._20, other._m.m._22);
  _m.m._02 =  other_det * DET2(other._m.m._10, other._m.m._11, other._m.m._20, other._m.m._21);

  _m.m._10 = -other_det * DET2(other._m.m._01, other._m.m._02, other._m.m._21, other._m.m._22);
  _m.m._11 =  other_det * DET2(other._m.m._00, other._m.m._02, other._m.m._20, other._m.m._22);
  _m.m._12 = -other_det * DET2(other._m.m._00, other._m.m._01, other._m.m._20, other._m.m._21);

  _m.m._20 =  other_det * DET2(other._m.m._01, other._m.m._02, other._m.m._11, other._m.m._12);
  _m.m._21 = -other_det * DET2(other._m.m._00, other._m.m._02, other._m.m._10, other._m.m._12);
  _m.m._22 =  other_det * DET2(other._m.m._00, other._m.m._01, other._m.m._10, other._m.m._11);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::invert_transpose_from
//       Access: Public
//  Description: Simultaneously computes the inverse of the indicated
//               matrix, and then the transpose of that inverse.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
invert_transpose_from(const FLOATNAME(LMatrix4) &other) {
  TAU_PROFILE("bool LMatrix3::invert_transpose_from(const LMatrix4 &)", " ", TAU_USER);

  FLOATTYPE other_det = MATRIX3_DETERMINANT(other._m.m);

  if (IS_THRESHOLD_ZERO(other_det, (NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE)))) {
#ifdef NOTIFY_DEBUG
    linmath_cat.warning() << "Tried to invert singular LMatrix4.\n";
#endif
    (*this) = ident_mat();
    return false;
  }

  other_det = 1.0f / other_det;
  _m.m._00 =  other_det * DET2(other._m.m._11, other._m.m._12, other._m.m._21, other._m.m._22);
  _m.m._01 = -other_det * DET2(other._m.m._10, other._m.m._12, other._m.m._20, other._m.m._22);
  _m.m._02 =  other_det * DET2(other._m.m._10, other._m.m._11, other._m.m._20, other._m.m._21);

  _m.m._10 = -other_det * DET2(other._m.m._01, other._m.m._02, other._m.m._21, other._m.m._22);
  _m.m._11 =  other_det * DET2(other._m.m._00, other._m.m._02, other._m.m._20, other._m.m._22);
  _m.m._12 = -other_det * DET2(other._m.m._00, other._m.m._01, other._m.m._20, other._m.m._21);

  _m.m._20 =  other_det * DET2(other._m.m._01, other._m.m._02, other._m.m._11, other._m.m._12);
  _m.m._21 = -other_det * DET2(other._m.m._00, other._m.m._02, other._m.m._10, other._m.m._12);
  _m.m._22 =  other_det * DET2(other._m.m._00, other._m.m._01, other._m.m._10, other._m.m._11);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::set_translate_mat
//       Access: Public
//  Description: Fills mat with a matrix that applies the indicated
//               translation.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_translate_mat(const FLOATNAME(LVecBase2) &trans) {
  set(1.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      trans._v.v._0, trans._v.v._1, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::set_rotate_mat
//       Access: Public
//  Description: Fills mat with a matrix that rotates by the given
//               angle in degrees counterclockwise.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_rotate_mat(FLOATTYPE angle) {
  TAU_PROFILE("void LMatrix3::rotate_mat(LMatrix3, FLOATTYPE)", " ", TAU_USER);
  FLOATTYPE angle_rad = deg_2_rad(angle);
  FLOATTYPE s, c;
  csincos(angle_rad, &s, &c);
  set(   c,    s,  0.0f,
        -s,    c,  0.0f,
      0.0f,  0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::set_scale_mat
//       Access: Public
//  Description: Fills mat with a matrix that applies the indicated
//               scale in each of the two axes.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_scale_mat(const FLOATNAME(LVecBase2) &scale) {
  set(scale._v.v._0, 0.0f, 0.0f,
      0.0f, scale._v.v._1, 0.0f,
      0.0f, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::translate_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               translation.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
translate_mat(const FLOATNAME(LVecBase2) &trans) {
  return FLOATNAME(LMatrix3)(1.0f, 0.0f, 0.0f,
                             0.0f, 1.0f, 0.0f,
                             trans._v.v._0, trans._v.v._1, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::translate_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               translation.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
translate_mat(FLOATTYPE tx, FLOATTYPE ty) {
  return FLOATNAME(LMatrix3)(1.0f, 0.0f, 0.0f,
                             0.0f, 1.0f, 0.0f,
                             tx, ty, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::rotate_mat
//       Access: Public, Static
//  Description: Returns a matrix that rotates by the given angle in
//               degrees counterclockwise.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
rotate_mat(FLOATTYPE angle) {
  FLOATNAME(LMatrix3) mat;
  mat.set_rotate_mat(angle);
  return mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale in each of the two axes.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_mat(const FLOATNAME(LVecBase2) &scale) {
  return FLOATNAME(LMatrix3)(scale._v.v._0, 0.0f, 0.0f,
                             0.0f, scale._v.v._1, 0.0f,
                             0.0f, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale in each of the two axes.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_mat(FLOATTYPE sx, FLOATTYPE sy) {
  return FLOATNAME(LMatrix3)(sx, 0.0f, 0.0f,
                             0.0f, sy, 0.0f,
                             0.0f, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::rotate_mat
//       Access: Public, Static
//  Description: Returns a matrix that rotates by the given angle in
//               degrees counterclockwise about the indicated vector.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
rotate_mat(FLOATTYPE angle, FLOATNAME(LVecBase3) axis,
           CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat;
  mat.set_rotate_mat(angle, axis, cs);
  return mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::rotate_mat_normaxis
//       Access: Public, Static
//  Description: Returns a matrix that rotates by the given angle in
//               degrees counterclockwise about the indicated vector.
//               Assumes axis has been normalized.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
rotate_mat_normaxis(FLOATTYPE angle, const FLOATNAME(LVecBase3) &axis,
           CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat;
  mat.set_rotate_mat_normaxis(angle, axis, cs);
  return mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::set_scale_mat
//       Access: Public
//  Description: Fills mat with a matrix that applies the indicated
//               scale in each of the three axes.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_scale_mat(const FLOATNAME(LVecBase3) &scale) {
  set(scale._v.v._0, 0.0f, 0.0f,
      0.0f, scale._v.v._1, 0.0f,
      0.0f, 0.0f, scale._v.v._2);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale in each of the three axes.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_mat(const FLOATNAME(LVecBase3) &scale) {
  return FLOATNAME(LMatrix3)(scale._v.v._0, 0.0f, 0.0f,
                             0.0f, scale._v.v._1, 0.0f,
                             0.0f, 0.0f, scale._v.v._2);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale in each of the three axes.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz) {
  return FLOATNAME(LMatrix3)(sx, 0.0f, 0.0f,
                             0.0f, sy, 0.0f,
                             0.0f, 0.0f, sz);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::set_shear_mat
//       Access: Public
//  Description: Fills mat with a matrix that applies the indicated
//               shear in each of the three planes.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_shear_mat(const FLOATNAME(LVecBase3) &shear, CoordinateSystem cs) {
  set_scale_shear_mat(FLOATNAME(LVecBase3)(1.0f, 1.0f, 1.0f),
                      shear, cs);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::shear_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               shear in each of the three planes.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
shear_mat(const FLOATNAME(LVecBase3) &shear, CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat;
  mat.set_shear_mat(shear, cs);
  return mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::shear_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               shear in each of the three planes.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
shear_mat(FLOATTYPE shxy, FLOATTYPE shxz, FLOATTYPE shyz, CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat;
  mat.set_shear_mat(FLOATNAME(LVecBase3)(shxy, shxz, shyz), cs);
  return mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_shear_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale and shear.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_shear_mat(const FLOATNAME(LVecBase3) &scale,
                const FLOATNAME(LVecBase3) &shear, 
                CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat;
  mat.set_scale_shear_mat(scale, shear, cs);
  return mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_shear_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale and shear.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_shear_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz,
                FLOATTYPE shxy, FLOATTYPE shxz, FLOATTYPE shyz,
                CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat;
  mat.set_scale_shear_mat(FLOATNAME(LVecBase3)(sx, sy, sz),
                          FLOATNAME(LVecBase3)(shxy, shxz, shyz), cs);
  return mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::almost_equal
//       Access: Public
//  Description: Returns true if two matrices are memberwise equal
//               within a default tolerance based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
almost_equal(const FLOATNAME(LMatrix3) &other) const {
  return almost_equal(other, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::generate_hash
//       Access: Public
//  Description: Adds the vector to the indicated hash generator.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix3)::
generate_hash(ChecksumHashGenerator &hashgen) const {
  generate_hash(hashgen, NEARLY_ZERO(FLOATTYPE));
}


////////////////////////////////////////////////////////////////////
//     Function: transpose
//  Description: Transposes the given matrix and returns it.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3)
transpose(const FLOATNAME(LMatrix3) &a) {
  FLOATNAME(LMatrix3) result;
  result.transpose_from(a);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: invert
//  Description: Inverts the given matrix and returns it.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3)
invert(const FLOATNAME(LMatrix3) &a) {
  TAU_PROFILE("LMatrix3 invert(const LMatrix3 &)", " ", TAU_USER);
  FLOATNAME(LMatrix3) result;
  bool nonsingular = result.invert_from(a);
#ifndef NDEBUG
  if (!nonsingular) {
    nassert_raise("Attempt to compute inverse of singular matrix!");
    return FLOATNAME(LMatrix3)::ident_mat();
  }
#endif
  return result;
}
