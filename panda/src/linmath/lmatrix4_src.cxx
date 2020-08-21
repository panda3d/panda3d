/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lmatrix4_src.cxx
 * @author drose
 * @date 1999-01-15
 */

TypeHandle FLOATNAME(LMatrix4)::_type_handle;
TypeHandle FLOATNAME(UnalignedLMatrix4)::_type_handle;

const FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::_ident_mat =
  FLOATNAME(LMatrix4)(1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);

const FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::_ones_mat =
  FLOATNAME(LMatrix4)(1.0f, 1.0f, 1.0f, 1.0f,
                      1.0f, 1.0f, 1.0f, 1.0f,
                      1.0f, 1.0f, 1.0f, 1.0f,
                      1.0f, 1.0f, 1.0f, 1.0f);

const FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::_zeros_mat =
  FLOATNAME(LMatrix4)(0.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 0.0f);

const FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::_y_to_z_up_mat =
  FLOATNAME(LMatrix4)(1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f,-1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);

const FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::_z_to_y_up_mat =
  FLOATNAME(LMatrix4)(1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 0.0f,-1.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);

const FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::_flip_y_mat =
  FLOATNAME(LMatrix4)(1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f,-1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);

const FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::_flip_z_mat =
  FLOATNAME(LMatrix4)(1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f,-1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);

const FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::_lz_to_ry_mat =
  FLOATNAME(LMatrix4)::_flip_y_mat * FLOATNAME(LMatrix4)::_z_to_y_up_mat;

const FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::_ly_to_rz_mat =
  FLOATNAME(LMatrix4)::_flip_z_mat * FLOATNAME(LMatrix4)::_y_to_z_up_mat;

/**
 * Returns a matrix that transforms from the indicated coordinate system to
 * the indicated coordinate system.
 */
const FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
convert_mat(CoordinateSystem from, CoordinateSystem to) {
  TAU_PROFILE("LMatrix4 LMatrix4::convert_mat(CoordinateSystem, CoordinateSystem)", " ", TAU_USER);
  if (from == CS_default) {
    from = get_default_coordinate_system();
  }
  if (to == CS_default) {
    to = get_default_coordinate_system();
  }
  switch (from) {
  case CS_zup_left:
    switch (to) {
    case CS_zup_left: return _ident_mat;
    case CS_yup_left: return _z_to_y_up_mat;
    case CS_zup_right: return _flip_y_mat;
    case CS_yup_right: return _lz_to_ry_mat;
    default: break;
    }
    break;

  case CS_yup_left:
    switch (to) {
    case CS_zup_left: return _y_to_z_up_mat;
    case CS_yup_left: return _ident_mat;
    case CS_zup_right: return _ly_to_rz_mat;
    case CS_yup_right: return _flip_z_mat;
    default: break;
    }
    break;

  case CS_zup_right:
    switch (to) {
    case CS_zup_left: return _flip_y_mat;
    case CS_yup_left: return _lz_to_ry_mat;
    case CS_zup_right: return _ident_mat;
    case CS_yup_right: return _z_to_y_up_mat;
    default: break;
    }
    break;

  case CS_yup_right:
    switch (to) {
    case CS_zup_left: return _ly_to_rz_mat;
    case CS_yup_left: return _flip_z_mat;
    case CS_zup_right: return _y_to_z_up_mat;
    case CS_yup_right: return _ident_mat;
    default: break;
    }
    break;

  default:
    break;
  }

  linmath_cat.error()
    << "Invalid coordinate system value!\n";
  return _ident_mat;
}

/**
 * Sorts matrices lexicographically, componentwise.  Returns a number less
 * than 0 if this matrix sorts before the other one, greater than zero if it
 * sorts after, 0 if they are equivalent (within the indicated tolerance).
 */
int FLOATNAME(LMatrix4)::
compare_to(const FLOATNAME(LMatrix4) &other, FLOATTYPE threshold) const {
  TAU_PROFILE("int LMatrix4::compare_to(const LMatrix4 &, FLOATTYPE)", " ", TAU_USER);
  // We compare values in reverse order, since the last row of the matrix is
  // most likely to be different between different matrices.
  for (int r = 3; r >= 0; --r) {
    for (int c = 0; c < 4; ++c) {
      if (!IS_THRESHOLD_COMPEQ(_m(r, c), other._m(r, c), threshold)) {
        return (_m(r, c) < other._m(r, c)) ? -1 : 1;
      }
    }
  }
  return 0;
}

/**
 * Sets mat to a matrix that rotates by the given angle in degrees
 * counterclockwise about the indicated vector.
 */
void FLOATNAME(LMatrix4)::
set_rotate_mat(FLOATTYPE angle, const FLOATNAME(LVecBase3) &axis,
               CoordinateSystem cs) {
  TAU_PROFILE("void LMatrix4::set_rotate_mat(FLOATTYPE, const LVecBase3 &, cs)", " ", TAU_USER);

  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  if (IS_LEFT_HANDED_COORDSYSTEM(cs)) {
    // In a left-handed coordinate system, counterclockwise is the other
    // direction.
    angle = -angle;
  }

  FLOATTYPE axis_0 = axis._v(0);
  FLOATTYPE axis_1 = axis._v(1);
  FLOATTYPE axis_2 = axis._v(2);

  // Normalize the axis.
  FLOATTYPE length_sq = axis_0 * axis_0 + axis_1 * axis_1 + axis_2 * axis_2;
  nassertv(length_sq != 0.0f);
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

  _m(0, 0) = t0 * axis_0 + c;
  _m(0, 1) = t0 * axis_1 + s2;
  _m(0, 2) = t0 * axis_2 - s1;

  _m(1, 0) = t1 * axis_0 - s2;
  _m(1, 1) = t1 * axis_1 + c;
  _m(1, 2) = t1 * axis_2 + s0;

  _m(2, 0) = t2 * axis_0 + s1;
  _m(2, 1) = t2 * axis_1 - s0;
  _m(2, 2) = t2 * axis_2 + c;

  _m(0, 3) = 0.0f;
  _m(1, 3) = 0.0f;
  _m(2, 3) = 0.0f;

  _m(3, 0) = 0.0f;
  _m(3, 1) = 0.0f;
  _m(3, 2) = 0.0f;
  _m(3, 3) = 1.0f;
}

/**
 * Fills mat with a matrix that rotates by the given angle in degrees
 * counterclockwise about the indicated vector.  Assumes axis has been
 * prenormalized.
 */
void FLOATNAME(LMatrix4)::
set_rotate_mat_normaxis(FLOATTYPE angle, const FLOATNAME(LVecBase3) &axis,
                        CoordinateSystem cs) {
  TAU_PROFILE("void LMatrix4::set_rotate_mat_normaxis(FLOATTYPE, const LVecBase3 &, cs)", " ", TAU_USER);
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  if (IS_LEFT_HANDED_COORDSYSTEM(cs)) {
    // In a left-handed coordinate system, counterclockwise is the other
    // direction.
    angle = -angle;
  }

  FLOATTYPE axis_0 = axis._v(0);
  FLOATTYPE axis_1 = axis._v(1);
  FLOATTYPE axis_2 = axis._v(2);

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

  _m(0, 0) = t0 * axis_0 + c;
  _m(0, 1) = t0 * axis_1 + s2;
  _m(0, 2) = t0 * axis_2 - s1;

  _m(1, 0) = t1 * axis_0 - s2;
  _m(1, 1) = t1 * axis_1 + c;
  _m(1, 2) = t1 * axis_2 + s0;

  _m(2, 0) = t2 * axis_0 + s1;
  _m(2, 1) = t2 * axis_1 - s0;
  _m(2, 2) = t2 * axis_2 + c;

  _m(0, 3) = 0.0f;
  _m(1, 3) = 0.0f;
  _m(2, 3) = 0.0f;

  _m(3, 0) = 0.0f;
  _m(3, 1) = 0.0f;
  _m(3, 2) = 0.0f;
  _m(3, 3) = 1.0f;
}

/**
 * Returns true if two matrices are memberwise equal within a specified
 * tolerance.  This is faster than the equivalence operator as this doesn't
 * have to guarantee that it is transitive.
 */
bool FLOATNAME(LMatrix4)::
almost_equal(const FLOATNAME(LMatrix4) &other, FLOATTYPE threshold) const {
  TAU_PROFILE("bool LMatrix4::almost_equal(const LMatrix4 &, FLOATTYPE)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return ((_m - other._m).cwiseAbs().maxCoeff() < threshold);
#else
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
#endif
}

/**
 *
 */
void FLOATNAME(LMatrix4)::
output(std::ostream &out) const {
  out << "[ "
      << MAYBE_ZERO(_m(0, 0)) << " "
      << MAYBE_ZERO(_m(0, 1)) << " "
      << MAYBE_ZERO(_m(0, 2)) << " "
      << MAYBE_ZERO(_m(0, 3))
      << " ] [ "
      << MAYBE_ZERO(_m(1, 0)) << " "
      << MAYBE_ZERO(_m(1, 1)) << " "
      << MAYBE_ZERO(_m(1, 2)) << " "
      << MAYBE_ZERO(_m(1, 3))
      << " ] [ "
      << MAYBE_ZERO(_m(2, 0)) << " "
      << MAYBE_ZERO(_m(2, 1)) << " "
      << MAYBE_ZERO(_m(2, 2)) << " "
      << MAYBE_ZERO(_m(2, 3))
      << " ] [ "
      << MAYBE_ZERO(_m(3, 0)) << " "
      << MAYBE_ZERO(_m(3, 1)) << " "
      << MAYBE_ZERO(_m(3, 2)) << " "
      << MAYBE_ZERO(_m(3, 3))
      << " ]";
}

/**
 *
 */
void FLOATNAME(LMatrix4)::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << MAYBE_ZERO(_m(0, 0)) << " "
    << MAYBE_ZERO(_m(0, 1)) << " "
    << MAYBE_ZERO(_m(0, 2)) << " "
    << MAYBE_ZERO(_m(0, 3))
    << "\n";
  indent(out, indent_level)
    << MAYBE_ZERO(_m(1, 0)) << " "
    << MAYBE_ZERO(_m(1, 1)) << " "
    << MAYBE_ZERO(_m(1, 2)) << " "
    << MAYBE_ZERO(_m(1, 3))
    << "\n";
  indent(out, indent_level)
    << MAYBE_ZERO(_m(2, 0)) << " "
    << MAYBE_ZERO(_m(2, 1)) << " "
    << MAYBE_ZERO(_m(2, 2)) << " "
    << MAYBE_ZERO(_m(2, 3))
    << "\n";
  indent(out, indent_level)
    << MAYBE_ZERO(_m(3, 0)) << " "
    << MAYBE_ZERO(_m(3, 1)) << " "
    << MAYBE_ZERO(_m(3, 2)) << " "
    << MAYBE_ZERO(_m(3, 3))
    << "\n";
}


/**
 * Adds the vector to the indicated hash generator.
 */
void FLOATNAME(LMatrix4)::
generate_hash(ChecksumHashGenerator &hashgen, FLOATTYPE threshold) const {
  TAU_PROFILE("void LMatrix4::generate_hash(ChecksumHashGenerator &, FLOATTYPE)", " ", TAU_USER);
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      hashgen.add_fp(get_cell(i,j), threshold);
    }
  }
}

/**
 *
 */
bool FLOATNAME(LMatrix4)::
decompose_mat(int index[4]) {
  TAU_PROFILE("bool LMatrix4::decompose_mat(int[4])", " ", TAU_USER);
  int i, j, k;
  FLOATTYPE vv[4];
  for (i = 0; i < 4; i++) {
    FLOATTYPE big = 0.0f;
    for (j = 0; j < 4; j++) {
      FLOATTYPE temp = fabs((*this)(i,j));
      if (temp > big) {
        big = temp;
      }
    }

    // We throw the value out only if it's smaller than our "small" threshold
    // squared.  This helps reduce overly-sensitive rejections.
    if (IS_THRESHOLD_ZERO(big, (NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE)))) {
      // if (IS_NEARLY_ZERO(big)) {
      return false;
    }
    vv[i] = 1.0f / big;
  }

  for (j = 0; j < 4; j++) {
    for (i = 0; i < j; i++) {
      FLOATTYPE sum = (*this)(i,j);
      for (k = 0; k < i; k++) {
        sum -= (*this)(i,k) * (*this)(k,j);
      }
      (*this)(i,j) = sum;
    }

    FLOATTYPE big = 0.0f;
    int imax = -1;
    for (i = j; i < 4; i++) {
      FLOATTYPE sum = (*this)(i,j);
      for (k = 0; k < j; k++) {
        sum -= (*this)(i,k) * (*this)(k,j);
      }
      (*this)(i,j) = sum;

      FLOATTYPE dum = vv[i] * fabs(sum);
      if (dum >= big) {
        big = dum;
        imax = i;
      }
    }
    nassertr(imax >= 0, false);
    if (j != imax) {
      for (k = 0; k < 4; k++) {
        FLOATTYPE dum = (*this)(imax,k);
        (*this)(imax,k) = (*this)(j,k);
        (*this)(j,k) = dum;
      }
      vv[imax] = vv[j];
    }
    index[j] = imax;

    if ((*this)(j,j) == 0.0f) {
      (*this)(j,j) = NEARLY_ZERO(FLOATTYPE);
    }

    if (j != 4 - 1) {
      FLOATTYPE dum = 1.0f / (*this)(j,j);
      for (i = j + 1; i < 4; i++) {
        (*this)(i,j) *= dum;
      }
    }
  }
  return true;
}

/**
 *
 */
bool FLOATNAME(LMatrix4)::
back_sub_mat(int index[4], FLOATNAME(LMatrix4) &inv, int row) const {
  TAU_PROFILE("bool LMatrix4::back_sub_mat(int[4], LMatrix4 &, int)", " ", TAU_USER);
  int ii = -1;
  int i, j;
  for (i = 0; i < 4; i++) {
    int ip = index[i];
    FLOATTYPE sum = inv(row, ip);
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
    FLOATTYPE sum = inv(row, i);
    for (j = i + 1; j < 4; j++) {
      sum -= (*this)(i,j) * inv(row, j);
    }
    inv(row, i) = sum / (*this)(i,i);
  }

  return true;
}

/**
 * Writes the matrix to the Datagram using add_float32() or add_float64(),
 * depending on the type of floats in the matrix, regardless of the setting of
 * Datagram::set_stdfloat_double().  This is appropriate when you want to
 * write a fixed-width value to the datagram, especially when you are not
 * writing a bam file.
 */
void FLOATNAME(LMatrix4)::
write_datagram_fixed(Datagram &destination) const {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
#if FLOATTOKEN == 'f'
      destination.add_float32(get_cell(i,j));
#else
      destination.add_float64(get_cell(i,j));
#endif
    }
  }
}

/**
 * Reads the matrix from the Datagram using get_float32() or get_float64().
 * See write_datagram_fixed().
 */
void FLOATNAME(LMatrix4)::
read_datagram_fixed(DatagramIterator &scan) {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
#if FLOATTOKEN == 'f'
      set_cell(i, j, scan.get_float32());
#else
      set_cell(i, j, scan.get_float64());
#endif
    }
  }
}

/**
 * Writes the matrix to the Datagram using add_stdfloat().  This is
 * appropriate when you want to write the matrix using the standard width
 * setting, especially when you are writing a bam file.
 */
void FLOATNAME(LMatrix4)::
write_datagram(Datagram &destination) const {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      destination.add_stdfloat(get_cell(i,j));
    }
  }
}

/**
 * Reads the matrix from the Datagram using get_stdfloat().
 */
void FLOATNAME(LMatrix4)::
read_datagram(DatagramIterator &scan) {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      set_cell(i, j, scan.get_stdfloat());
    }
  }
}

/**
 *
 */
void FLOATNAME(LMatrix4)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    register_type(_type_handle, FLOATNAME_STR(LMatrix4));
  }
}

/**
 *
 */
void FLOATNAME(UnalignedLMatrix4)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    register_type(_type_handle, FLOATNAME_STR(UnalignedLMatrix4));
  }
}
