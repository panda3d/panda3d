// Filename: lmatrix3_src.I
// Created by:  drose (29Jan99)
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
INLINE const FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
ident_mat() {
  return _ident_mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Default Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3):: 
FLOATNAME(LMatrix3)() {
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3):: 
FLOATNAME(LMatrix3)(const FLOATNAME(LMatrix3) &copy) {
  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3):: 
operator = (const FLOATNAME(LMatrix3) &copy) {
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
INLINE FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator = (FLOATTYPE fill_value) {
  fill(fill_value);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3):: 
FLOATNAME(LMatrix3)(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02,
	 FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12,
	 FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22) {
  set(e00, e01, e02,
      e10, e11, e12,
      e20, e21, e22);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::set
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LMatrix3)::
set(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e02,
    FLOATTYPE e10, FLOATTYPE e11, FLOATTYPE e12,
    FLOATTYPE e20, FLOATTYPE e21, FLOATTYPE e22) {
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
INLINE void FLOATNAME(LMatrix3)::
set_row(int row, const FLOATNAME(LVecBase3) &v) {
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
INLINE void FLOATNAME(LMatrix3)::
set_col(int col, const FLOATNAME(LVecBase3) &v) {
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
INLINE void FLOATNAME(LMatrix3)::
set_row(int row, const FLOATNAME(LVecBase2) &v) {
  (*this)(row, 0) = v[0];
  (*this)(row, 1) = v[1];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::set_column
//       Access: Public
//  Description: Replaces the indicated column of the matrix from a
//               two-component vector, ignoring the last row.
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LMatrix3)::
set_col(int col, const FLOATNAME(LVecBase2) &v) {
  (*this)(0, col) = v[0];
  (*this)(1, col) = v[1];
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_row
//       Access: Public
//  Description: Returns the indicated row of the matrix as a
//               three-component vector.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3) FLOATNAME(LMatrix3)::
get_row(int row) const {
  return FLOATNAME(LVecBase3)((*this)(row, 0), (*this)(row, 1), (*this)(row, 2));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_col
//       Access: Public
//  Description: Returns the indicated column of the matrix as a
//               three-component vector.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3) FLOATNAME(LMatrix3)::
get_col(int col) const {
  return FLOATNAME(LVecBase3)((*this)(0, col), (*this)(1, col), (*this)(2, col));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_row2
//       Access: Public
//  Description: Returns the indicated row of the matrix as a
//               two-component vector, ignoring the last column.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase2) FLOATNAME(LMatrix3)::
get_row2(int row) const {
  return FLOATNAME(LVecBase2)((*this)(row, 0), (*this)(row, 1));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_col2
//       Access: Public
//  Description: Returns the indicated column of the matrix as a
//               two-component vector, ignoring the last row.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase2) FLOATNAME(LMatrix3)::
get_col2(int col) const {
  return FLOATNAME(LVecBase2)((*this)(0, col), (*this)(1, col));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Indexing operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE &FLOATNAME(LMatrix3)::
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
INLINE FLOATTYPE FLOATNAME(LMatrix3)::
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
INLINE bool FLOATNAME(LMatrix3)::
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
INLINE FLOATTYPE FLOATNAME(LMatrix3)::
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
INLINE void FLOATNAME(LMatrix3)::
set_cell(int row, int col, FLOATTYPE value) {
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
INLINE const FLOATTYPE *FLOATNAME(LMatrix3)::
get_data() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::get_num_components
//       Access: Public
//  Description: Returns the number of elements in the matrix, nine.
////////////////////////////////////////////////////////////////////
INLINE int FLOATNAME(LMatrix3)::
get_num_components() const {
  return 9;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3)::iterator FLOATNAME(LMatrix3)::
begin() {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3)::iterator FLOATNAME(LMatrix3)::
end() {
  return begin() + get_num_components();
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::begin
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3)::const_iterator FLOATNAME(LMatrix3)::
begin() const {
  return _data;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::end
//       Access: Public
//  Description: Returns an iterator that may be used to traverse the
//               elements of the matrix, STL-style.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3)::const_iterator FLOATNAME(LMatrix3)::
end() const {
  return begin() + get_num_components();
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LMatrix3)::
operator != (const FLOATNAME(LMatrix3) &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::compare_to
//       Access: Public
//  Description: This flavor of compare_to uses a default threshold
//               value based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE int FLOATNAME(LMatrix3)::
compare_to(const FLOATNAME(LMatrix3) &other) const {
  return compare_to(other, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::xform
//       Access: Public
//  Description: 3-component vector or point times matrix.  This is a
//               fully general operation.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase3) FLOATNAME(LMatrix3)::
xform(const FLOATNAME(LVecBase3) &v) const {
  return FLOATNAME(LVecBase3)(v.dot(get_col(0)),
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
INLINE FLOATNAME(LVecBase2) FLOATNAME(LMatrix3)::
xform_point(const FLOATNAME(LVecBase2) &v) const {
  return FLOATNAME(LVecBase2)(v.dot(get_col2(0)) + (*this)(2, 0),
			    v.dot(get_col2(1)) + (*this)(2, 1));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::xform_vec
//       Access: Public
//  Description: The matrix transforms a 2-component vector (without
//               translation component) and returns the result.  This
//               assumes the matrix is an affine transform.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LVecBase2) FLOATNAME(LMatrix3)::
xform_vec(const FLOATNAME(LVecBase2) &v) const {
  return FLOATNAME(LVecBase2)(v.dot(get_col2(0)),
			    v.dot(get_col2(1)));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::mult_cel
//       Access: Private
//  Description: Returns one cell of the result of a matrix-matrix
//               multiplication operation.
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LMatrix3)::
mult_cel(const FLOATNAME(LMatrix3) &other, int row, int col) const {
  return get_row(row).dot(other.get_col(col));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix *= matrix
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator *= (const FLOATNAME(LMatrix3) &other) {
  (*this) = (*this) * other;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::matrix *= scalar
//       Access: Public
//  Description: Performs a memberwise scale.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator *= (FLOATTYPE scalar) {
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
INLINE FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator /= (FLOATTYPE scalar) {
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
INLINE void FLOATNAME(LMatrix3)::
transpose_from(const FLOATNAME(LMatrix3) &other) {
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
INLINE void FLOATNAME(LMatrix3)::
transpose_in_place() {
  FLOATNAME(LMatrix3) temp = (*this);
  transpose_from(temp);
}

// Matrix inversion code from Numerical Recipes in C.

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::det2
//       Access: Private, Static
//  Description: Returns the determinant of a 2x2 matrix.
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LMatrix3)::
det2(FLOATTYPE e00, FLOATTYPE e01, FLOATTYPE e10, FLOATTYPE e11) const {
  return (e00 * e11 - e10 * e01);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::determinant
//       Access: Public
//  Description: Returns the determinant of the matrix.
////////////////////////////////////////////////////////////////////
INLINE FLOATTYPE FLOATNAME(LMatrix3)::
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
INLINE bool FLOATNAME(LMatrix3)::
invert_from(const FLOATNAME(LMatrix3) &other) {
  FLOATTYPE d = other.determinant();

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
INLINE bool FLOATNAME(LMatrix3)::
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
INLINE FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
translate_mat(const FLOATNAME(LVecBase2) &trans) {
  return FLOATNAME(LMatrix3)(1.0, 0.0, 0.0,
			   0.0, 1.0, 0.0,
			   trans[0], trans[1], 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::translate_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               translation.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
translate_mat(FLOATTYPE tx, FLOATTYPE ty) {
  return FLOATNAME(LMatrix3)(1.0, 0.0, 0.0,
			   0.0, 1.0, 0.0,
			   tx, ty, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::rotate_mat
//       Access: Public, Static
//  Description: Returns a matrix that rotates by the given angle in
//               degrees counterclockwise.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
rotate_mat(FLOATTYPE angle) {
  double angle_rad=deg_2_rad(angle);
  double s,c;
  csincos(angle_rad,&s,&c);
  return FLOATNAME(LMatrix3)(  c,    s,  0.0,
			      -s,    c,  0.0,
			     0.0,  0.0,  1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale in each of the two axes.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_mat(const FLOATNAME(LVecBase2) &scale) {
  return FLOATNAME(LMatrix3)(scale[0], 0.0, 0.0,
			     0.0, scale[1], 0.0,
			     0.0, 0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale in each of the two axes.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_mat(FLOATTYPE sx, FLOATTYPE sy) {
  return FLOATNAME(LMatrix3)(sx, 0.0, 0.0,
			     0.0, sy, 0.0,
			     0.0, 0.0, 1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::rotate_mat
//       Access: Public, Static
//  Description: Returns a matrix that rotates by the given angle in
//               degrees counterclockwise about the indicated vector.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
rotate_mat(FLOATTYPE angle, FLOATNAME(LVecBase3) axis,
	   CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }
  FLOATNAME(LMatrix3) mat;

  if (!is_right_handed(cs)) {
    // In a left-handed coordinate system, counterclockwise is the
    // other direction.
    angle = -angle;
  }

  // Normalize the axis.
  FLOATTYPE length = axis.dot(axis);
  nassertr(length != 0.0, ident_mat());
  FLOATTYPE recip_length=1.0f/length;
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
INLINE FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_mat(const FLOATNAME(LVecBase3) &scale) {
  return FLOATNAME(LMatrix3)(scale[0], 0.0, 0.0,
			     0.0, scale[1], 0.0,
			     0.0, 0.0, scale[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale in each of the three axes.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_mat(FLOATTYPE sx, FLOATTYPE sy, FLOATTYPE sz) {
  return FLOATNAME(LMatrix3)(sx, 0.0, 0.0,
			     0.0, sy, 0.0,
			     0.0, 0.0, sz);
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::almost_equal
//       Access: Public
//  Description: Returns true if two matrices are memberwise equal
//               within a default tolerance based on the numeric type.
////////////////////////////////////////////////////////////////////
INLINE bool FLOATNAME(LMatrix3)::
almost_equal(const FLOATNAME(LMatrix3) &other) const {
  return almost_equal(other, NEARLY_ZERO(FLOATTYPE));
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LMatrix3)::
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
//     Function: LMatrix3::transpose
//  Description: Transposes the given matrix and returns it.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3)
transpose(const FLOATNAME(LMatrix3) &a) {
  FLOATNAME(LMatrix3) result;
  result.transpose_from(a);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::invert
//  Description: Inverts the given matrix and returns it.
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3)
invert(const FLOATNAME(LMatrix3) &a) {
  FLOATNAME(LMatrix3) result;
  bool nonsingular = result.invert_from(a);
  nassertr(nonsingular, FLOATNAME(LMatrix3)::ident_mat());
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void FLOATNAME(LMatrix3)::
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
//     Function: LMatrix3::matrix * matrix
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
operator * (const FLOATNAME(LMatrix3) &other) const {
  FLOATNAME(LMatrix3) t;

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
INLINE FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
operator * (FLOATTYPE scalar) const {
  FLOATNAME(LMatrix3) t;

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
INLINE FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
operator / (FLOATTYPE scalar) const {
  FLOATNAME(LMatrix3) t;

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
INLINE FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator += (const FLOATNAME(LMatrix3) &other) {
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
INLINE FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
operator -= (const FLOATNAME(LMatrix3) &other) {
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
