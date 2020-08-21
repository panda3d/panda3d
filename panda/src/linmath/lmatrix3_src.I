/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lmatrix3_src.I
 * @author drose
 * @date 1999-01-29
 */

/**
 * Defines a row-level index accessor to the matrix.
 */
INLINE_LINMATH FLOATNAME(LMatrix3)::Row::
Row(FLOATTYPE *row) : _row(row) {
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix3)::Row::
operator [](int i) const {
  nassertr(i >= 0 && i < 3, 0.0);
  return _row[i];
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE &FLOATNAME(LMatrix3)::Row::
operator [](int i) {
  nassertr(i >= 0 && i < 3, _row[0]);
  return _row[i];
}

/**
 * Returns 3: the number of columns of a LMatrix3.
 */
INLINE_LINMATH int FLOATNAME(LMatrix3)::Row::
size() {
  return 3;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix3)::Row::
operator const FLOATNAME(LVecBase3) &() const {
  return *(const FLOATNAME(LVecBase3) *)_row;
}

/**
 * Defines a row-level constant accessor to the matrix.
 */
INLINE_LINMATH FLOATNAME(LMatrix3)::CRow::
CRow(const FLOATTYPE *row) : _row(row) {
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix3)::CRow::
operator [](int i) const {
  nassertr(i >= 0 && i < 3, 0.0);
  return _row[i];
}

/**
 * Returns 3: the number of columns of a LMatrix3.
 */
INLINE_LINMATH int FLOATNAME(LMatrix3)::CRow::
size() {
  return 3;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix3)::CRow::
operator const FLOATNAME(LVecBase3) &() const {
  return *(const FLOATNAME(LVecBase3) *)_row;
}

/**
 * Returns an identity matrix.
 *
 * This function definition must appear first, since some inline functions
 * below take advantage of it.
 */
INLINE_LINMATH const FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
ident_mat() {
  return _ident_mat;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator = (FLOATTYPE fill_value) {
  fill(fill_value);
  return *this;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix3)::
FLOATNAME(LMatrix3)(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02,
                    FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12,
                    FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22) {
  TAU_PROFILE("LMatrix3::LMatrix3(FLOATTYPE, ...)", " ", TAU_USER);
  _m(0, 0) = e00;
  _m(0, 1) = e01;
  _m(0, 2) = e02;
  _m(1, 0) = e10;
  _m(1, 1) = e11;
  _m(1, 2) = e12;
  _m(2, 0) = e20;
  _m(2, 1) = e21;
  _m(2, 2) = e22;
}

/**
 * Constructs the matrix from three individual rows.
 */
INLINE_LINMATH FLOATNAME(LMatrix3)::
FLOATNAME(LMatrix3)(const FLOATNAME(LVecBase3) &row0,
                    const FLOATNAME(LVecBase3) &row1,
                    const FLOATNAME(LVecBase3) &row2) {
  TAU_PROFILE("LMatrix3::LMatrix3(const LVecBase3 &, ...)", " ", TAU_USER);

  _m(0, 0) = row0._v(0);
  _m(0, 1) = row0._v(1);
  _m(0, 2) = row0._v(2);
  _m(1, 0) = row1._v(0);
  _m(1, 1) = row1._v(1);
  _m(1, 2) = row1._v(2);
  _m(2, 0) = row2._v(0);
  _m(2, 1) = row2._v(1);
  _m(2, 2) = row2._v(2);
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02,
    FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12,
    FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22) {
  TAU_PROFILE("void LMatrix3::set(FLOATTYPE, ...)", " ", TAU_USER);
  _m(0, 0) = e00;
  _m(0, 1) = e01;
  _m(0, 2) = e02;
  _m(1, 0) = e10;
  _m(1, 1) = e11;
  _m(1, 2) = e12;
  _m(2, 0) = e20;
  _m(2, 1) = e21;
  _m(2, 2) = e22;
}

/**
 * Replaces the indicated row of the matrix from a three-component vector.
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_row(int row, const FLOATNAME(LVecBase3) &v) {
#ifdef HAVE_EIGEN
  _m.row(row) = v._v;
#else
  (*this)(row, 0) = v._v(0);
  (*this)(row, 1) = v._v(1);
  (*this)(row, 2) = v._v(2);
#endif  // HAVE_EIGEN
}

/**
 * Replaces the indicated column of the matrix from a three-component vector.
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_col(int col, const FLOATNAME(LVecBase3) &v) {
#ifdef HAVE_EIGEN
  _m.col(col) = v._v;
#else
  (*this)(0, col) = v._v(0);
  (*this)(1, col) = v._v(1);
  (*this)(2, col) = v._v(2);
#endif  // HAVE_EIGEN
}

/**
 * Replaces the indicated row of the matrix from a two-component vector,
 * ignoring the last column.
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_row(int row, const FLOATNAME(LVecBase2) &v) {
#ifdef HAVE_EIGEN
  _m.block<1, 2>(row, 0) = v._v;
#else
  (*this)(row, 0) = v._v(0);
  (*this)(row, 1) = v._v(1);
#endif  // HAVE_EIGEN
}

/**
 * Replaces the indicated column of the matrix from a two-component vector,
 * ignoring the last row.
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_col(int col, const FLOATNAME(LVecBase2) &v) {
#ifdef HAVE_EIGEN
  _m.block<2, 1>(0, col) = v._v;
#else
  (*this)(0, col) = v._v(0);
  (*this)(1, col) = v._v(1);
#endif  // HAVE_EIGEN
}

/**
 * Returns the indicated row of the matrix as a three-component vector.
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix3)::
get_row(int row) const {
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase3)(_m.row(row));
#else
  return FLOATNAME(LVecBase3)((*this)(row, 0), (*this)(row, 1), (*this)(row, 2));
#endif  // HAVE_EIGEN
}

/**
 * Stores the indicated row of the matrix as a three-component vector.
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
get_row(FLOATNAME(LVecBase3) &result_vec,int row) const {
#ifdef HAVE_EIGEN
  result_vec._v = _m.row(row);
#else
  result_vec._v(0) = (*this)(row, 0);
  result_vec._v(1) = (*this)(row, 1);
  result_vec._v(2) = (*this)(row, 2);
#endif  // HAVE_EIGEN
}

/**
 * Returns the indicated column of the matrix as a three-component vector.
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix3)::
get_col(int col) const {
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase3)(_m.col(col));
#else
  return FLOATNAME(LVecBase3)((*this)(0, col), (*this)(1, col), (*this)(2, col));
#endif  // HAVE_EIGEN
}

/**
 * Returns the indicated row of the matrix as a two-component vector, ignoring
 * the last column.
 */
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LMatrix3)::
get_row2(int row) const {
  return FLOATNAME(LVecBase2)((*this)(row, 0), (*this)(row, 1));
}

/**
 * Returns the indicated column of the matrix as a two-component vector,
 * ignoring the last row.
 */
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LMatrix3)::
get_col2(int col) const {
  return FLOATNAME(LVecBase2)((*this)(0, col), (*this)(1, col));
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE &FLOATNAME(LMatrix3)::
operator () (int row, int col) {
  nassertr(row >= 0 && row < 3 && col >= 0 && col < 3, _m(0, 0));
  return _m(row, col);
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix3)::
operator () (int row, int col) const {
  nassertr(row >= 0 && row < 3 && col >= 0 && col < 3, 0.0);
  return _m(row, col);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix3)::CRow FLOATNAME(LMatrix3)::
operator [](int i) const {
  nassertr(i >= 0 && i < 3, CRow(&_m(0, 0)));
  return CRow(&_m(i, 0));
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix3)::Row FLOATNAME(LMatrix3)::
operator [](int i) {
  nassertr(i >= 0 && i < 3, Row(&_m(0, 0)));
  return Row(&_m(i, 0));
}

/**
 * Returns 3: the number of rows of a LMatrix3.
 */
INLINE_LINMATH int FLOATNAME(LMatrix3)::
size() {
  return 3;
}

/**
 * Returns true if any component of the matrix is not-a-number, false
 * otherwise.
 */
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
is_nan() const {
  TAU_PROFILE("bool LMatrix3::is_nan()", " ", TAU_USER);
  return
    cnan(_m(0, 0)) || cnan(_m(0, 1)) || cnan(_m(0, 2)) ||
    cnan(_m(1, 0)) || cnan(_m(1, 1)) || cnan(_m(1, 2)) ||
    cnan(_m(2, 0)) || cnan(_m(2, 1)) || cnan(_m(2, 2));
}

/**
 * Returns true if this is (close enough to) the identity matrix, false
 * otherwise.
 */
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
is_identity() const {
  return almost_equal(ident_mat(), NEARLY_ZERO(FLOATTYPE));
}

/**
 * Returns a particular element of the matrix.
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix3)::
get_cell(int row, int col) const {
  nassertr(row >= 0 && row < 3 && col >= 0 && col < 3, 0.0);
  return _m(row, col);
}

/**
 * Changes a particular element of the matrix.
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_cell(int row, int col, FLOATTYPE value) {
  nassertv(row >= 0 && row < 3 && col >= 0 && col < 3);
  _m(row, col) = value;
}

/**
 * Returns the address of the first of the nine data elements in the matrix.
 * The remaining elements occupy the next eight positions in row-major order.
 */
INLINE_LINMATH const FLOATTYPE *FLOATNAME(LMatrix3)::
get_data() const {
  return &_m(0, 0);
}

/**
 * Returns the number of elements in the matrix, nine.
 */
INLINE_LINMATH int FLOATNAME(LMatrix3)::
get_num_components() const {
  return 9;
}

/**
 * Returns an iterator that may be used to traverse the elements of the
 * matrix, STL-style.
 */
INLINE_LINMATH FLOATNAME(LMatrix3)::iterator FLOATNAME(LMatrix3)::
begin() {
  return &_m(0, 0);
}

/**
 * Returns an iterator that may be used to traverse the elements of the
 * matrix, STL-style.
 */
INLINE_LINMATH FLOATNAME(LMatrix3)::iterator FLOATNAME(LMatrix3)::
end() {
  return begin() + num_components;
}

/**
 * Returns an iterator that may be used to traverse the elements of the
 * matrix, STL-style.
 */
INLINE_LINMATH FLOATNAME(LMatrix3)::const_iterator FLOATNAME(LMatrix3)::
begin() const {
  return &_m(0, 0);
}

/**
 * Returns an iterator that may be used to traverse the elements of the
 * matrix, STL-style.
 */
INLINE_LINMATH FLOATNAME(LMatrix3)::const_iterator FLOATNAME(LMatrix3)::
end() const {
  return begin() + num_components;
}

/**
 * This performs a lexicographical comparison.  It's of questionable
 * mathematical meaning, but sometimes has a practical purpose for sorting
 * unique vectors, especially in an STL container.  Also see compare_to().
 */
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
operator < (const FLOATNAME(LMatrix3) &other) const {
  return compare_to(other) < 0;
}

/**
 *
 */
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
operator == (const FLOATNAME(LMatrix3) &other) const {
  return compare_to(other) == 0;
}

/**
 *
 */
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
operator != (const FLOATNAME(LMatrix3) &other) const {
  return compare_to(other) != 0;
}

/**
 * This flavor of compare_to uses a default threshold value based on the
 * numeric type.
 */
INLINE_LINMATH int FLOATNAME(LMatrix3)::
compare_to(const FLOATNAME(LMatrix3) &other) const {
  return compare_to(other, NEARLY_ZERO(FLOATTYPE));
}

/**
 * Returns a suitable hash for phash_map.
 */
INLINE_LINMATH size_t FLOATNAME(LMatrix3)::
get_hash() const {
  return add_hash(0);
}

/**
 * Returns a suitable hash for phash_map.
 */
INLINE_LINMATH size_t FLOATNAME(LMatrix3)::
get_hash(FLOATTYPE threshold) const {
  return add_hash(0, threshold);
}

/**
 * Adds the vector into the running hash.
 */
INLINE_LINMATH size_t FLOATNAME(LMatrix3)::
add_hash(size_t hash) const {
  return add_hash(hash, NEARLY_ZERO(FLOATTYPE));
}

/**
 * Adds the vector into the running hash.
 */
INLINE_LINMATH size_t FLOATNAME(LMatrix3)::
add_hash(size_t hash, FLOATTYPE threshold) const {
  TAU_PROFILE("size_t LMatrix3::add_hash(size_t, FLOATTYPE)", " ", TAU_USER);
  float_hash fhasher(threshold);

  hash = fhasher.add_hash(hash, _m(0, 0));
  hash = fhasher.add_hash(hash, _m(0, 1));
  hash = fhasher.add_hash(hash, _m(0, 2));

  hash = fhasher.add_hash(hash, _m(1, 0));
  hash = fhasher.add_hash(hash, _m(1, 1));
  hash = fhasher.add_hash(hash, _m(1, 2));

  hash = fhasher.add_hash(hash, _m(2, 0));
  hash = fhasher.add_hash(hash, _m(2, 1));
  hash = fhasher.add_hash(hash, _m(2, 2));

  return hash;
}

#define VECTOR3_MATRIX3_PRODUCT(v_res, v, mat)                                              \
v_res._v(0) = v._v(0)*mat._m(0, 0) + v._v(1)*mat._m(1, 0) + v._v(2)*mat._m(2, 0);   \
v_res._v(1) = v._v(0)*mat._m(0, 1) + v._v(1)*mat._m(1, 1) + v._v(2)*mat._m(2, 1);   \
v_res._v(2) = v._v(0)*mat._m(0, 2) + v._v(1)*mat._m(1, 2) + v._v(2)*mat._m(2, 2);

/**
 * 3-component vector or point times matrix.
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix3)::
xform(const FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix3::xform(const LVecBase3 &)", " ", TAU_USER);
  FLOATNAME(LVecBase3) v_res;
#ifdef HAVE_EIGEN
  v_res._v.noalias() = v._v * _m;
#else
  VECTOR3_MATRIX3_PRODUCT(v_res, v,(*this));
#endif  // HAVE_EIGEN

  return v_res;
}

#undef VECTOR3_MATRIX3_PRODUCT

/**
 * The matrix transforms a 2-component point (including translation component)
 * and returns the result.  This assumes the matrix is an affine transform.
 */
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LMatrix3)::
xform_point(const FLOATNAME(LVecBase2) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix3::xform_point(const LVecBase3 &)", " ", TAU_USER);
  FLOATNAME(LVecBase2) v_res;

  // v._v(2) == 1.0f for this case

#ifdef HAVE_EIGEN
  v_res._v.noalias() = v._v * _m.block<2, 2>(0, 0) + _m.block<1, 2>(2, 0);
#else
  v_res._v(0) = v._v(0)*_m(0, 0) + v._v(1)*_m(1, 0) + _m(2, 0);
  v_res._v(1) = v._v(0)*_m(0, 1) + v._v(1)*_m(1, 1) + _m(2, 1);
#endif  // HAVE_EIGEN

  return v_res;
}

/**
 * The matrix transforms a 2-component vector (without translation component)
 * and returns the result.  This assumes the matrix is an affine transform.
 */
INLINE_LINMATH FLOATNAME(LVecBase2) FLOATNAME(LMatrix3)::
xform_vec(const FLOATNAME(LVecBase2) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix3::xform_vec(const LVecBase3 &)", " ", TAU_USER);

  FLOATNAME(LVecBase2) v_res;

  // v._v(2) == 0.0f for this case

#ifdef HAVE_EIGEN
  v_res._v.noalias() = v._v * _m.block<2, 2>(0, 0);
#else
  v_res._v(0) = v._v(0)*_m(0, 0) + v._v(1)*_m(1, 0);
  v_res._v(1) = v._v(0)*_m(0, 1) + v._v(1)*_m(1, 1);
#endif  // HAVE_EIGEN

  return v_res;
}

/**
 * The matrix transforms a 3-component vector and returns the result.  This
 * assumes the matrix is an orthonormal transform.
 *
 * In practice, this is the same computation as xform().
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix3)::
xform_vec(const FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix3::xform_vec(const LVecBase3 &)", " ", TAU_USER);
  return xform(v);
}

/**
 * The matrix transforms a 3-component vector (without translation component)
 * and returns the result, as a fully general operation.
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix3)::
xform_vec_general(const FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix3::xform_vec_general(const LVecBase3 &)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase3)(v._v * _m.inverse().transpose());
#else
  FLOATNAME(LMatrix3) i;
  i.invert_transpose_from(*this);

  return i.xform(v);
#endif  // HAVE_EIGEN
}

/**
 * 3-component vector or point times matrix.
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
xform_in_place(FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("void LMatrix3::xform_in_place(LVecBase3 &)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  v._v = v._v * _m;
#else
  v = xform(v);
#endif  // HAVE_EIGEN
}

/**
 * The matrix transforms a 2-component point (including translation
 * component).  This assumes the matrix is an affine transform.
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
xform_point_in_place(FLOATNAME(LVecBase2) &v) const {
  TAU_PROFILE("void LMatrix3::xform_point_in_place(LVecBase3 &)", " ", TAU_USER);
  // v._v(2) == 1.0f for this case

#ifdef HAVE_EIGEN
  v._v = v._v * _m.block<2, 2>(0, 0) + _m.block<1, 2>(2, 0);
#else
  v = xform_point(v);
#endif  // HAVE_EIGEN
}

/**
 * The matrix transforms a 2-component vector (without translation component).
 * This assumes the matrix is an affine transform.
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
xform_vec_in_place(FLOATNAME(LVecBase2) &v) const {
  TAU_PROFILE("void LMatrix3::xform_vec_in_place(LVecBase3 &)", " ", TAU_USER);
  // v._v(2) == 0.0f for this case

#ifdef HAVE_EIGEN
  v._v = v._v * _m.block<2, 2>(0, 0);
#else
  v = xform_vec(v);
#endif  // HAVE_EIGEN
}

/**
 * The matrix transforms a 3-component vector.  This assumes the matrix is an
 * orthonormal transform.
 *
 * In practice, this is the same computation as xform().
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
xform_vec_in_place(FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("void LMatrix3::xform_vec_in_place(LVecBase3 &)", " ", TAU_USER);
  xform_in_place(v);
}

/**
 * The matrix transforms a 3-component vector (without translation component),
 * as a fully general operation.
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
xform_vec_general_in_place(FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("void LMatrix3::xform_vec_general_in_place(LVecBase3 &)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  v._v = v._v * _m.inverse().transpose();
#else
  v = xform_vec(v);
#endif  // HAVE_EIGEN
}

#define MATRIX3_PRODUCT(res, a, b)                   \
res._m(0, 0) = a._m(0, 0)*b._m(0, 0) + a._m(0, 1)*b._m(1, 0) + a._m(0, 2)*b._m(2, 0);   \
res._m(0, 1) = a._m(0, 0)*b._m(0, 1) + a._m(0, 1)*b._m(1, 1) + a._m(0, 2)*b._m(2, 1);   \
res._m(0, 2) = a._m(0, 0)*b._m(0, 2) + a._m(0, 1)*b._m(1, 2) + a._m(0, 2)*b._m(2, 2);   \
res._m(1, 0) = a._m(1, 0)*b._m(0, 0) + a._m(1, 1)*b._m(1, 0) + a._m(1, 2)*b._m(2, 0);   \
res._m(1, 1) = a._m(1, 0)*b._m(0, 1) + a._m(1, 1)*b._m(1, 1) + a._m(1, 2)*b._m(2, 1);   \
res._m(1, 2) = a._m(1, 0)*b._m(0, 2) + a._m(1, 1)*b._m(1, 2) + a._m(1, 2)*b._m(2, 2);   \
res._m(2, 0) = a._m(2, 0)*b._m(0, 0) + a._m(2, 1)*b._m(1, 0) + a._m(2, 2)*b._m(2, 0);   \
res._m(2, 1) = a._m(2, 0)*b._m(0, 1) + a._m(2, 1)*b._m(1, 1) + a._m(2, 2)*b._m(2, 1);   \
res._m(2, 2) = a._m(2, 0)*b._m(0, 2) + a._m(2, 1)*b._m(1, 2) + a._m(2, 2)*b._m(2, 2);


/**
 *
 */
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
  // faster than operator * since it writes result in place, avoiding extra
  // copying this will fail if you try to mat.multiply(mat,other_mat)

  nassertv((&other1 != this) && (&other2 != this));

  MATRIX3_PRODUCT((*this), other1, other2);
}

#undef MATRIX3_PRODUCT

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
operator * (FLOATTYPE scalar) const {
  TAU_PROFILE("LMatrix3 operator *(const LMatrix3 &, FLOATTYPE)", " ", TAU_USER);
  FLOATNAME(LMatrix3) t;

  t._m(0, 0) = _m(0, 0) * scalar;
  t._m(0, 1) = _m(0, 1) * scalar;
  t._m(0, 2) = _m(0, 2) * scalar;

  t._m(1, 0) = _m(1, 0) * scalar;
  t._m(1, 1) = _m(1, 1) * scalar;
  t._m(1, 2) = _m(1, 2) * scalar;

  t._m(2, 0) = _m(2, 0) * scalar;
  t._m(2, 1) = _m(2, 1) * scalar;
  t._m(2, 2) = _m(2, 2) * scalar;

  return t;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
operator / (FLOATTYPE scalar) const {
  FLOATTYPE recip_scalar = 1.0f/scalar;
  return (*this) * recip_scalar;
}

/**
 * Performs a memberwise addition between two matrices.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator += (const FLOATNAME(LMatrix3) &other) {
  TAU_PROFILE("LMatrix3 LMatrix3::operator +=(const LMatrix3 &)", " ", TAU_USER);
  _m(0, 0) += other._m(0, 0);
  _m(0, 1) += other._m(0, 1);
  _m(0, 2) += other._m(0, 2);

  _m(1, 0) += other._m(1, 0);
  _m(1, 1) += other._m(1, 1);
  _m(1, 2) += other._m(1, 2);

  _m(2, 0) += other._m(2, 0);
  _m(2, 1) += other._m(2, 1);
  _m(2, 2) += other._m(2, 2);

  return *this;
}

/**
 * Performs a memberwise subtraction between two matrices.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator -= (const FLOATNAME(LMatrix3) &other) {
  TAU_PROFILE("LMatrix3 LMatrix3::operator -=(const LMatrix3 &)", " ", TAU_USER);
  _m(0, 0) -= other._m(0, 0);
  _m(0, 1) -= other._m(0, 1);
  _m(0, 2) -= other._m(0, 2);

  _m(1, 0) -= other._m(1, 0);
  _m(1, 1) -= other._m(1, 1);
  _m(1, 2) -= other._m(1, 2);

  _m(2, 0) -= other._m(2, 0);
  _m(2, 1) -= other._m(2, 1);
  _m(2, 2) -= other._m(2, 2);

  return *this;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator *= (const FLOATNAME(LMatrix3) &other) {
  TAU_PROFILE("LMatrix3 LMatrix3::operator *=(const LMatrix3 &)", " ", TAU_USER);
  FLOATNAME(LMatrix3) temp = *this;
  multiply(temp, other);

  return *this;
}

/**
 * Performs a memberwise scale.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator *= (FLOATTYPE scalar) {
  TAU_PROFILE("LMatrix3 LMatrix3::operator *=(FLOATTYPE)", " ", TAU_USER);
  _m(0, 0) *= scalar;
  _m(0, 1) *= scalar;
  _m(0, 2) *= scalar;

  _m(1, 0) *= scalar;
  _m(1, 1) *= scalar;
  _m(1, 2) *= scalar;

  _m(2, 0) *= scalar;
  _m(2, 1) *= scalar;
  _m(2, 2) *= scalar;

  return *this;
}

/**
 * Performs a memberwise scale.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator /= (FLOATTYPE scalar) {
  TAU_PROFILE("LMatrix3 LMatrix3::operator /=(FLOATTYPE)", " ", TAU_USER);
  FLOATTYPE recip_scalar = 1.0f/scalar;
  _m(0, 0) *= recip_scalar;
  _m(0, 1) *= recip_scalar;
  _m(0, 2) *= recip_scalar;

  _m(1, 0) *= recip_scalar;
  _m(1, 1) *= recip_scalar;
  _m(1, 2) *= recip_scalar;

  _m(2, 0) *= recip_scalar;
  _m(2, 1) *= recip_scalar;
  _m(2, 2) *= recip_scalar;

  return *this;
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
componentwise_mult(const FLOATNAME(LMatrix3) &other) {
#ifdef HAVE_EIGEN
  _m = _m.cwiseProduct(other._m);
#else
  _m(0, 0) *= other._m(0, 0);
  _m(0, 1) *= other._m(0, 1);
  _m(0, 2) *= other._m(0, 2);

  _m(1, 0) *= other._m(1, 0);
  _m(1, 1) *= other._m(1, 1);
  _m(1, 2) *= other._m(1, 2);

  _m(2, 0) *= other._m(2, 0);
  _m(2, 1) *= other._m(2, 1);
  _m(2, 2) *= other._m(2, 2);
#endif  // HAVE_EIGEN
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
transpose_from(const FLOATNAME(LMatrix3) &other) {
  TAU_PROFILE("LMatrix3 LMatrix3::transpose_from(const LMatrix3 &other)", " ", TAU_USER);
  _m(0, 0) = other._m(0, 0);
  _m(0, 1) = other._m(1, 0);
  _m(0, 2) = other._m(2, 0);

  _m(1, 0) = other._m(0, 1);
  _m(1, 1) = other._m(1, 1);
  _m(1, 2) = other._m(2, 1);

  _m(2, 0) = other._m(0, 2);
  _m(2, 1) = other._m(1, 2);
  _m(2, 2) = other._m(2, 2);
}


/**
 *
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
transpose_in_place() {
  TAU_PROFILE("void LMatrix3::transpose_in_place()", " ", TAU_USER);

  std::swap(_m(0, 1), _m(1, 0));
  std::swap(_m(0, 2), _m(2, 0));
  std::swap(_m(1, 2), _m(2, 1));
}

// Matrix inversion code from Numerical Recipes in C.

// don't trust compilers to inline these
#define DET2(E00,E01,E10,E11) ((E00)*(E11) - (E10)*(E01))
#define MATRIX3_DETERMINANT(mat)                                  \
   ( (mat)(0, 0) * DET2((mat)(1, 1),(mat)(1, 2),(mat)(2, 1),(mat)(2, 2))    \
    -(mat)(0, 1) * DET2((mat)(1, 0),(mat)(1, 2),(mat)(2, 0),(mat)(2, 2))    \
    +(mat)(0, 2) * DET2((mat)(1, 0),(mat)(1, 1),(mat)(2, 0),(mat)(2, 1)))

/**
 * Returns the determinant of the matrix.
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix3)::
determinant() const {
  TAU_PROFILE("FLOATTYPE LMatrix3::determinant()", " ", TAU_USER);

#ifdef HAVE_EIGEN
  return _m.determinant();
#else
 return MATRIX3_DETERMINANT(_m);
#endif  // HAVE_EIGEN
}

/**
 * Computes the inverse of the other matrix, and stores the result in this
 * matrix.  This is a fully general operation and makes no assumptions about
 * the type of transform represented by the matrix.
 *
 * The other matrix must be a different object than this matrix.  However, if
 * you need to invert a matrix in place, see invert_in_place.
 *
 * The return value is true if the matrix was successfully inverted, false if
 * there was a singularity.
 */
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
invert_from(const FLOATNAME(LMatrix3) &other) {
  TAU_PROFILE("bool LMatrix3::invert_from(const LMatrix3 &)", " ", TAU_USER);

  // We throw the value out only if it's smaller than our "small" threshold
  // squared.  This helps reduce overly-sensitive rejections.
#ifdef HAVE_EIGEN
  bool invertible;
  other._m.computeInverseWithCheck(_m, invertible,
    NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE));

  if (!invertible) {
#ifdef NOTIFY_DEBUG
    linmath_cat.warning() << "Tried to invert singular LMatrix3.\n";
#endif
    (*this) = ident_mat();
    nassertr(!no_singular_invert, false);
  }

  return invertible;

#else  // HAVE_EIGEN

  FLOATTYPE other_det = MATRIX3_DETERMINANT(other._m);

  if (IS_THRESHOLD_ZERO(other_det, (NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE)))) {
#ifdef NOTIFY_DEBUG
    linmath_cat.warning() << "Tried to invert singular LMatrix3.\n";
#endif
    (*this) = ident_mat();
    nassertr(!no_singular_invert, false);
    return false;
  }

  other_det = 1.0f / other_det;
  _m(0, 0) =  other_det * DET2(other._m(1, 1), other._m(1, 2), other._m(2, 1), other._m(2, 2));
  _m(1, 0) = -other_det * DET2(other._m(1, 0), other._m(1, 2), other._m(2, 0), other._m(2, 2));
  _m(2, 0) =  other_det * DET2(other._m(1, 0), other._m(1, 1), other._m(2, 0), other._m(2, 1));

  _m(0, 1) = -other_det * DET2(other._m(0, 1), other._m(0, 2), other._m(2, 1), other._m(2, 2));
  _m(1, 1) =  other_det * DET2(other._m(0, 0), other._m(0, 2), other._m(2, 0), other._m(2, 2));
  _m(2, 1) = -other_det * DET2(other._m(0, 0), other._m(0, 1), other._m(2, 0), other._m(2, 1));

  _m(0, 2) =  other_det * DET2(other._m(0, 1), other._m(0, 2), other._m(1, 1), other._m(1, 2));
  _m(1, 2) = -other_det * DET2(other._m(0, 0), other._m(0, 2), other._m(1, 0), other._m(1, 2));
  _m(2, 2) =  other_det * DET2(other._m(0, 0), other._m(0, 1), other._m(1, 0), other._m(1, 1));

  return true;
#endif  // HAVE_EIGEN
}

/**
 * Inverts the current matrix.  Returns true if the inverse is successful,
 * false if the matrix was singular.
 */
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
invert_in_place() {
  TAU_PROFILE("bool LMatrix3::invert_in_place()", " ", TAU_USER);
  FLOATNAME(LMatrix3) temp = (*this);
  return invert_from(temp);
}

/**
 * Simultaneously computes the inverse of the indicated matrix, and then the
 * transpose of that inverse.
 */
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
invert_transpose_from(const FLOATNAME(LMatrix3) &other) {
  TAU_PROFILE("bool LMatrix3::invert_transpose_from(const LMatrix3 &)", " ", TAU_USER);

#ifdef HAVE_EIGEN
  bool invertible;
  EMatrix3 temp;
  other._m.computeInverseWithCheck(temp, invertible,
    NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE));

  if (!invertible) {
#ifdef NOTIFY_DEBUG
    linmath_cat.warning() << "Tried to invert singular LMatrix3.\n";
#endif
    (*this) = ident_mat();
    nassertr(!no_singular_invert, false);
  }

  _m = temp.transpose();
  return true;

#else  // HAVE_EIGEN

  FLOATTYPE other_det = MATRIX3_DETERMINANT(other._m);

  if (IS_THRESHOLD_ZERO(other_det, (NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE)))) {
#ifdef NOTIFY_DEBUG
    linmath_cat.warning() << "Tried to invert singular LMatrix3.\n";
#endif
    (*this) = ident_mat();
    nassertr(!no_singular_invert, false);
    return false;
  }

  other_det = 1.0f / other_det;
  _m(0, 0) =  other_det * DET2(other._m(1, 1), other._m(1, 2), other._m(2, 1), other._m(2, 2));
  _m(0, 1) = -other_det * DET2(other._m(1, 0), other._m(1, 2), other._m(2, 0), other._m(2, 2));
  _m(0, 2) =  other_det * DET2(other._m(1, 0), other._m(1, 1), other._m(2, 0), other._m(2, 1));

  _m(1, 0) = -other_det * DET2(other._m(0, 1), other._m(0, 2), other._m(2, 1), other._m(2, 2));
  _m(1, 1) =  other_det * DET2(other._m(0, 0), other._m(0, 2), other._m(2, 0), other._m(2, 2));
  _m(1, 2) = -other_det * DET2(other._m(0, 0), other._m(0, 1), other._m(2, 0), other._m(2, 1));

  _m(2, 0) =  other_det * DET2(other._m(0, 1), other._m(0, 2), other._m(1, 1), other._m(1, 2));
  _m(2, 1) = -other_det * DET2(other._m(0, 0), other._m(0, 2), other._m(1, 0), other._m(1, 2));
  _m(2, 2) =  other_det * DET2(other._m(0, 0), other._m(0, 1), other._m(1, 0), other._m(1, 1));

  return true;
#endif  // HAVE_EIGEN
}

/**
 * Simultaneously computes the inverse of the indicated matrix, and then the
 * transpose of that inverse.
 */
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
invert_transpose_from(const FLOATNAME(LMatrix4) &other) {
  TAU_PROFILE("bool LMatrix3::invert_transpose_from(const LMatrix4 &)", " ", TAU_USER);

#ifdef HAVE_EIGEN
  bool invertible;
  EMatrix3 temp;
  other._m.block<3, 3>(0, 0).computeInverseWithCheck(temp, invertible,
    NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE));

  if (!invertible) {
#ifdef NOTIFY_DEBUG
    linmath_cat.warning() << "Tried to invert singular LMatrix3.\n";
#endif
    (*this) = ident_mat();
    nassertr(!no_singular_invert, false);
  }

  _m = temp.transpose();
  return true;

#else  // HAVE_EIGEN

  FLOATTYPE other_det = MATRIX3_DETERMINANT(other._m);

  if (IS_THRESHOLD_ZERO(other_det, (NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE)))) {
#ifdef NOTIFY_DEBUG
    linmath_cat.warning() << "Tried to invert singular LMatrix4.\n";
#endif
    (*this) = ident_mat();
    nassertr(!no_singular_invert, false);
    return false;
  }

  other_det = 1.0f / other_det;
  _m(0, 0) =  other_det * DET2(other._m(1, 1), other._m(1, 2), other._m(2, 1), other._m(2, 2));
  _m(0, 1) = -other_det * DET2(other._m(1, 0), other._m(1, 2), other._m(2, 0), other._m(2, 2));
  _m(0, 2) =  other_det * DET2(other._m(1, 0), other._m(1, 1), other._m(2, 0), other._m(2, 1));

  _m(1, 0) = -other_det * DET2(other._m(0, 1), other._m(0, 2), other._m(2, 1), other._m(2, 2));
  _m(1, 1) =  other_det * DET2(other._m(0, 0), other._m(0, 2), other._m(2, 0), other._m(2, 2));
  _m(1, 2) = -other_det * DET2(other._m(0, 0), other._m(0, 1), other._m(2, 0), other._m(2, 1));

  _m(2, 0) =  other_det * DET2(other._m(0, 1), other._m(0, 2), other._m(1, 1), other._m(1, 2));
  _m(2, 1) = -other_det * DET2(other._m(0, 0), other._m(0, 2), other._m(1, 0), other._m(1, 2));
  _m(2, 2) =  other_det * DET2(other._m(0, 0), other._m(0, 1), other._m(1, 0), other._m(1, 1));

  return true;
#endif  // HAVE_EIGEN
}

#undef MATRIX3_DETERMINANT
#undef DET2

/**
 * Fills mat with a matrix that applies the indicated translation.
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_translate_mat(const FLOATNAME(LVecBase2) &trans) {
  set(1.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      trans._v(0), trans._v(1), 1.0f);
}

/**
 * Fills mat with a matrix that rotates by the given angle in degrees
 * counterclockwise.
 */
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

/**
 * Fills mat with a matrix that applies the indicated scale in each of the two
 * axes.
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_scale_mat(const FLOATNAME(LVecBase2) &scale) {
  set(scale._v(0), 0.0f, 0.0f,
      0.0f, scale._v(1), 0.0f,
      0.0f, 0.0f, 1.0f);
}

/**
 * Returns a matrix that applies the indicated translation.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
translate_mat(const FLOATNAME(LVecBase2) &trans) {
  return FLOATNAME(LMatrix3)(1.0f, 0.0f, 0.0f,
                             0.0f, 1.0f, 0.0f,
                             trans._v(0), trans._v(1), 1.0f);
}

/**
 * Returns a matrix that applies the indicated translation.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
translate_mat(FLOATTYPE tx, FLOATTYPE ty) {
  return FLOATNAME(LMatrix3)(1.0f, 0.0f, 0.0f,
                             0.0f, 1.0f, 0.0f,
                             tx, ty, 1.0f);
}

/**
 * Returns a matrix that rotates by the given angle in degrees
 * counterclockwise.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
rotate_mat(FLOATTYPE angle) {
  FLOATNAME(LMatrix3) mat;
  mat.set_rotate_mat(angle);
  return mat;
}

/**
 * Returns a matrix that applies the indicated scale in each of the two axes.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_mat(const FLOATNAME(LVecBase2) &scale) {
  return FLOATNAME(LMatrix3)(scale._v(0), 0.0f, 0.0f,
                             0.0f, scale._v(1), 0.0f,
                             0.0f, 0.0f, 1.0f);
}

/**
 * Returns a matrix that applies the indicated scale in each of the two axes.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_mat(FLOATTYPE sx, FLOATTYPE sy) {
  return FLOATNAME(LMatrix3)(sx, 0.0f, 0.0f,
                             0.0f, sy, 0.0f,
                             0.0f, 0.0f, 1.0f);
}

/**
 * Returns a matrix that rotates by the given angle in degrees
 * counterclockwise about the indicated vector.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
rotate_mat(FLOATTYPE angle, const FLOATNAME(LVecBase3) &axis,
           CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat;
  mat.set_rotate_mat(angle, axis, cs);
  return mat;
}

/**
 * Returns a matrix that rotates by the given angle in degrees
 * counterclockwise about the indicated vector.  Assumes axis has been
 * normalized.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
rotate_mat_normaxis(FLOATTYPE angle, const FLOATNAME(LVecBase3) &axis,
           CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat;
  mat.set_rotate_mat_normaxis(angle, axis, cs);
  return mat;
}

/**
 * Fills mat with a matrix that applies the indicated scale in each of the
 * three axes.
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_scale_mat(const FLOATNAME(LVecBase3) &scale) {
  set(scale._v(0), 0.0f, 0.0f,
      0.0f, scale._v(1), 0.0f,
      0.0f, 0.0f, scale._v(2));
}

/**
 * Returns a matrix that applies the indicated scale in each of the three
 * axes.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_mat(const FLOATNAME(LVecBase3) &scale) {
  return FLOATNAME(LMatrix3)(scale._v(0), 0.0f, 0.0f,
                             0.0f, scale._v(1), 0.0f,
                             0.0f, 0.0f, scale._v(2));
}

/**
 * Returns a matrix that applies the indicated scale in each of the three
 * axes.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz) {
  return FLOATNAME(LMatrix3)(sx, 0.0f, 0.0f,
                             0.0f, sy, 0.0f,
                             0.0f, 0.0f, sz);
}

/**
 * Fills mat with a matrix that applies the indicated shear in each of the
 * three planes.
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
set_shear_mat(const FLOATNAME(LVecBase3) &shear, CoordinateSystem cs) {
  set_scale_shear_mat(FLOATNAME(LVecBase3)(1.0f, 1.0f, 1.0f),
                      shear, cs);
}

/**
 * Returns a matrix that applies the indicated shear in each of the three
 * planes.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
shear_mat(const FLOATNAME(LVecBase3) &shear, CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat;
  mat.set_shear_mat(shear, cs);
  return mat;
}

/**
 * Returns a matrix that applies the indicated shear in each of the three
 * planes.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
shear_mat(FLOATTYPE shxy, FLOATTYPE shxz, FLOATTYPE shyz, CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat;
  mat.set_shear_mat(FLOATNAME(LVecBase3)(shxy, shxz, shyz), cs);
  return mat;
}

/**
 * Returns a matrix that applies the indicated scale and shear.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_shear_mat(const FLOATNAME(LVecBase3) &scale,
                const FLOATNAME(LVecBase3) &shear,
                CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat;
  mat.set_scale_shear_mat(scale, shear, cs);
  return mat;
}

/**
 * Returns a matrix that applies the indicated scale and shear.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_shear_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz,
                FLOATTYPE shxy, FLOATTYPE shxz, FLOATTYPE shyz,
                CoordinateSystem cs) {
  FLOATNAME(LMatrix3) mat;
  mat.set_scale_shear_mat(FLOATNAME(LVecBase3)(sx, sy, sz),
                          FLOATNAME(LVecBase3)(shxy, shxz, shyz), cs);
  return mat;
}

/**
 * Returns true if two matrices are memberwise equal within a default
 * tolerance based on the numeric type.
 */
INLINE_LINMATH bool FLOATNAME(LMatrix3)::
almost_equal(const FLOATNAME(LMatrix3) &other) const {
  return almost_equal(other, NEARLY_ZERO(FLOATTYPE));
}

/**
 * Adds the vector to the indicated hash generator.
 */
INLINE_LINMATH void FLOATNAME(LMatrix3)::
generate_hash(ChecksumHashGenerator &hashgen) const {
  generate_hash(hashgen, NEARLY_ZERO(FLOATTYPE));
}


/**
 * Transposes the given matrix and returns it.
 */
INLINE_LINMATH FLOATNAME(LMatrix3)
transpose(const FLOATNAME(LMatrix3) &a) {
  FLOATNAME(LMatrix3) result;
  result.transpose_from(a);
  return result;
}

/**
 * Inverts the given matrix and returns it.
 */
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
