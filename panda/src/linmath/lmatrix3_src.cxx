// Filename: lmatrix3_src.cxx
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


#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::__reduce__
//       Access: Published
//  Description: This special Python method is implement to provide
//               support for the pickle module.
////////////////////////////////////////////////////////////////////
PyObject *FLOATNAME(LMatrix3)::
__reduce__(PyObject *self) const {
  // We should return at least a 2-tuple, (Class, (args)): the
  // necessary class object whose constructor we should call
  // (e.g. this), and the arguments necessary to reconstruct this
  // object.
  PyObject *this_class = PyObject_Type(self);
  if (this_class == NULL) {
    return NULL;
  }

  PyObject *result = Py_BuildValue("(O(fffffffff))", this_class, 
                                   _m.m._00, _m.m._01, _m.m._02,
                                   _m.m._10, _m.m._11, _m.m._12,
                                   _m.m._20, _m.m._21, _m.m._22);
  Py_DECREF(this_class);
  return result;
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::set_scale_shear_mat
//       Access: Public
//  Description: Fills mat with a matrix that applies the indicated
//               scale and shear.
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix3)::
set_scale_shear_mat(const FLOATNAME(LVecBase3) &scale,
                    const FLOATNAME(LVecBase3) &shear,
                    CoordinateSystem cs) {
  TAU_PROFILE("void LMatrix3::set_scale_shear_mat(const LVecBase3 &, const LVecBase3 &)", " ", TAU_USER);
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  // We have to match the placement of the shear components in the
  // matrix to the way we extract out the rotation in
  // decompose_matrix().  Therefore, the shear is sensitive to the
  // coordinate system.

  switch (cs) {
  case CS_zup_right:
    if (temp_hpr_fix) {
      set(scale._v.v._0, shear._v.v._0 * scale._v.v._0, 0.0f,
          0.0f, scale._v.v._1, 0.0f,
          shear._v.v._1 * scale._v.v._2, shear._v.v._2 * scale._v.v._2, scale._v.v._2);
    } else {
      set(scale._v.v._0, 0.0f, 0.0f,
          shear._v.v._0 * scale._v.v._1, scale._v.v._1, 0.0f,
          shear._v.v._1 * scale._v.v._2, shear._v.v._2 * scale._v.v._2, scale._v.v._2);
    }
    break;
    
  case CS_zup_left:
    if (temp_hpr_fix) {
      set(scale._v.v._0, shear._v.v._0 * scale._v.v._0, 0.0f,
          0.0f, scale._v.v._1, 0.0f,
          -shear._v.v._1 * scale._v.v._2, -shear._v.v._2 * scale._v.v._2, scale._v.v._2);
    } else {
      set(scale._v.v._0, 0.0f, 0.0f,
          shear._v.v._0 * scale._v.v._1, scale._v.v._1, 0.0f,
          -shear._v.v._1 * scale._v.v._2, -shear._v.v._2 * scale._v.v._2, scale._v.v._2);
    }
    break;
    
  case CS_yup_right:
    if (temp_hpr_fix) {
      set(scale._v.v._0, 0.0f, shear._v.v._1 * scale._v.v._0,
          shear._v.v._0 * scale._v.v._1, scale._v.v._1, shear._v.v._2 * scale._v.v._1,
          0.0f, 0.0f, scale._v.v._2);
    } else {
      set(scale._v.v._0, 0.0f, 0.0f,
          shear._v.v._0 * scale._v.v._1, scale._v.v._1, shear._v.v._2 * scale._v.v._1,
          shear._v.v._1 * scale._v.v._2, 0.0f, scale._v.v._2);
    }
    break;
    
  case CS_yup_left:
    if (temp_hpr_fix) {
      set(scale._v.v._0, 0.0f, -shear._v.v._1 * scale._v.v._0,
          shear._v.v._0 * scale._v.v._1, scale._v.v._1, -shear._v.v._2 * scale._v.v._1,
          0.0f, 0.0f, scale._v.v._2);
    } else {
      set(scale._v.v._0, 0.0f, 0.0f,
          shear._v.v._0 * scale._v.v._1, scale._v.v._1, -shear._v.v._2 * scale._v.v._1,
          -shear._v.v._1 * scale._v.v._2, 0.0f, scale._v.v._2);
    }
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

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::convert_mat
//       Access: Public, Static
//  Description: Returns a matrix that transforms from the indicated
//               coordinate system to the indicated coordinate system.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::fill
//       Access: Public
//  Description: Sets each element of the matrix to the indicated
//               fill_value.  This is of questionable value, but is
//               sometimes useful when initializing to zero.
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix3)::
fill(FLOATTYPE fill_value) {
  TAU_PROFILE("void LMatrix3::fill(FLOATTYPE)", " ", TAU_USER);
  set(fill_value, fill_value, fill_value,
      fill_value, fill_value, fill_value,
      fill_value, fill_value, fill_value);
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
int FLOATNAME(LMatrix3)::
compare_to(const FLOATNAME(LMatrix3) &other, FLOATTYPE threshold) const {
  TAU_PROFILE("int LMatrix3::compare_to(const LMatrix3 &, FLOATTYPE)", " ", TAU_USER);
  for (int i = 0; i < 9; i++) {
    if (!IS_THRESHOLD_COMPEQ(_m.data[i], other._m.data[i], threshold)) {
      return (_m.data[i] < other._m.data[i]) ? -1 : 1;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::set_rotate_mat
//       Access: Public
//  Description: Fills mat with a matrix that rotates by the given
//               angle in degrees counterclockwise about the indicated
//               vector.
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix3)::
set_rotate_mat(FLOATTYPE angle, FLOATNAME(LVecBase3) axis,
               CoordinateSystem cs) {
  TAU_PROFILE("void LMatrix3::set_rotate_mat(FLOATTYPE, LVecBase3, CoordinateSystem)", " ", TAU_USER);
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  if (IS_LEFT_HANDED_COORDSYSTEM(cs)) {
    // In a left-handed coordinate system, counterclockwise is the
    // other direction.
    angle = -angle;
  }

  FLOATTYPE axis_0 = axis._v.v._0;
  FLOATTYPE axis_1 = axis._v.v._1;
  FLOATTYPE axis_2 = axis._v.v._2;

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

  _m.m._00 = t0 * axis_0 + c;
  _m.m._01 = t0 * axis_1 + s2;
  _m.m._02 = t0 * axis_2 - s1;

  _m.m._10 = t1 * axis_0 - s2;
  _m.m._11 = t1 * axis_1 + c;
  _m.m._12 = t1 * axis_2 + s0;

  _m.m._20 = t2 * axis_0 + s1;
  _m.m._21 = t2 * axis_1 - s0;
  _m.m._22 = t2 * axis_2 + c;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::set_rotate_mat_normaxis
//       Access: Public
//  Description: Fills mat with a matrix that rotates by the given
//               angle in degrees counterclockwise about the indicated
//               vector.  Assumes axis has been normalized.
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix3)::
set_rotate_mat_normaxis(FLOATTYPE angle, const FLOATNAME(LVecBase3) &axis,
                        CoordinateSystem cs) {
  TAU_PROFILE("void LMatrix3::set_rotate_mat_normaxis(FLOATTYPE, LVecBase3, CoordinateSystem)", " ", TAU_USER);
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  if (IS_LEFT_HANDED_COORDSYSTEM(cs)) {
    // In a left-handed coordinate system, counterclockwise is the
    // other direction.
    angle = -angle;
  }

  FLOATTYPE axis_0 = axis._v.v._0;
  FLOATTYPE axis_1 = axis._v.v._1;
  FLOATTYPE axis_2 = axis._v.v._2;

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

  _m.m._00 = t0 * axis_0 + c;
  _m.m._01 = t0 * axis_1 + s2;
  _m.m._02 = t0 * axis_2 - s1;

  _m.m._10 = t1 * axis_0 - s2;
  _m.m._11 = t1 * axis_1 + c;
  _m.m._12 = t1 * axis_2 + s0;

  _m.m._20 = t2 * axis_0 + s1;
  _m.m._21 = t2 * axis_1 - s0;
  _m.m._22 = t2 * axis_2 + c;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::almost_equal
//       Access: Public
//  Description: Returns true if two matrices are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////
bool FLOATNAME(LMatrix3)::
almost_equal(const FLOATNAME(LMatrix3) &other, FLOATTYPE threshold) const {
  TAU_PROFILE("bool LMatrix3::almost_equal(const LMatrix3 &, FLOATTYPE)", " ", TAU_USER);
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
//     Function: LMatrix3::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix3)::
output(ostream &out) const {
  out << "[ "
      << MAYBE_ZERO(_m.m._00) << " "
      << MAYBE_ZERO(_m.m._01) << " "
      << MAYBE_ZERO(_m.m._02)
      << " ] [ "
      << MAYBE_ZERO(_m.m._10) << " "
      << MAYBE_ZERO(_m.m._11) << " "
      << MAYBE_ZERO(_m.m._12)
      << " ] [ "
      << MAYBE_ZERO(_m.m._20) << " "
      << MAYBE_ZERO(_m.m._21) << " "
      << MAYBE_ZERO(_m.m._22)
      << " ]";
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix3)::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << MAYBE_ZERO(_m.m._00) << " "
    << MAYBE_ZERO(_m.m._01) << " "
    << MAYBE_ZERO(_m.m._02)
    << "\n";
  indent(out, indent_level)
    << MAYBE_ZERO(_m.m._10) << " "
    << MAYBE_ZERO(_m.m._11) << " "
    << MAYBE_ZERO(_m.m._12)
    << "\n";
  indent(out, indent_level)
    << MAYBE_ZERO(_m.m._20) << " "
    << MAYBE_ZERO(_m.m._21) << " "
    << MAYBE_ZERO(_m.m._22)
    << "\n";
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::python_repr
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix3)::
python_repr(ostream &out, const string &class_name) const {
  out << class_name << "(" 
      << MAYBE_ZERO(_m.m._00) << ", "
      << MAYBE_ZERO(_m.m._01) << ", "
      << MAYBE_ZERO(_m.m._02) << ", "

      << MAYBE_ZERO(_m.m._10) << ", "
      << MAYBE_ZERO(_m.m._11) << ", "
      << MAYBE_ZERO(_m.m._12) << ", "

      << MAYBE_ZERO(_m.m._20) << ", "
      << MAYBE_ZERO(_m.m._21) << ", "
      << MAYBE_ZERO(_m.m._22) << ")";
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::generate_hash
//       Access: Public
//  Description: Adds the vector to the indicated hash generator.
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix3)::
generate_hash(ChecksumHashGenerator &hashgen, FLOATTYPE threshold) const {
  TAU_PROFILE("void LMatrix3::generate_hash(ChecksumHashGenerator &, FLOATTYPE)", " ", TAU_USER);
  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      hashgen.add_fp(get_cell(i,j), threshold);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::write_datagram
//  Description: Writes the matrix to the datagram
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix3)::
write_datagram(Datagram &destination) const {
  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      destination.add_float32(get_cell(i,j));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::read_datagram
//  Description: Reads itself out of the datagram
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix3)::
read_datagram(DatagramIterator &scan) {
  for(int i = 0; i < 3; i++) {
    for(int j = 0; j < 3; j++) {
      set_cell(i, j, scan.get_float32());
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix3)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    string name = "LMatrix3";
    name += FLOATTOKEN;
    register_type(_type_handle, name);
  }
}
