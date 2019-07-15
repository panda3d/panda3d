/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lmatrix4_src.I
 * @author drose
 * @date 1999-01-15
 */

/**
 * Defines a row-level index accessor to the matrix.
 */
INLINE_LINMATH FLOATNAME(LMatrix4)::Row::
Row(FLOATTYPE *row) : _row(row) {
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix4)::Row::
operator [](int i) const {
  nassertr(i >= 0 && i < 4, 0.0);
  return _row[i];
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE &FLOATNAME(LMatrix4)::Row::
operator [](int i) {
  nassertr(i >= 0 && i < 4, _row[0]);
  return _row[i];
}

/**
 * Returns 4: the number of columns of a LMatrix4.
 */
INLINE_LINMATH int FLOATNAME(LMatrix4)::Row::
size() {
  return 4;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix4)::Row::
operator const FLOATNAME(LVecBase4) &() const {
  return *(const FLOATNAME(LVecBase4) *)_row;
}

/**
 * Defines a row-level constant accessor to the matrix.
 */
INLINE_LINMATH FLOATNAME(LMatrix4)::CRow::
CRow(const FLOATTYPE *row) : _row(row) {
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix4)::CRow::
operator [](int i) const {
  nassertr(i >= 0 && i < 4, 0.0);
  return _row[i];
}

/**
 * Returns 4: the number of columns of a LMatrix4.
 */
INLINE_LINMATH int FLOATNAME(LMatrix4)::CRow::
size() {
  return 4;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix4)::CRow::
operator const FLOATNAME(LVecBase4) &() const {
  return *(const FLOATNAME(LVecBase4) *)_row;
}

/**
 * Returns an identity matrix.
 *
 * This function definition must appear first, since some inline functions
 * below take advantage of it.
 */
INLINE_LINMATH const FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
ident_mat() {
  return _ident_mat;
}

/**
 * Returns an matrix filled with ones.
 */
INLINE_LINMATH const FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
ones_mat() {
  return _ones_mat;
}

/**
 * Returns an matrix filled with zeros.
 */
INLINE_LINMATH const FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
zeros_mat() {
  return _zeros_mat;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix4)::
FLOATNAME(LMatrix4)(const FLOATNAME(UnalignedLMatrix4) &copy) {
  operator = (copy);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator = (const FLOATNAME(UnalignedLMatrix4) &copy) {
  TAU_PROFILE("void LMatrix4::operator = (const UnalignedLMatrix4 &)", " ", TAU_USER);

  memcpy(&_m(0, 0), copy.get_data(), sizeof(FLOATTYPE) * num_components);
  return *this;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator = (FLOATTYPE fill_value) {
  fill(fill_value);
  return *this;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix4)::
FLOATNAME(LMatrix4)(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02, FLOATTYPE e03,
                    FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12, FLOATTYPE e13,
                    FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22, FLOATTYPE e23,
                    FLOATTYPE e30, FLOATTYPE e31, FLOATTYPE e32, FLOATTYPE e33) {
  TAU_PROFILE("LMatrix4::LMatrix4(FLOATTYPE, ...)", " ", TAU_USER);

  _m(0, 0) = e00;
  _m(0, 1) = e01;
  _m(0, 2) = e02;
  _m(0, 3) = e03;

  _m(1, 0) = e10;
  _m(1, 1) = e11;
  _m(1, 2) = e12;
  _m(1, 3) = e13;

  _m(2, 0) = e20;
  _m(2, 1) = e21;
  _m(2, 2) = e22;
  _m(2, 3) = e23;

  _m(3, 0) = e30;
  _m(3, 1) = e31;
  _m(3, 2) = e32;
  _m(3, 3) = e33;
}

/**
 * Constructs the matrix from four individual rows.
 */
INLINE_LINMATH FLOATNAME(LMatrix4)::
FLOATNAME(LMatrix4)(const FLOATNAME(LVecBase4) &row0,
                    const FLOATNAME(LVecBase4) &row1,
                    const FLOATNAME(LVecBase4) &row2,
                    const FLOATNAME(LVecBase4) &row3) {
  TAU_PROFILE("LMatrix4::LMatrix4(const LVecBase4 &, ...)", " ", TAU_USER);

#ifdef HAVE_EIGEN
  _m.row(0) = row0._v;
  _m.row(1) = row1._v;
  _m.row(2) = row2._v;
  _m.row(3) = row3._v;
#else
  _m(0, 0) = row0._v(0);
  _m(0, 1) = row0._v(1);
  _m(0, 2) = row0._v(2);
  _m(0, 3) = row0._v(3);
  _m(1, 0) = row1._v(0);
  _m(1, 1) = row1._v(1);
  _m(1, 2) = row1._v(2);
  _m(1, 3) = row1._v(3);
  _m(2, 0) = row2._v(0);
  _m(2, 1) = row2._v(1);
  _m(2, 2) = row2._v(2);
  _m(2, 3) = row2._v(3);
  _m(3, 0) = row3._v(0);
  _m(3, 1) = row3._v(1);
  _m(3, 2) = row3._v(2);
  _m(3, 3) = row3._v(3);
#endif  // HAVE_EIGEN
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix4)::
FLOATNAME(LMatrix4)(const FLOATNAME(LMatrix3) &upper3) {
  TAU_PROFILE("void LMatrix4::LMatrix4(const LMatrix3 &)", " ", TAU_USER);

  _m(0, 0) = upper3._m(0, 0);
  _m(0, 1) = upper3._m(0, 1);
  _m(0, 2) = upper3._m(0, 2);
  _m(0, 3) = 0.0f;

  _m(1, 0) = upper3._m(1, 0);
  _m(1, 1) = upper3._m(1, 1);
  _m(1, 2) = upper3._m(1, 2);
  _m(1, 3) = 0.0f;

  _m(2, 0) = upper3._m(2, 0);
  _m(2, 1) = upper3._m(2, 1);
  _m(2, 2) = upper3._m(2, 2);
  _m(2, 3) = 0.0f;

  _m(3, 0) = 0.0f;
  _m(3, 1) = 0.0f;
  _m(3, 2) = 0.0f;
  _m(3, 3) = 1.0f;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix4)::
FLOATNAME(LMatrix4)(const FLOATNAME(LMatrix3) &upper3,
         const FLOATNAME(LVecBase3) &trans) {
  TAU_PROFILE("void LMatrix4::LMatrix4(upper3, const LVecBase3 &)", " ", TAU_USER);

  _m(0, 0) = upper3._m(0, 0);
  _m(0, 1) = upper3._m(0, 1);
  _m(0, 2) = upper3._m(0, 2);
  _m(0, 3) = 0.0f;

  _m(1, 0) = upper3._m(1, 0);
  _m(1, 1) = upper3._m(1, 1);
  _m(1, 2) = upper3._m(1, 2);
  _m(1, 3) = 0.0f;

  _m(2, 0) = upper3._m(2, 0);
  _m(2, 1) = upper3._m(2, 1);
  _m(2, 2) = upper3._m(2, 2);
  _m(2, 3) = 0.0f;

  _m(3, 0) = trans._v(0);
  _m(3, 1) = trans._v(1);
  _m(3, 2) = trans._v(2);
  _m(3, 3) = 1.0f;
}

/**
 * Sets each element of the matrix to the indicated fill_value.  This is of
 * questionable value, but is sometimes useful when initializing to zero.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
fill(FLOATTYPE fill_value) {
  TAU_PROFILE("void LMatrix4::fill(FLOATTYPE)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  _m = EMatrix4::Constant(fill_value);
#else
  set(fill_value, fill_value, fill_value, fill_value,
      fill_value, fill_value, fill_value, fill_value,
      fill_value, fill_value, fill_value, fill_value,
      fill_value, fill_value, fill_value, fill_value);
#endif  // HAVE_EIGEN
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02, FLOATTYPE e03,
    FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12, FLOATTYPE e13,
    FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22, FLOATTYPE e23,
    FLOATTYPE e30, FLOATTYPE e31, FLOATTYPE e32, FLOATTYPE e33) {
  TAU_PROFILE("void LMatrix4::set(FLOATTYPE, ...)", " ", TAU_USER);

  _m(0, 0) = e00;
  _m(0, 1) = e01;
  _m(0, 2) = e02;
  _m(0, 3) = e03;

  _m(1, 0) = e10;
  _m(1, 1) = e11;
  _m(1, 2) = e12;
  _m(1, 3) = e13;

  _m(2, 0) = e20;
  _m(2, 1) = e21;
  _m(2, 2) = e22;
  _m(2, 3) = e23;

  _m(3, 0) = e30;
  _m(3, 1) = e31;
  _m(3, 2) = e32;
  _m(3, 3) = e33;
}

/**
 * Sets the upper 3x3 submatrix.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_upper_3(const FLOATNAME(LMatrix3) &upper3) {
  TAU_PROFILE("void LMatrix4::set_upper_3(const LMatrix3 &)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  _m.block<3, 3>(0, 0) = upper3._m;
#else
  _m(0, 0) = upper3(0, 0);
  _m(0, 1) = upper3(0, 1);
  _m(0, 2) = upper3(0, 2);

  _m(1, 0) = upper3(1, 0);
  _m(1, 1) = upper3(1, 1);
  _m(1, 2) = upper3(1, 2);

  _m(2, 0) = upper3(2, 0);
  _m(2, 1) = upper3(2, 1);
  _m(2, 2) = upper3(2, 2);
#endif  // HAVE_EIGEN
}

/**
 * Retrieves the upper 3x3 submatrix.
 */
INLINE_LINMATH FLOATNAME(LMatrix3) FLOATNAME(LMatrix4)::
get_upper_3() const {
  TAU_PROFILE("LMatrix3 LMatrix4::get_upper_3()", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return FLOATNAME(LMatrix3)(_m.block<3, 3>(0, 0));
#else
  return FLOATNAME(LMatrix3)
    (_m(0, 0), _m(0, 1), _m(0, 2),
     _m(1, 0), _m(1, 1), _m(1, 2),
     _m(2, 0), _m(2, 1), _m(2, 2));
#endif  // HAVE_EIGEN
}

/**
 * Replaces the indicated row of the matrix.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_row(int row, const FLOATNAME(LVecBase4) &v) {
#ifdef HAVE_EIGEN
  _m.row(row) = v._v;
#else
  (*this)(row, 0) = v._v(0);
  (*this)(row, 1) = v._v(1);
  (*this)(row, 2) = v._v(2);
  (*this)(row, 3) = v._v(3);
#endif  // HAVE_EIGEN
}

/**
 * Replaces the indicated column of the matrix.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_col(int col, const FLOATNAME(LVecBase4) &v) {
#ifdef HAVE_EIGEN
  _m.col(col) = v._v;
#else
  (*this)(0, col) = v._v(0);
  (*this)(1, col) = v._v(1);
  (*this)(2, col) = v._v(2);
  (*this)(3, col) = v._v(3);
#endif  // HAVE_EIGEN
}

/**
 * Replaces the indicated row of the matrix with the indicated 3-component
 * vector, ignoring the last column.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_row(int row, const FLOATNAME(LVecBase3) &v) {
#ifdef HAVE_EIGEN
  _m.block<1, 3>(row, 0) = v._v;
#else
  (*this)(row, 0) = v._v(0);
  (*this)(row, 1) = v._v(1);
  (*this)(row, 2) = v._v(2);
#endif  // HAVE_EIGEN
}

/**
 * Replaces the indicated column of the matrix with the indicated 3-component
 * vector, ignoring the last row.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_col(int col, const FLOATNAME(LVecBase3) &v) {
#ifdef HAVE_EIGEN
  _m.block<3, 1>(0, col) = v._v;
#else
  (*this)(0, col) = v._v(0);
  (*this)(1, col) = v._v(1);
  (*this)(2, col) = v._v(2);
#endif
}

/**
 * Retrieves the indicated row of the matrix as a 4-component vector.
 */
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LMatrix4)::
get_row(int row) const {
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase4)(_m.row(row));
#else
  return FLOATNAME(LVecBase4)((*this)(row, 0),
                              (*this)(row, 1),
                              (*this)(row, 2),
                              (*this)(row, 3));
#endif  // HAVE_EIGEN
}

/**
 * Stores the indicated row of the matrix as a 4-component vector.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
get_row(FLOATNAME(LVecBase4) &result_vec, int row) const {
#ifdef HAVE_EIGEN
  result_vec._v = _m.row(row);
#else
  result_vec._v(0) = (*this)(row, 0);
  result_vec._v(1) = (*this)(row, 1);
  result_vec._v(2) = (*this)(row, 2);
  result_vec._v(3) = (*this)(row, 3);
#endif  // HAVE_EIGEN
}

/**
 * Retrieves the indicated column of the matrix as a 4-component vector.
 */
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LMatrix4)::
get_col(int col) const {
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase4)(_m.col(col));
#else
  return FLOATNAME(LVecBase4)((*this)(0, col),
                              (*this)(1, col),
                              (*this)(2, col),
                              (*this)(3, col));
#endif  // HAVE_EIGEN
}

/**
 * Retrieves the row column of the matrix as a 3-component vector, ignoring
 * the last column.
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix4)::
get_row3(int row) const {
  return FLOATNAME(LVecBase3)((*this)(row, 0),
                              (*this)(row, 1),
                              (*this)(row, 2));
}

/**
 * Stores the row column of the matrix as a 3-component vector, ignoring the
 * last column.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
get_row3(FLOATNAME(LVecBase3) &result_vec,int row) const {
#ifdef HAVE_EIGEN
  result_vec._v = _m.block<1, 3>(row, 0);
#else
  result_vec._v(0) = (*this)(row, 0);
  result_vec._v(1) = (*this)(row, 1);
  result_vec._v(2) = (*this)(row, 2);
#endif  // HAVE_EIGEN
}

/**
 * Retrieves the indicated column of the matrix as a 3-component vector,
 * ignoring the last row.
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix4)::
get_col3(int col) const {
  return FLOATNAME(LVecBase3)((*this)(0, col),
                              (*this)(1, col),
                              (*this)(2, col));
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE &FLOATNAME(LMatrix4)::
operator () (int row, int col) {
  nassertr(row >= 0 && row < 4 && col >= 0 && col < 4, _m(0, 0));
  return _m(row, col);
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix4)::
operator () (int row, int col) const {
  nassertr(row >= 0 && row < 4 && col >= 0 && col < 4, _m(0, 0));
  return _m(row, col);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix4)::CRow FLOATNAME(LMatrix4)::
operator [](int i) const {
  nassertr(i >= 0 && i < 4, CRow(&_m(0, 0)));
  return CRow(&_m(i, 0));
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix4)::Row FLOATNAME(LMatrix4)::
operator [](int i) {
  nassertr(i >= 0 && i < 4, Row(&_m(0, 0)));
  return Row(&_m(i, 0));
}

/**
 * Returns 4: the number of rows of a LMatrix4.
 */
INLINE_LINMATH int FLOATNAME(LMatrix4)::
size() {
  return 4;
}

/**
 * Returns true if any component of the matrix is not-a-number, false
 * otherwise.
 */
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
is_nan() const {
  TAU_PROFILE("bool LMatrix4::is_nan()", " ", TAU_USER);
  return
    cnan(_m(0, 0)) || cnan(_m(0, 1)) || cnan(_m(0, 2)) || cnan(_m(0, 3)) ||
    cnan(_m(1, 0)) || cnan(_m(1, 1)) || cnan(_m(1, 2)) || cnan(_m(1, 3)) ||
    cnan(_m(2, 0)) || cnan(_m(2, 1)) || cnan(_m(2, 2)) || cnan(_m(2, 3)) ||
    cnan(_m(3, 0)) || cnan(_m(3, 1)) || cnan(_m(3, 2)) || cnan(_m(3, 3));
}

/**
 * Returns true if this is (close enough to) the identity matrix, false
 * otherwise.
 */
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
is_identity() const {
  // Eigen has isIdentity, but it seems to be twice as slow as this.
  return almost_equal(ident_mat(), NEARLY_ZERO(FLOATTYPE));
}

/**
 * Returns a particular element of the matrix.
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(LMatrix4)::
get_cell(int row, int col) const {
  nassertr(row >= 0 && row < 4 && col >= 0 && col < 4, 0.0f);
  return _m(row, col);
}

/**
 * Changes a particular element of the matrix.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_cell(int row, int col, FLOATTYPE value) {
  nassertv(row >= 0 && row < 4 && col >= 0 && col < 4);
  _m(row, col) = value;
}

/**
 * Returns the address of the first of the nine data elements in the matrix.
 * The remaining elements occupy the next eight positions in row-major order.
 */
INLINE_LINMATH const FLOATTYPE *FLOATNAME(LMatrix4)::
get_data() const {
  return &_m(0, 0);
}

/**
 * Returns the number of elements in the matrix, 16.
 */
INLINE_LINMATH int FLOATNAME(LMatrix4)::
get_num_components() const {
  return 16;
}

/**
 * Returns an iterator that may be used to traverse the elements of the
 * matrix, STL-style.
 */
INLINE_LINMATH FLOATNAME(LMatrix4)::iterator FLOATNAME(LMatrix4)::
begin() {
  return &_m(0);
}

/**
 * Returns an iterator that may be used to traverse the elements of the
 * matrix, STL-style.
 */
INLINE_LINMATH FLOATNAME(LMatrix4)::iterator FLOATNAME(LMatrix4)::
end() {
  return begin() + 16;
}

/**
 * Returns an iterator that may be used to traverse the elements of the
 * matrix, STL-style.
 */
INLINE_LINMATH FLOATNAME(LMatrix4)::const_iterator FLOATNAME(LMatrix4)::
begin() const {
  return &_m(0);
}

/**
 * Returns an iterator that may be used to traverse the elements of the
 * matrix, STL-style.
 */
INLINE_LINMATH FLOATNAME(LMatrix4)::const_iterator FLOATNAME(LMatrix4)::
end() const {
  return begin() + 16;
}

/**
 * This performs a lexicographical comparison.  It's of questionable
 * mathematical meaning, but sometimes has a practical purpose for sorting
 * unique vectors, especially in an STL container.  Also see compare_to().
 */
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
operator < (const FLOATNAME(LMatrix4) &other) const {
  return compare_to(other) < 0;
}

/**
 *
 */
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
operator == (const FLOATNAME(LMatrix4) &other) const {
  return compare_to(other) == 0;
}

/**
 *
 */
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
operator != (const FLOATNAME(LMatrix4) &other) const {
  return compare_to(other) != 0;
}

/**
 * This flavor of compare_to uses a default threshold value based on the
 * numeric type.
 */
INLINE_LINMATH int FLOATNAME(LMatrix4)::
compare_to(const FLOATNAME(LMatrix4) &other) const {
  return compare_to(other, NEARLY_ZERO(FLOATTYPE));
}

/**
 * Returns a suitable hash for phash_map.
 */
INLINE_LINMATH size_t FLOATNAME(LMatrix4)::
get_hash() const {
  return add_hash(0);
}

/**
 * Returns a suitable hash for phash_map.
 */
INLINE_LINMATH size_t FLOATNAME(LMatrix4)::
get_hash(FLOATTYPE threshold) const {
  return add_hash(0, threshold);
}

/**
 * Adds the vector into the running hash.
 */
INLINE_LINMATH size_t FLOATNAME(LMatrix4)::
add_hash(size_t hash) const {
  return add_hash(hash, NEARLY_ZERO(FLOATTYPE));
}

/**
 * Adds the vector into the running hash.
 */
INLINE_LINMATH size_t FLOATNAME(LMatrix4)::
add_hash(size_t hash, FLOATTYPE threshold) const {
  TAU_PROFILE("size_t LMatrix4::add_hash(size_t, FLOATTYPE)", " ", TAU_USER);
  float_hash fhasher(threshold);

  hash = fhasher.add_hash(hash, _m(0, 0));
  hash = fhasher.add_hash(hash, _m(0, 1));
  hash = fhasher.add_hash(hash, _m(0, 2));
  hash = fhasher.add_hash(hash, _m(0, 3));

  hash = fhasher.add_hash(hash, _m(1, 0));
  hash = fhasher.add_hash(hash, _m(1, 1));
  hash = fhasher.add_hash(hash, _m(1, 2));
  hash = fhasher.add_hash(hash, _m(1, 3));

  hash = fhasher.add_hash(hash, _m(2, 0));
  hash = fhasher.add_hash(hash, _m(2, 1));
  hash = fhasher.add_hash(hash, _m(2, 2));
  hash = fhasher.add_hash(hash, _m(2, 3));

  hash = fhasher.add_hash(hash, _m(3, 0));
  hash = fhasher.add_hash(hash, _m(3, 1));
  hash = fhasher.add_hash(hash, _m(3, 2));
  hash = fhasher.add_hash(hash, _m(3, 3));

  return hash;
}

#define VECTOR4_MATRIX4_PRODUCT(v_res, v, mat)       \
v_res._v(0) = v._v(0)*mat._m(0, 0) + v._v(1)*mat._m(1, 0) + v._v(2)*mat._m(2, 0) + v._v(3)*mat._m(3, 0);   \
v_res._v(1) = v._v(0)*mat._m(0, 1) + v._v(1)*mat._m(1, 1) + v._v(2)*mat._m(2, 1) + v._v(3)*mat._m(3, 1);   \
v_res._v(2) = v._v(0)*mat._m(0, 2) + v._v(1)*mat._m(1, 2) + v._v(2)*mat._m(2, 2) + v._v(3)*mat._m(3, 2);   \
v_res._v(3) = v._v(0)*mat._m(0, 3) + v._v(1)*mat._m(1, 3) + v._v(2)*mat._m(2, 3) + v._v(3)*mat._m(3, 3);

/**
 * 4-component vector or point times matrix.  This is a fully general
 * operation.
 */
INLINE_LINMATH FLOATNAME(LVecBase4) FLOATNAME(LMatrix4)::
xform(const FLOATNAME(LVecBase4) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix4::xform(const LVecBase3 &)", " ", TAU_USER);
  FLOATNAME(LVecBase4) v_res;

#ifdef HAVE_EIGEN
  v_res._v.noalias() = v._v * _m;
#else
  VECTOR4_MATRIX4_PRODUCT(v_res, v,(*this));
#endif  // HAVE_EIGEN
  return v_res;
}

#undef VECTOR4_MATRIX4_PRODUCT

/**
 * The matrix transforms a 3-component point (including translation component)
 * and returns the result.  This assumes the matrix is an affine transform.
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix4)::
xform_point(const FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix4::xform_point(const LVecBase3 &)", " ", TAU_USER);
  FLOATNAME(LVecBase3) v_res;

  // v._v(3) == 1.0f for this case

#ifdef HAVE_EIGEN
  v_res._v.noalias() = v._v * _m.block<3, 3>(0, 0) + _m.block<1, 3>(3, 0);
#else
  v_res._v(0) = v._v(0)*_m(0, 0) + v._v(1)*_m(1, 0) + v._v(2)*_m(2, 0) + _m(3, 0);
  v_res._v(1) = v._v(0)*_m(0, 1) + v._v(1)*_m(1, 1) + v._v(2)*_m(2, 1) + _m(3, 1);
  v_res._v(2) = v._v(0)*_m(0, 2) + v._v(1)*_m(1, 2) + v._v(2)*_m(2, 2) + _m(3, 2);
#endif  // HAVE_EIGEN

  return v_res;
}

/**
 * The matrix transforms a 3-component point (including translation component)
 * and returns the result, as a fully general operation.
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix4)::
xform_point_general(const FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix4::xform_point_general(const LVecBase3 &)", " ", TAU_USER);
  FLOATNAME(LVecBase4) v4(v[0], v[1], v[2], 1.0);
  v4 = xform(v4);
  return FLOATNAME(LVecBase3)(v4[0] / v4[3], v4[1] / v4[3], v4[2] / v4[3]);
}

/**
 * The matrix transforms a 3-component vector (without translation component)
 * and returns the result.  This assumes the matrix is an orthonormal
 * transform.
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix4)::
xform_vec(const FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix4::xform_vec(const LVecBase3 &)", " ", TAU_USER);
  FLOATNAME(LVecBase3) v_res;

  // v._v(3) == 0.0f for this case

#ifdef HAVE_EIGEN
  v_res._v.noalias() = v._v * _m.block<3, 3>(0, 0);
#else
  v_res._v(0) = v._v(0)*_m(0, 0) + v._v(1)*_m(1, 0) + v._v(2)*_m(2, 0);
  v_res._v(1) = v._v(0)*_m(0, 1) + v._v(1)*_m(1, 1) + v._v(2)*_m(2, 1);
  v_res._v(2) = v._v(0)*_m(0, 2) + v._v(1)*_m(1, 2) + v._v(2)*_m(2, 2);
#endif  // HAVE_EIGEN

  return v_res;
}

/**
 * The matrix transforms a 3-component vector (without translation component)
 * and returns the result, as a fully general operation.
 */
INLINE_LINMATH FLOATNAME(LVecBase3) FLOATNAME(LMatrix4)::
xform_vec_general(const FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("LVecBase3 LMatrix4::xform_vec_general(const LVecBase3 &)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return FLOATNAME(LVecBase3)(v._v * _m.block<3, 3>(0, 0).inverse().transpose());
#else
  FLOATNAME(LMatrix3) i;
  i.invert_transpose_from(*this);

  return i.xform(v);
#endif  // HAVE_EIGEN
}

/**
 * 4-component vector or point times matrix.  This is a fully general
 * operation.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
xform_in_place(FLOATNAME(LVecBase4) &v) const {
  TAU_PROFILE("void LMatrix4::xform_in_place(LVecBase3 &)", " ", TAU_USER);

#ifdef HAVE_EIGEN
  v._v = v._v * _m;
#else
  v = xform(v);
#endif  // HAVE_EIGEN
}

/**
 * The matrix transforms a 3-component point (including translation
 * component).  This assumes the matrix is an affine transform.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
xform_point_in_place(FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("void LMatrix4::xform_point_in_place(LVecBase3 &)", " ", TAU_USER);
  // v._v(3) == 1.0f for this case

#ifdef HAVE_EIGEN
  v._v = v._v * _m.block<3, 3>(0, 0) + _m.block<1, 3>(3, 0);
#else
  v = xform_point(v);
#endif  // HAVE_EIGEN
}

/**
 * The matrix transforms a 3-component point (including translation
 * component), as a fully general operation.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
xform_point_general_in_place(FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("void LMatrix4::xform_point_general_in_place(LVecBase3 &)", " ", TAU_USER);
  v = xform_point_general(v);
}

/**
 * The matrix transforms a 3-component vector (without translation component).
 * This assumes the matrix is an orthonormal transform.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
xform_vec_in_place(FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("void LMatrix4::xform_vec_in_place(LVecBase3 &)", " ", TAU_USER);
  // v._v(3) == 0.0f for this case

#ifdef HAVE_EIGEN
  v._v = v._v * _m.block<3, 3>(0, 0);
#else
  v = xform_vec(v);
#endif  // HAVE_EIGEN
}

/**
 * The matrix transforms a 3-component vector (without translation component),
 * as a fully general operation.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
xform_vec_general_in_place(FLOATNAME(LVecBase3) &v) const {
  TAU_PROFILE("void LMatrix4::xform_vec_general_in_place(LVecBase3 &)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  v._v = v._v * _m.block<3, 3>(0, 0).inverse().transpose();
#else
  v = xform_vec_general(v);
#endif  // HAVE_EIGEN
}

#define MATRIX4_PRODUCT(res, a, b)                                          \
res._m(0, 0) = a._m(0, 0)*b._m(0, 0) + a._m(0, 1)*b._m(1, 0) + a._m(0, 2)*b._m(2, 0) + a._m(0, 3)*b._m(3, 0);   \
res._m(0, 1) = a._m(0, 0)*b._m(0, 1) + a._m(0, 1)*b._m(1, 1) + a._m(0, 2)*b._m(2, 1) + a._m(0, 3)*b._m(3, 1);   \
res._m(0, 2) = a._m(0, 0)*b._m(0, 2) + a._m(0, 1)*b._m(1, 2) + a._m(0, 2)*b._m(2, 2) + a._m(0, 3)*b._m(3, 2);   \
res._m(0, 3) = a._m(0, 0)*b._m(0, 3) + a._m(0, 1)*b._m(1, 3) + a._m(0, 2)*b._m(2, 3) + a._m(0, 3)*b._m(3, 3);   \
                                                                   \
res._m(1, 0) = a._m(1, 0)*b._m(0, 0) + a._m(1, 1)*b._m(1, 0) + a._m(1, 2)*b._m(2, 0) + a._m(1, 3)*b._m(3, 0);   \
res._m(1, 1) = a._m(1, 0)*b._m(0, 1) + a._m(1, 1)*b._m(1, 1) + a._m(1, 2)*b._m(2, 1) + a._m(1, 3)*b._m(3, 1);   \
res._m(1, 2) = a._m(1, 0)*b._m(0, 2) + a._m(1, 1)*b._m(1, 2) + a._m(1, 2)*b._m(2, 2) + a._m(1, 3)*b._m(3, 2);   \
res._m(1, 3) = a._m(1, 0)*b._m(0, 3) + a._m(1, 1)*b._m(1, 3) + a._m(1, 2)*b._m(2, 3) + a._m(1, 3)*b._m(3, 3);   \
                                                                   \
res._m(2, 0) = a._m(2, 0)*b._m(0, 0) + a._m(2, 1)*b._m(1, 0) + a._m(2, 2)*b._m(2, 0) + a._m(2, 3)*b._m(3, 0);   \
res._m(2, 1) = a._m(2, 0)*b._m(0, 1) + a._m(2, 1)*b._m(1, 1) + a._m(2, 2)*b._m(2, 1) + a._m(2, 3)*b._m(3, 1);   \
res._m(2, 2) = a._m(2, 0)*b._m(0, 2) + a._m(2, 1)*b._m(1, 2) + a._m(2, 2)*b._m(2, 2) + a._m(2, 3)*b._m(3, 2);   \
res._m(2, 3) = a._m(2, 0)*b._m(0, 3) + a._m(2, 1)*b._m(1, 3) + a._m(2, 2)*b._m(2, 3) + a._m(2, 3)*b._m(3, 3);   \
                                                                   \
res._m(3, 0) = a._m(3, 0)*b._m(0, 0) + a._m(3, 1)*b._m(1, 0) + a._m(3, 2)*b._m(2, 0) + a._m(3, 3)*b._m(3, 0);   \
res._m(3, 1) = a._m(3, 0)*b._m(0, 1) + a._m(3, 1)*b._m(1, 1) + a._m(3, 2)*b._m(2, 1) + a._m(3, 3)*b._m(3, 1);   \
res._m(3, 2) = a._m(3, 0)*b._m(0, 2) + a._m(3, 1)*b._m(1, 2) + a._m(3, 2)*b._m(2, 2) + a._m(3, 3)*b._m(3, 2);   \
res._m(3, 3) = a._m(3, 0)*b._m(0, 3) + a._m(3, 1)*b._m(1, 3) + a._m(3, 2)*b._m(2, 3) + a._m(3, 3)*b._m(3, 3);

/**
 *
 */
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
  // faster than operator * since it writes result in place, avoiding extra
  // copying this will fail if you try to mat.multiply(mat,other_mat)

  nassertv((&other1 != this) && (&other2 != this));

#ifdef HAVE_EIGEN
  _m.noalias() = other1._m * other2._m;

#else
  MATRIX4_PRODUCT((*this),other1,other2);
#endif  // HAVE_EIGEN
}

#undef MATRIX4_PRODUCT

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
operator * (FLOATTYPE scalar) const {
  TAU_PROFILE("LMatrix4 operator *(const LMatrix4 &, FLOATTYPE)", " ", TAU_USER);
  FLOATNAME(LMatrix4) t;

#ifdef HAVE_EIGEN
  t._m = _m * scalar;

#else
  t._m(0, 0) = _m(0, 0) * scalar;
  t._m(0, 1) = _m(0, 1) * scalar;
  t._m(0, 2) = _m(0, 2) * scalar;
  t._m(0, 3) = _m(0, 3) * scalar;

  t._m(1, 0) = _m(1, 0) * scalar;
  t._m(1, 1) = _m(1, 1) * scalar;
  t._m(1, 2) = _m(1, 2) * scalar;
  t._m(1, 3) = _m(1, 3) * scalar;

  t._m(2, 0) = _m(2, 0) * scalar;
  t._m(2, 1) = _m(2, 1) * scalar;
  t._m(2, 2) = _m(2, 2) * scalar;
  t._m(2, 3) = _m(2, 3) * scalar;

  t._m(3, 0) = _m(3, 0) * scalar;
  t._m(3, 1) = _m(3, 1) * scalar;
  t._m(3, 2) = _m(3, 2) * scalar;
  t._m(3, 3) = _m(3, 3) * scalar;
#endif  // HAVE_EIGEN

  return t;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
operator / (FLOATTYPE scalar) const {
  FLOATTYPE recip_scalar = 1.0f/scalar;
  return (*this) * recip_scalar;
}

/**
 * Performs a memberwise addition between two matrices.
 */
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator += (const FLOATNAME(LMatrix4) &other) {
  TAU_PROFILE("LMatrix4 LMatrix4::operator +=(const LMatrix4 &)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  _m += other._m;

#else
  _m(0, 0) += other._m(0, 0);
  _m(0, 1) += other._m(0, 1);
  _m(0, 2) += other._m(0, 2);
  _m(0, 3) += other._m(0, 3);

  _m(1, 0) += other._m(1, 0);
  _m(1, 1) += other._m(1, 1);
  _m(1, 2) += other._m(1, 2);
  _m(1, 3) += other._m(1, 3);

  _m(2, 0) += other._m(2, 0);
  _m(2, 1) += other._m(2, 1);
  _m(2, 2) += other._m(2, 2);
  _m(2, 3) += other._m(2, 3);

  _m(3, 0) += other._m(3, 0);
  _m(3, 1) += other._m(3, 1);
  _m(3, 2) += other._m(3, 2);
  _m(3, 3) += other._m(3, 3);
#endif  // HAVE_EIGEN

  return *this;
}

/**
 * Performs a memberwise addition between two matrices.
 */
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator -= (const FLOATNAME(LMatrix4) &other) {
  TAU_PROFILE("LMatrix4 LMatrix4::operator -=(const LMatrix4 &)", " ", TAU_USER);

#ifdef HAVE_EIGEN
  _m -= other._m;

#else
  _m(0, 0) -= other._m(0, 0);
  _m(0, 1) -= other._m(0, 1);
  _m(0, 2) -= other._m(0, 2);
  _m(0, 3) -= other._m(0, 3);

  _m(1, 0) -= other._m(1, 0);
  _m(1, 1) -= other._m(1, 1);
  _m(1, 2) -= other._m(1, 2);
  _m(1, 3) -= other._m(1, 3);

  _m(2, 0) -= other._m(2, 0);
  _m(2, 1) -= other._m(2, 1);
  _m(2, 2) -= other._m(2, 2);
  _m(2, 3) -= other._m(2, 3);

  _m(3, 0) -= other._m(3, 0);
  _m(3, 1) -= other._m(3, 1);
  _m(3, 2) -= other._m(3, 2);
  _m(3, 3) -= other._m(3, 3);
#endif  // HAVE_EIGEN

  return *this;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator *= (const FLOATNAME(LMatrix4) &other) {
  TAU_PROFILE("LMatrix4 LMatrix4::operator *=(const LMatrix4 &)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  _m *= other._m;

#else
  FLOATNAME(LMatrix4) temp = *this;
  multiply(temp, other);
#endif  // HAVE_EIGEN

  return *this;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator *= (FLOATTYPE scalar) {
  TAU_PROFILE("LMatrix4 LMatrix4::operator *=(FLOATTYPE)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  _m *= scalar;

#else
  _m(0, 0) *= scalar;
  _m(0, 1) *= scalar;
  _m(0, 2) *= scalar;
  _m(0, 3) *= scalar;

  _m(1, 0) *= scalar;
  _m(1, 1) *= scalar;
  _m(1, 2) *= scalar;
  _m(1, 3) *= scalar;

  _m(2, 0) *= scalar;
  _m(2, 1) *= scalar;
  _m(2, 2) *= scalar;
  _m(2, 3) *= scalar;

  _m(3, 0) *= scalar;
  _m(3, 1) *= scalar;
  _m(3, 2) *= scalar;
  _m(3, 3) *= scalar;
#endif // HAVE_EIGEN

  return *this;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
operator /= (FLOATTYPE scalar) {
  TAU_PROFILE("LMatrix4 LMatrix4::operator /=(FLOATTYPE)", " ", TAU_USER);
  FLOATTYPE recip_scalar = 1.0f/scalar;
  return operator *= (recip_scalar);
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
componentwise_mult(const FLOATNAME(LMatrix4) &other) {
#ifdef HAVE_EIGEN
  _m = _m.cwiseProduct(other._m);
#else
  _m(0, 0) *= other._m(0, 0);
  _m(0, 1) *= other._m(0, 1);
  _m(0, 2) *= other._m(0, 2);
  _m(0, 3) *= other._m(0, 3);

  _m(1, 0) *= other._m(1, 0);
  _m(1, 1) *= other._m(1, 1);
  _m(1, 2) *= other._m(1, 2);
  _m(1, 3) *= other._m(1, 3);

  _m(2, 0) *= other._m(2, 0);
  _m(2, 1) *= other._m(2, 1);
  _m(2, 2) *= other._m(2, 2);
  _m(2, 3) *= other._m(2, 3);

  _m(3, 0) *= other._m(3, 0);
  _m(3, 1) *= other._m(3, 1);
  _m(3, 2) *= other._m(3, 2);
  _m(3, 3) *= other._m(3, 3);
#endif  // HAVE_EIGEN
}


/**
 *
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
transpose_from(const FLOATNAME(LMatrix4) &other) {
  TAU_PROFILE("LMatrix4 LMatrix4::transpose_from(const LMatrix4 &other)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  _m = other._m.transpose();

#else
  _m(0, 0) = other._m(0, 0);
  _m(0, 1) = other._m(1, 0);
  _m(0, 2) = other._m(2, 0);
  _m(0, 3) = other._m(3, 0);

  _m(1, 0) = other._m(0, 1);
  _m(1, 1) = other._m(1, 1);
  _m(1, 2) = other._m(2, 1);
  _m(1, 3) = other._m(3, 1);

  _m(2, 0) = other._m(0, 2);
  _m(2, 1) = other._m(1, 2);
  _m(2, 2) = other._m(2, 2);
  _m(2, 3) = other._m(3, 2);

  _m(3, 0) = other._m(0, 3);
  _m(3, 1) = other._m(1, 3);
  _m(3, 2) = other._m(2, 3);
  _m(3, 3) = other._m(3, 3);
#endif  // HAVE_EIGEN
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
transpose_in_place() {
  TAU_PROFILE("void LMatrix4::transpose_in_place()", " ", TAU_USER);

#ifdef HAVE_EIGEN
  _m.transposeInPlace();

#else
  std::swap(_m(0, 1), _m(1, 0));
  std::swap(_m(0, 2), _m(2, 0));
  std::swap(_m(0, 3), _m(3, 0));
  std::swap(_m(1, 2), _m(2, 1));
  std::swap(_m(1, 3), _m(3, 1));
  std::swap(_m(2, 3), _m(3, 2));
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
 * the was a singularity.
 */
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
invert_from(const FLOATNAME(LMatrix4) &other) {
  TAU_PROFILE("bool LMatrix4::invert_from(const LMatrix4 &)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  // We use the squared nearly_zero value as determinant threshold for
  // checking whether a matrix is singular, since that's the same constant we
  // use in the non-Eigen case (see lmatrix3_src.I) and also because we
  // otherwise run into issues very quickly.
  bool invertible;
  other._m.computeInverseWithCheck(_m, invertible,
    NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE));

  if (!invertible) {
#ifdef NOTIFY_DEBUG
    linmath_cat.warning() << "Tried to invert singular LMatrix4.\n";
#endif

    (*this) = ident_mat();
    nassertr(!no_singular_invert, false);
  }

  return invertible;

#else  // HAVE_EIGEN
  if (IS_NEARLY_EQUAL(other._m(0, 3), 0.0f) &&
      IS_NEARLY_EQUAL(other._m(1, 3), 0.0f) &&
      IS_NEARLY_EQUAL(other._m(2, 3), 0.0f) &&
      IS_NEARLY_EQUAL(other._m(3, 3), 1.0f)) {
    return invert_affine_from(other);
  }

  (*this) = other;

  int index[4];

  if (!decompose_mat(index)) {
#ifdef NOTIFY_DEBUG
    linmath_cat.warning() << "Tried to invert singular LMatrix4.\n";
#endif

    (*this) = ident_mat();
    nassertr(!no_singular_invert, false);
    return false;
  }

  FLOATNAME(LMatrix4) inv = FLOATNAME(LMatrix4)::ident_mat();
  int row;

  for (row = 0; row < 4; row++) {
    back_sub_mat(index, inv, row);
  }

  transpose_from(inv);
  return true;
#endif  // HAVE_EIGEN
}

/**
 * Performs an invert of the indicated matrix, storing the result in this
 * matrix.  The calculation is only correct of the other matrix represents an
 * affine transform.
 *
 * The other matrix must be a different object than this matrix.  However, if
 * you need to invert a matrix in place, see invert_in_place.
 *
 * The return value is true if the matrix was successfully inverted, false if
 * there was a singularity.
 */

// bugbug: we could optimize this for rotationscaletranslation matrices
// (transpose upper 3x3 and take negative of translation component)
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
invert_affine_from(const FLOATNAME(LMatrix4) &other) {
  TAU_PROFILE("bool LMatrix4::invert_affine_from(const LMatrix4 &)", " ", TAU_USER);
  FLOATNAME(LMatrix3) rot;

  // probably could use transpose here
  if (!rot.invert_from(other.get_upper_3())) {
    return false;
  }

  set_upper_3(rot);

  _m(0, 3) = 0.0f;
  _m(1, 3) = 0.0f;
  _m(2, 3) = 0.0f;
  _m(3, 3) = 1.0f;

  _m(3, 0) = -(other._m(3, 0) * _m(0, 0) +
               other._m(3, 1) * _m(1, 0) +
               other._m(3, 2) * _m(2, 0));

  _m(3, 1) = -(other._m(3, 0) * _m(0, 1) +
               other._m(3, 1) * _m(1, 1) +
               other._m(3, 2) * _m(2, 1));

  _m(3, 2) = -(other._m(3, 0) * _m(0, 2) +
               other._m(3, 1) * _m(1, 2) +
               other._m(3, 2) * _m(2, 2));

  return true;
}

/**
 * Inverts the current matrix.  Returns true if the inverse is successful,
 * false if the matrix was singular.
 */
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
invert_in_place() {
  TAU_PROFILE("bool LMatrix4::invert_in_place()", " ", TAU_USER);
  FLOATNAME(LMatrix4) temp = (*this);
  return invert_from(temp);
}

/**
 * Computes (*this) += other * weight.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
accumulate(const FLOATNAME(LMatrix4) &other, FLOATTYPE weight) {
#ifdef HAVE_EIGEN
  _m += other._m * weight;
#else
  _m(0, 0) += other._m(0, 0) * weight;
  _m(0, 1) += other._m(0, 1) * weight;
  _m(0, 2) += other._m(0, 2) * weight;
  _m(0, 3) += other._m(0, 3) * weight;

  _m(1, 0) += other._m(1, 0) * weight;
  _m(1, 1) += other._m(1, 1) * weight;
  _m(1, 2) += other._m(1, 2) * weight;
  _m(1, 3) += other._m(1, 3) * weight;

  _m(2, 0) += other._m(2, 0) * weight;
  _m(2, 1) += other._m(2, 1) * weight;
  _m(2, 2) += other._m(2, 2) * weight;
  _m(2, 3) += other._m(2, 3) * weight;

  _m(3, 0) += other._m(3, 0) * weight;
  _m(3, 1) += other._m(3, 1) * weight;
  _m(3, 2) += other._m(3, 2) * weight;
  _m(3, 3) += other._m(3, 3) * weight;
#endif  // HAVE_EIGEN
}

/**
 * Fills mat with a matrix that applies the indicated translation.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_translate_mat(const FLOATNAME(LVecBase3) &trans) {
  set(1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      trans._v(0), trans._v(1), trans._v(2), 1.0f);
}

/**
 * Fills mat with a matrix that applies the indicated scale in each of the
 * three axes.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_scale_mat(const FLOATNAME(LVecBase3) &scale) {
  set(scale._v(0), 0.0f, 0.0f, 0.0f,
      0.0f, scale._v(1), 0.0f, 0.0f,
      0.0f, 0.0f, scale._v(2), 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f);
}

/**
 * Fills mat with a matrix that applies the indicated shear in each of the
 * three planes.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_shear_mat(const FLOATNAME(LVecBase3) &shear, CoordinateSystem cs) {
  set_scale_shear_mat(FLOATNAME(LVecBase3)(1.0f, 1.0f, 1.0f),
                      shear, cs);
}

/**
 * Fills mat with a matrix that applies the indicated scale and shear.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
set_scale_shear_mat(const FLOATNAME(LVecBase3) &scale,
                    const FLOATNAME(LVecBase3) &shear,
                    CoordinateSystem cs) {
  FLOATNAME(LMatrix3) m3;
  m3.set_scale_shear_mat(scale, shear, cs);
  set_upper_3(m3);

  _m(0, 3) = 0.0f;
  _m(1, 3) = 0.0f;
  _m(2, 3) = 0.0f;
  _m(3, 3) = 1.0f;

  _m(3, 0) = 0.0f;
  _m(3, 1) = 0.0f;
  _m(3, 2) = 0.0f;
}

/**
 * Returns a matrix that applies the indicated translation.
 */
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
translate_mat(const FLOATNAME(LVecBase3) &trans) {
  return FLOATNAME(LMatrix4)(1.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, 1.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 1.0f, 0.0f,
                             trans._v(0), trans._v(1), trans._v(2), 1.0f);
}

/**
 * Returns a matrix that applies the indicated translation.
 */
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
translate_mat(FLOATTYPE tx, FLOATTYPE ty, FLOATTYPE tz) {
  return FLOATNAME(LMatrix4)(1.0f, 0.0f, 0.0f, 0.0f,
                           0.0f, 1.0f, 0.0f, 0.0f,
                           0.0f, 0.0f, 1.0f, 0.0f,
                           tx, ty, tz, 1.0f);
}

/**
 * Returns a matrix that rotates by the given angle in degrees
 * counterclockwise about the indicated vector.
 */
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
rotate_mat(FLOATTYPE angle, const FLOATNAME(LVecBase3) &axis,
           CoordinateSystem cs) {
  FLOATNAME(LMatrix4) mat;
  mat.set_rotate_mat(angle, axis, cs);
  return mat;
}

/**
 * Returns a matrix that rotates by the given angle in degrees
 * counterclockwise about the indicated vector.  Assumes axis has been
 * prenormalized.
 */
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
rotate_mat_normaxis(FLOATTYPE angle, const FLOATNAME(LVecBase3) &axis,
                    CoordinateSystem cs) {
  TAU_PROFILE("LMatrix4 LMatrix4::rotate_mat_normaxis(FLOATTYPE, const LVecBase3 &, cs)", " ", TAU_USER);

  FLOATNAME(LMatrix4) mat;
  mat.set_rotate_mat_normaxis(angle, axis, cs);
  return mat;
}

/**
 * Returns a matrix that applies the indicated scale in each of the three
 * axes.
 */
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
scale_mat(const FLOATNAME(LVecBase3) &scale) {
  return FLOATNAME(LMatrix4)(scale._v(0), 0.0f, 0.0f, 0.0f,
                             0.0f, scale._v(1), 0.0f, 0.0f,
                             0.0f, 0.0f, scale._v(2), 0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f);
}


/**
 * Returns a matrix that applies the indicated scale in each of the three
 * axes.
 */
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
scale_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz) {
  return FLOATNAME(LMatrix4)(sx, 0.0f, 0.0f, 0.0f,
                             0.0f, sy, 0.0f, 0.0f,
                             0.0f, 0.0f, sz, 0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f);
}

/**
 * Returns a matrix that applies the indicated uniform scale.
 */
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
scale_mat(FLOATTYPE scale) {
  return FLOATNAME(LMatrix4)(scale, 0.0f, 0.0f, 0.0f,
                             0.0f, scale, 0.0f, 0.0f,
                             0.0f, 0.0f, scale, 0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f);
}

/**
 * Returns a matrix that applies the indicated shear in each of the three
 * planes.
 */
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
shear_mat(const FLOATNAME(LVecBase3) &shear, CoordinateSystem cs) {
  FLOATNAME(LMatrix4) mat;
  mat.set_shear_mat(shear, cs);
  return mat;
}

/**
 * Returns a matrix that applies the indicated shear in each of the three
 * planes.
 */
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
shear_mat(FLOATTYPE shxy, FLOATTYPE shxz, FLOATTYPE shyz,
          CoordinateSystem cs) {
  FLOATNAME(LMatrix4) mat;
  mat.set_shear_mat(FLOATNAME(LVecBase3)(shxy, shxz, shyz), cs);
  return mat;
}

/**
 * Returns a matrix that applies the indicated scale and shear.
 */
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
scale_shear_mat(const FLOATNAME(LVecBase3) &scale,
                const FLOATNAME(LVecBase3) &shear,
                CoordinateSystem cs) {
  FLOATNAME(LMatrix4) mat;
  mat.set_scale_shear_mat(scale, shear, cs);
  return mat;
}

/**
 * Returns a matrix that applies the indicated scale and shear.
 */
INLINE_LINMATH FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
scale_shear_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz,
                FLOATTYPE shxy, FLOATTYPE shxz, FLOATTYPE shyz,
                CoordinateSystem cs) {
  FLOATNAME(LMatrix4) mat;
  mat.set_scale_shear_mat(FLOATNAME(LVecBase3)(sx, sy, sz),
                          FLOATNAME(LVecBase3)(shxy, shxz, shyz), cs);
  return mat;
}

/**
 * Returns a matrix that transforms from the Y-up coordinate system to the
 * Z-up coordinate system.
 */
INLINE_LINMATH const FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
y_to_z_up_mat() {
  return _y_to_z_up_mat;
}

/**
 * Returns a matrix that transforms from the Y-up coordinate system to the
 * Z-up coordinate system.
 */
INLINE_LINMATH const FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
z_to_y_up_mat() {
  return _z_to_y_up_mat;
}

/**
 * Returns true if two matrices are memberwise equal within a default
 * tolerance based on the numeric type.
 */
INLINE_LINMATH bool FLOATNAME(LMatrix4)::
almost_equal(const FLOATNAME(LMatrix4) &other) const {
  return almost_equal(other, NEARLY_ZERO(FLOATTYPE));
}

/**
 * Adds the vector to the indicated hash generator.
 */
INLINE_LINMATH void FLOATNAME(LMatrix4)::
generate_hash(ChecksumHashGenerator &hashgen) const {
  generate_hash(hashgen, NEARLY_ZERO(FLOATTYPE));
}

/**
 * Transposes the given matrix and returns it.
 */
INLINE_LINMATH FLOATNAME(LMatrix4)
transpose(const FLOATNAME(LMatrix4) &a) {
  FLOATNAME(LMatrix4) result;
  result.transpose_from(a);
  return result;
}

/**
 * Inverts the given matrix and returns it.
 */
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

/**
 *
 */
INLINE_LINMATH FLOATNAME(UnalignedLMatrix4)::
FLOATNAME(UnalignedLMatrix4)(const FLOATNAME(LMatrix4) &copy) {
  operator = (copy);
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(UnalignedLMatrix4) &FLOATNAME(UnalignedLMatrix4)::
operator = (const FLOATNAME(LMatrix4) &copy) {
  memcpy(&_m(0, 0), copy.get_data(), sizeof(FLOATTYPE) * num_components);
  return *this;
}

/**
 *
 */
INLINE_LINMATH FLOATNAME(UnalignedLMatrix4)::
FLOATNAME(UnalignedLMatrix4)(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02, FLOATTYPE e03,
                             FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12, FLOATTYPE e13,
                             FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22, FLOATTYPE e23,
                             FLOATTYPE e30, FLOATTYPE e31, FLOATTYPE e32, FLOATTYPE e33) {
  TAU_PROFILE("UnalignedLMatrix4::UnalignedLMatrix4(FLOATTYPE, ...)", " ", TAU_USER);
  set(e00, e01, e02, e03,
      e10, e11, e12, e13,
      e20, e21, e22, e23,
      e30, e31, e32, e33);
}

/**
 *
 */
INLINE_LINMATH void FLOATNAME(UnalignedLMatrix4)::
set(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02, FLOATTYPE e03,
    FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12, FLOATTYPE e13,
    FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22, FLOATTYPE e23,
    FLOATTYPE e30, FLOATTYPE e31, FLOATTYPE e32, FLOATTYPE e33) {
  TAU_PROFILE("void UnalignedLMatrix4::set()", " ", TAU_USER);

  _m(0, 0) = e00;
  _m(0, 1) = e01;
  _m(0, 2) = e02;
  _m(0, 3) = e03;

  _m(1, 0) = e10;
  _m(1, 1) = e11;
  _m(1, 2) = e12;
  _m(1, 3) = e13;

  _m(2, 0) = e20;
  _m(2, 1) = e21;
  _m(2, 2) = e22;
  _m(2, 3) = e23;

  _m(3, 0) = e30;
  _m(3, 1) = e31;
  _m(3, 2) = e32;
  _m(3, 3) = e33;
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE &FLOATNAME(UnalignedLMatrix4)::
operator () (int row, int col) {
  nassertr(row >= 0 && row < 4 && col >= 0 && col < 4, _m(0, 0));
  return _m(row, col);
}

/**
 *
 */
INLINE_LINMATH FLOATTYPE FLOATNAME(UnalignedLMatrix4)::
operator () (int row, int col) const {
  nassertr(row >= 0 && row < 4 && col >= 0 && col < 4, 0.0);
  return _m(row, col);
}

/**
 * Returns the address of the first of the nine data elements in the matrix.
 * The remaining elements occupy the next eight positions in row-major order.
 */
INLINE_LINMATH const FLOATTYPE *FLOATNAME(UnalignedLMatrix4)::
get_data() const {
  return &_m(0, 0);
}

/**
 * Returns the number of elements in the matrix, sixteen.
 */
INLINE_LINMATH int FLOATNAME(UnalignedLMatrix4)::
get_num_components() const {
  return 16;
}

/**
 *
 */
INLINE_LINMATH bool FLOATNAME(UnalignedLMatrix4)::
operator == (const FLOATNAME(UnalignedLMatrix4) &other) const {
  return memcmp(get_data(), other.get_data(), sizeof(FLOATTYPE) * 16) == 0;
}

/**
 *
 */
INLINE_LINMATH bool FLOATNAME(UnalignedLMatrix4)::
operator != (const FLOATNAME(UnalignedLMatrix4) &other) const {
  return !operator == (other);
}
