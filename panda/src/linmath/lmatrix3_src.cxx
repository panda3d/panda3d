// Filename: lmatrix3_src.cxx
// Created by:  drose (29Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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


////////////////////////////////////////////////////////////////////
//     Function: LMatrix::scale_shear_mat
//       Access: Public, Static
//  Description: Returns a matrix that applies the indicated
//               scale and shear.
////////////////////////////////////////////////////////////////////
FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::
scale_shear_mat(const FLOATNAME(LVecBase3) &scale,
                const FLOATNAME(LVecBase3) &shear,
                CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }

  // We have to match the placement of the shear components in the
  // matrix to the way we extract out the rotation in
  // decompose_matrix().  Therefore, the shear is sensitive to the
  // coordinate system.

  switch (cs) {
  case CS_zup_right:
    if (temp_hpr_fix) {
      return FLOATNAME(LMatrix3)(scale._v.v._0, shear._v.v._0 * scale._v.v._0, 0.0f,
                                 0.0f, scale._v.v._1, 0.0f,
                                 shear._v.v._1 * scale._v.v._2, shear._v.v._2 * scale._v.v._2, scale._v.v._2);
    } else {
      return FLOATNAME(LMatrix3)(scale._v.v._0, 0.0f, 0.0f,
                                 shear._v.v._0 * scale._v.v._1, scale._v.v._1, 0.0f,
                                 shear._v.v._1 * scale._v.v._2, shear._v.v._2 * scale._v.v._2, scale._v.v._2);
    }

  case CS_zup_left:
    if (temp_hpr_fix) {
      return FLOATNAME(LMatrix3)(scale._v.v._0, shear._v.v._0 * scale._v.v._0, 0.0f,
                                 0.0f, scale._v.v._1, 0.0f,
                                 -shear._v.v._1 * scale._v.v._2, -shear._v.v._2 * scale._v.v._2, scale._v.v._2);
    } else {
      return FLOATNAME(LMatrix3)(scale._v.v._0, 0.0f, 0.0f,
                                 shear._v.v._0 * scale._v.v._1, scale._v.v._1, 0.0f,
                                 -shear._v.v._1 * scale._v.v._2, -shear._v.v._2 * scale._v.v._2, scale._v.v._2);
    }

  case CS_yup_right:
    if (temp_hpr_fix) {
      return FLOATNAME(LMatrix3)(scale._v.v._0, 0.0f, shear._v.v._1 * scale._v.v._0,
                                 shear._v.v._0 * scale._v.v._1, scale._v.v._1, shear._v.v._2 * scale._v.v._1,
                                 0.0f, 0.0f, scale._v.v._2);
    } else {
      return FLOATNAME(LMatrix3)(scale._v.v._0, 0.0f, 0.0f,
                                 shear._v.v._0 * scale._v.v._1, scale._v.v._1, shear._v.v._2 * scale._v.v._1,
                                 shear._v.v._1 * scale._v.v._2, 0.0f, scale._v.v._2);
    }

  case CS_yup_left:
    if (temp_hpr_fix) {
      return FLOATNAME(LMatrix3)(scale._v.v._0, 0.0f, -shear._v.v._1 * scale._v.v._0,
                                 shear._v.v._0 * scale._v.v._1, scale._v.v._1, -shear._v.v._2 * scale._v.v._1,
                                 0.0f, 0.0f, scale._v.v._2);
    } else {
      return FLOATNAME(LMatrix3)(scale._v.v._0, 0.0f, 0.0f,
                                 shear._v.v._0 * scale._v.v._1, scale._v.v._1, -shear._v.v._2 * scale._v.v._1,
                                 -shear._v.v._1 * scale._v.v._2, 0.0f, scale._v.v._2);
    }
  }

  linmath_cat.error()
    << "Invalid coordinate system value!\n";
  return _ident_mat;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::convert_mat
//       Access: Public, Static
//  Description: Returns a matrix that transforms from the indicated
//               coordinate system to the indicated coordinate system.
////////////////////////////////////////////////////////////////////
const FLOATNAME(LMatrix3) &FLOATNAME(LMatrix3)::
convert_mat(CoordinateSystem from, CoordinateSystem to) {
  if (from == CS_default) {
    from = default_coordinate_system;
  }
  if (to == CS_default) {
    to = default_coordinate_system;
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
  for (int i = 0; i < 9; i++) {
    if (!IS_THRESHOLD_COMPEQ(_m.data[i], other._m.data[i], threshold)) {
      return (_m.data[i] < other._m.data[i]) ? -1 : 1;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::almost_equal
//       Access: Public
//  Description: Returns true if two matrices are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////
bool FLOATNAME(LMatrix3)::
almost_equal(const FLOATNAME(LMatrix3) &other, FLOATTYPE threshold) const {
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

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::generate_hash
//       Access: Public
//  Description: Adds the vector to the indicated hash generator.
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix3)::
generate_hash(ChecksumHashGenerator &hashgen, FLOATTYPE threshold) const {
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
