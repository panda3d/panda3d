// Filename: lmatrix3_src.I
// Created by:  drose (29Jan99)
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
        memcpy(_m.data,copy._m.data,sizeof(_m.data));
//  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator = (const FLOATNAME(LMatrix3) &copy) {
  memcpy(_m.data,copy._m.data,sizeof(_m.data));
//  set(copy(0, 0), copy(0, 1), copy(0, 2),
//      copy(1, 0), copy(1, 1), copy(1, 2),
//      copy(2, 0), copy(2, 1), copy(2, 2));
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
  _m.m._00 = e00;
  _m.m._01 = e01;
  _m.m._02 = e02;
  _m.m._10 = e10;
  _m.m._11 = e11;
  _m.m._12 = e12;
  _m.m._20 = e20;
  _m.m._21 = e21;
  _m.m._22 = e22;

//  set(e00, e01, e02,
//      e10, e11, e12,
//      e20, e21, e22);
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
  _m.m._00 = e00;
  _m.m._01 = e01;
  _m.m._02 = e02;
  _m.m._10 = e10;
  _m.m._11 = e11;
  _m.m._12 = e12;
  _m.m._20 = e20;
  _m.m._21 = e21;
  _m.m._22 = e22;
  /*
  _m.m._00 = e00;
  _m.m._01 = e01;
  _m.m._02 = e02;
  _m.m._10 = e10;
  _m.m._11 = e11;
  _m.m._12 = e12;
  _m.m._20 = e20;
  _m.m._21 = e21;
  _m.m._22 = e22;
  */
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
//     Function: LMatrix3::is_nan
//       Access: Public
//  Description: Returns true if any component of the matrix is
//               not-a-number, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
is_nan() const {
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

// bugbug:  need 2 check if this is correct, or reversed from previous implementation
//          before checkin

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::xform
//       Access: Public
//  Description: 3-component vector or point times matrix.  This is a
//               fully general operation.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix3)::
xform(const FLOATNAME(LVecBase3) &v) const {
    FLOATNAME(LVecBase3) v_res;



        VECTOR3_MATRIX3_PRODUCT(v_res, v,(*this));



        return v_res;

//  return FLOATNAME(LVecBase3)(v.dot(get_col(0)),
//                          v.dot(get_col(1)),
//                          v.dot(get_col(2)));
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

        FLOATNAME(LVecBase2) v_res;

        // v._v.v._2 == 1.0f for this case

        v_res._v.v._0 = v._v.v._0*_m.m._00 + v._v.v._1*_m.m._10 + _m.m._20;
        v_res._v.v._1 = v._v.v._0*_m.m._01 + v._v.v._1*_m.m._11 + _m.m._21;

        return v_res;

//  return FLOATNAME(LVecBase2)(v.dot(get_col2(0)) + _m.m._20,
//                          v.dot(get_col2(1)) + _m.m._21);
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

        FLOATNAME(LVecBase2) v_res;

        // v._v.v._2 == 0.0f for this case

        v_res._v.v._0 = v._v.v._0*_m.m._00 + v._v.v._1*_m.m._10;
        v_res._v.v._1 = v._v.v._0*_m.m._01 + v._v.v._1*_m.m._11;

        return v_res;

//  return FLOATNAME(LVecBase2)(v.dot(get_col2(0)),
//                          v.dot(get_col2(1)));
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
        FLOATNAME(LMatrix3) t;

        MATRIX3_PRODUCT(t,(*this),other);
/*
typedef union {
        struct {
            FLOATTYPE     _m.m._11, _m.m._12, _m.m._13;
            FLOATTYPE     _m.m._21, _m.m._22, _m.m._23;
            FLOATTYPE     _m.m._31, _m.m._32, _m.m._33;
        };

                FLOATTYPE m[3][3];
    } MYMATRIX3;

        FLOATNAME(LMatrix3) t;

        MYMATRIX3 *result_ptr=(MYMATRIX3 *)t.get_m.data();
        MYMATRIX3 *mat1_ptr=(MYMATRIX3 *)this->get_m.data();
        MYMATRIX3 *mat2_ptr=(MYMATRIX3 *)other.get_m.data();

        MATRIX3_PRODUCT(result_ptr,mat1_ptr,mat2_ptr);
*/
/*
  t(0, 0) = mult_cel(other, 0, 0);
  t(0, 1) = mult_cel(other, 0, 1);
  t(0, 2) = mult_cel(other, 0, 2);

  t(1, 0) = mult_cel(other, 1, 0);
  t(1, 1) = mult_cel(other, 1, 1);
  t(1, 2) = mult_cel(other, 1, 2);

  t(2, 0) = mult_cel(other, 2, 0);
  t(2, 1) = mult_cel(other, 2, 1);
  t(2, 2) = mult_cel(other, 2, 2);
*/
  return t;
}

// this = other1 * other2
INLINE_LINMATH void FLOATNAME(LMatrix3)::
multiply(const FLOATNAME(LMatrix3) &other1, const FLOATNAME(LMatrix3) &other2) {
// faster than operator * since it writes result in place, avoiding extra copying
// this will fail if you try to mat.multiply(mat,other_mat)

  #ifdef _DEBUG
     assert((&other1 != this) && (&other2 != this));
  #endif

  MATRIX3_PRODUCT((*this),other1,other2);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix * scalar
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
operator * (FLOATTYPE scalar) const {
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
  (*this) = (*this) * other;

  // bugbug can we do any optimization here?
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix *= scalar
//       Access: Public
//  Description: Performs a memberwise scale.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator *= (FLOATTYPE scalar) {
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
//  FLOATNAME(LMatrix3) temp = (*this);
//  transpose_from(temp);

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
//  return (e00 * e11 - e10 * e01);
        return DET2(e00,e01,e10,e11);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::determinant
//       Access: Public
//  Description: Returns the determinant of the matrix.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix3)::
determinant() const {
/*
  return
    (*this)(0,0) * det2((*this)(1,1),(*this)(1,2),(*this)(2,1),(*this)(2,2))
    -(*this)(0,1) * det2((*this)(1,0),(*this)(1,2),(*this)(2,0),(*this)(2,2))
    +(*this)(0,2) * det2((*this)(1,0),(*this)(1,1),(*this)(2,0),(*this)(2,1));
*/

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
  FLOATNAME(LMatrix3) temp = (*this);
  return invert_from(temp);
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
  FLOATTYPE angle_rad=deg_2_rad(angle);
  FLOATTYPE s,c;
  csincos(angle_rad,&s,&c);
  return FLOATNAME(LMatrix3)(  c,    s,  0.0f,
                              -s,    c,  0.0f,
                             0.0f,  0.0f,  1.0f);
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
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }
  FLOATNAME(LMatrix3) mat;

  if(IS_LEFT_HANDED_COORDSYSTEM(cs)) {
    // In a left-handed coordinate system, counterclockwise is the
    // other direction.
    angle = -angle;
  }

  FLOATTYPE axis_0 = axis._v.v._0;
  FLOATTYPE axis_1 = axis._v.v._1;
  FLOATTYPE axis_2 = axis._v.v._2;

  // Normalize the axis.
  FLOATTYPE length_sq = axis_0 * axis_0 + axis_1 * axis_1 + axis_2 * axis_2;
#ifdef _DEBUG
  nassertr(length_sq != 0.0f, ident_mat());
#endif
  FLOATTYPE recip_length = 1.0f/csqrt(length_sq);

  axis_0 *= recip_length;
  axis_1 *= recip_length;
  axis_2 *= recip_length;

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

/*
  mat._m.m._00 = t * axis._v.v._0 * axis._v.v._0 + c;
  mat._m.m._01 = t * axis._v.v._0 * axis._v.v._1 + s * axis._v.v._2;
  mat._m.m._02 = t * axis._v.v._0 * axis._v.v._2 - s * axis._v.v._1;

  mat._m.m._10 = t * axis._v.v._1 * axis._v.v._0 - s * axis._v.v._2;
  mat._m.m._11 = t * axis._v.v._1 * axis._v.v._1 + c;
  mat._m.m._12 = t * axis._v.v._1 * axis._v.v._2 + s * axis._v.v._0;

  mat._m.m._20 = t * axis._v.v._2 * axis._v.v._0 + s * axis._v.v._1;
  mat._m.m._21 = t * axis._v.v._2 * axis._v.v._1 - s * axis._v.v._0;
  mat._m.m._22 = t * axis._v.v._2 * axis._v.v._2 + c;
*/

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
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }
  FLOATNAME(LMatrix3) mat;

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

  mat._m.m._00 = t0 * axis_0 + c;
  mat._m.m._01 = t0 * axis_1 + s2;
  mat._m.m._02 = t0 * axis_2 - s1;

  mat._m.m._10 = t1 * axis_0 - s2;
  mat._m.m._11 = t1 * axis_1 + c;
  mat._m.m._12 = t1 * axis_2 + s0;

  mat._m.m._20 = t2 * axis_0 + s1;
  mat._m.m._21 = t2 * axis_1 - s0;
  mat._m.m._22 = t2 * axis_2 + c;

/*
  mat._m.m._00 = t * axis._v.v._0 * axis._v.v._0 + c;
  mat._m.m._01 = t * axis._v.v._0 * axis._v.v._1 + s * axis._v.v._2;
  mat._m.m._02 = t * axis._v.v._0 * axis._v.v._2 - s * axis._v.v._1;

  mat._m.m._10 = t * axis._v.v._1 * axis._v.v._0 - s * axis._v.v._2;
  mat._m.m._11 = t * axis._v.v._1 * axis._v.v._1 + c;
  mat._m.m._12 = t * axis._v.v._1 * axis._v.v._2 + s * axis._v.v._0;

  mat._m.m._20 = t * axis._v.v._2 * axis._v.v._0 + s * axis._v.v._1;
  mat._m.m._21 = t * axis._v.v._2 * axis._v.v._1 - s * axis._v.v._0;
  mat._m.m._22 = t * axis._v.v._2 * axis._v.v._2 + c;
*/

  return mat;
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
//     Function: LMatrix::shear_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               shear in each of the three planes.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
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
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
shear_mat(FLOATTYPE shxy, FLOATTYPE shxz, FLOATTYPE shyz, CoordinateSystem cs) {
  return scale_shear_mat(1.0f, 1.0f, 1.0f,
                         shxy, shxz, shyz, cs);
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
  return scale_shear_mat(FLOATNAME(LVecBase3)(sx, sy, sz),
                         FLOATNAME(LVecBase3)(shxy, shxz, shyz),
                         cs);
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
  FLOATNAME(LMatrix3) result;
  bool nonsingular = result.invert_from(a);
  nassertr(nonsingular, FLOATNAME(LMatrix3)::ident_mat());
  return result;
}
