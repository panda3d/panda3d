/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lmatrix3_src.cxx
 * @author drose
 * @date 1999-01-29
 */

TypeHandle FLOATNAME(LMatrix3)::_type_handle;

const FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::_ident_mat =
  FLOATNAME(LMatrix3)(1.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 1.0f);

const FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::_y_to_z_up_mat =
  FLOATNAME(LMatrix3)(1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f,
                      0.0f,-1.0f, 0.0f);

const FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::_z_to_y_up_mat =
  FLOATNAME(LMatrix3)(1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f,-1.0f,
                      0.0f, 1.0f, 0.0f);

const FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::_flip_y_mat =
  FLOATNAME(LMatrix3)(1.0f, 0.0f, 0.0f,
                      0.0f,-1.0f, 0.0f,
                      0.0f, 0.0f, 1.0f);

const FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::_flip_z_mat =
  FLOATNAME(LMatrix3)(1.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f,-1.0f);

const FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::_lz_to_ry_mat =
  FLOATNAME(LMatrix3)::_flip_y_mat * FLOATNAME(LMatrix3)::_z_to_y_up_mat;

const FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::_ly_to_rz_mat =
  FLOATNAME(LMatrix3)::_flip_z_mat * FLOATNAME(LMatrix3)::_y_to_z_up_mat;

/**
 * Fills mat with a matrix that applies the indicated scale and shear.
 */
void FLOATNAME(LMatrix3)::
set_scale_shear_mat(const FLOATNAME(LVecBase3) &scale,
                    const FLOATNAME(LVecBase3) &shear,
                    CoordinateSystem cs) {
  TAU_PROFILE("void LMatrix3::set_scale_shear_mat(const LVecBase3 &, const LVecBase3 &)", " ", TAU_USER);
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  // We have to match the placement of the shear components in the matrix to
  // the way we extract out the rotation in decompose_matrix().  Therefore,
  // the shear is sensitive to the coordinate system.

  switch (cs) {
  case CS_zup_right:
    set(scale._v(0), shear._v(0) * scale._v(0), 0.0f,
        0.0f, scale._v(1), 0.0f,
        shear._v(1) * scale._v(2), shear._v(2) * scale._v(2), scale._v(2));
    break;

  case CS_zup_left:
    set(scale._v(0), shear._v(0) * scale._v(0), 0.0f,
        0.0f, scale._v(1), 0.0f,
        -shear._v(1) * scale._v(2), -shear._v(2) * scale._v(2), scale._v(2));
    break;

  case CS_yup_right:
    set(scale._v(0), 0.0f, shear._v(1) * scale._v(0),
        shear._v(0) * scale._v(1), scale._v(1), shear._v(2) * scale._v(1),
        0.0f, 0.0f, scale._v(2));
    break;

  case CS_yup_left:
    set(scale._v(0), 0.0f, -shear._v(1) * scale._v(0),
        shear._v(0) * scale._v(1), scale._v(1), -shear._v(2) * scale._v(1),
        0.0f, 0.0f, scale._v(2));
    break;

  case CS_default:
  case CS_invalid:
  default:
    // These should not be possible.
    linmath_cat.error()
      << "Invalid coordinate system value!\n";
    break;
  }
}

/**
 * Returns a matrix that transforms from the indicated coordinate system to
 * the indicated coordinate system.
 */
const FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
convert_mat(CoordinateSystem from, CoordinateSystem to) {
  TAU_PROFILE("LMatrix3 LMatrix3::convert_mat(CoordinateSystem, CoordinateSystem)", " ", TAU_USER);
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
 * Sets each element of the matrix to the indicated fill_value.  This is of
 * questionable value, but is sometimes useful when initializing to zero.
 */
void FLOATNAME(LMatrix3)::
fill(FLOATTYPE fill_value) {
  TAU_PROFILE("void LMatrix3::fill(FLOATTYPE)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  _m = EMatrix3::Constant(fill_value);
#else
  set(fill_value, fill_value, fill_value,
      fill_value, fill_value, fill_value,
      fill_value, fill_value, fill_value);
#endif  // HAVE_EIGEN
}

/**
 * Sorts matrices lexicographically, componentwise.  Returns a number less
 * than 0 if this matrix sorts before the other one, greater than zero if it
 * sorts after, 0 if they are equivalent (within the indicated tolerance).
 */
int FLOATNAME(LMatrix3)::
compare_to(const FLOATNAME(LMatrix3) &other, FLOATTYPE threshold) const {
  TAU_PROFILE("int LMatrix3::compare_to(const LMatrix3 &, FLOATTYPE)", " ", TAU_USER);
  for (int r = 0; r < 3; ++r) {
    for (int c = 0; c < 3; ++c) {
      if (!IS_THRESHOLD_COMPEQ(_m(r, c), other._m(r, c), threshold)) {
        return (_m(r, c) < other._m(r, c)) ? -1 : 1;
      }
    }
  }
  return 0;
}

/**
 * Fills mat with a matrix that rotates by the given angle in degrees
 * counterclockwise about the indicated vector.
 */
void FLOATNAME(LMatrix3)::
set_rotate_mat(FLOATTYPE angle, const FLOATNAME(LVecBase3) &axis,
               CoordinateSystem cs) {
  TAU_PROFILE("void LMatrix3::set_rotate_mat(FLOATTYPE, LVecBase3, CoordinateSystem)", " ", TAU_USER);
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

  FLOATTYPE angle_rad = deg_2_rad(angle);
  FLOATTYPE s,c;
  csincos(angle_rad, &s, &c);
  FLOATTYPE t = 1.0f - c;

  FLOATTYPE t0, t1, t2, s0, s1, s2;

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
}

/**
 * Fills mat with a matrix that rotates by the given angle in degrees
 * counterclockwise about the indicated vector.  Assumes axis has been
 * normalized.
 */
void FLOATNAME(LMatrix3)::
set_rotate_mat_normaxis(FLOATTYPE angle, const FLOATNAME(LVecBase3) &axis,
                        CoordinateSystem cs) {
  TAU_PROFILE("void LMatrix3::set_rotate_mat_normaxis(FLOATTYPE, LVecBase3, CoordinateSystem)", " ", TAU_USER);
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

  FLOATTYPE angle_rad = deg_2_rad(angle);
  FLOATTYPE s, c;
  csincos(angle_rad, &s, &c);
  FLOATTYPE t = 1.0f - c;

  FLOATTYPE t0, t1, t2, s0, s1, s2;

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
}

/**
 * Returns true if two matrices are memberwise equal within a specified
 * tolerance.
 */
bool FLOATNAME(LMatrix3)::
almost_equal(const FLOATNAME(LMatrix3) &other, FLOATTYPE threshold) const {
  TAU_PROFILE("bool LMatrix3::almost_equal(const LMatrix3 &, FLOATTYPE)", " ", TAU_USER);
#ifdef HAVE_EIGEN
  return ((_m - other._m).cwiseAbs().maxCoeff() < threshold);
#else
  return (IS_THRESHOLD_EQUAL((*this)(0, 0), other(0, 0), threshold) &&
          IS_THRESHOLD_EQUAL((*this)(0, 1), other(0, 1), threshold) &&
          IS_THRESHOLD_EQUAL((*this)(0, 2), other(0, 2), threshold) &&
          IS_THRESHOLD_EQUAL((*this)(1, 0), other(1, 0), threshold) &&
          IS_THRESHOLD_EQUAL((*this)(1, 1), other(1, 1), threshold) &&
          IS_THRESHOLD_EQUAL((*this)(1, 2), other(1, 2), threshold) &&
          IS_THRESHOLD_EQUAL((*this)(2, 0), other(2, 0), threshold) &&
          IS_THRESHOLD_EQUAL((*this)(2, 1), other(2, 1), threshold) &&
          IS_THRESHOLD_EQUAL((*this)(2, 2), other(2, 2), threshold));
#endif
}

/**
 *
 */
void FLOATNAME(LMatrix3)::
output(std::ostream &out) const {
  out << "[ "
      << MAYBE_ZERO(_m(0, 0)) << " "
      << MAYBE_ZERO(_m(0, 1)) << " "
      << MAYBE_ZERO(_m(0, 2))
      << " ] [ "
      << MAYBE_ZERO(_m(1, 0)) << " "
      << MAYBE_ZERO(_m(1, 1)) << " "
      << MAYBE_ZERO(_m(1, 2))
      << " ] [ "
      << MAYBE_ZERO(_m(2, 0)) << " "
      << MAYBE_ZERO(_m(2, 1)) << " "
      << MAYBE_ZERO(_m(2, 2))
      << " ]";
}

/**
 *
 */
void FLOATNAME(LMatrix3)::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << MAYBE_ZERO(_m(0, 0)) << " "
    << MAYBE_ZERO(_m(0, 1)) << " "
    << MAYBE_ZERO(_m(0, 2))
    << "\n";
  indent(out, indent_level)
    << MAYBE_ZERO(_m(1, 0)) << " "
    << MAYBE_ZERO(_m(1, 1)) << " "
    << MAYBE_ZERO(_m(1, 2))
    << "\n";
  indent(out, indent_level)
    << MAYBE_ZERO(_m(2, 0)) << " "
    << MAYBE_ZERO(_m(2, 1)) << " "
    << MAYBE_ZERO(_m(2, 2))
    << "\n";
}

/**
 * Adds the vector to the indicated hash generator.
 */
void FLOATNAME(LMatrix3)::
generate_hash(ChecksumHashGenerator &hashgen, FLOATTYPE threshold) const {
  TAU_PROFILE("void LMatrix3::generate_hash(ChecksumHashGenerator &, FLOATTYPE)", " ", TAU_USER);
  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      hashgen.add_fp(get_cell(i,j), threshold);
    }
  }
}

/**
 * Writes the matrix to the Datagram using add_float32() or add_float64(),
 * depending on the type of floats in the matrix, regardless of the setting of
 * Datagram::set_stdfloat_double().  This is appropriate when you want to
 * write a fixed-width value to the datagram, especially when you are not
 * writing a bam file.
 */
void FLOATNAME(LMatrix3)::
write_datagram_fixed(Datagram &destination) const {
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
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
void FLOATNAME(LMatrix3)::
read_datagram_fixed(DatagramIterator &scan) {
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
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
void FLOATNAME(LMatrix3)::
write_datagram(Datagram &destination) const {
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      destination.add_stdfloat(get_cell(i,j));
    }
  }
}

/**
 * Reads the matrix from the Datagram using get_stdfloat().
 */
void FLOATNAME(LMatrix3)::
read_datagram(DatagramIterator &scan) {
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      set_cell(i, j, scan.get_stdfloat());
    }
  }
}


/**
 *
 */
void FLOATNAME(LMatrix3)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    register_type(_type_handle, FLOATNAME_STR(LMatrix3));
  }
}
