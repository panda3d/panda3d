// Filename: lmatrix4_src.cxx
// Created by:  drose (15Jan99)
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

TypeHandle FLOATNAME(LMatrix4)::_type_handle;

const FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::_ident_mat =
  FLOATNAME(LMatrix4)(1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);

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

////////////////////////////////////////////////////////////////////
//     Function: LMatrix::convert_mat
//       Access: Public, Static
//  Description: Returns a matrix that transforms from the indicated
//               coordinate system to the indicated coordinate system.
////////////////////////////////////////////////////////////////////
const FLOATNAME(LMatrix4) &FLOATNAME(LMatrix4)::
convert_mat(CoordinateSystem from, CoordinateSystem to) {
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
//     Function: LMatrix4::compare_to
//       Access: Public
//  Description: Sorts matrices lexicographically, componentwise.
//               Returns a number less than 0 if this matrix sorts
//               before the other one, greater than zero if it sorts
//               after, 0 if they are equivalent (within the indicated
//               tolerance).
////////////////////////////////////////////////////////////////////
int FLOATNAME(LMatrix4)::
compare_to(const FLOATNAME(LMatrix4) &other, FLOATTYPE threshold) const {
  // We compare values in reverse order, since the last row of the
  // matrix is most likely to be different between different matrices.
  for (int i = 15; i >= 0; i--) {
    if (!IS_THRESHOLD_COMPEQ(_m.data[i], other._m.data[i], threshold)) {
      return (_m.data[i] < other._m.data[i]) ? -1 : 1;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::almost_equal
//       Access: Public
//  Description: Returns true if two matrices are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////
bool FLOATNAME(LMatrix4)::
almost_equal(const FLOATNAME(LMatrix4) &other, FLOATTYPE threshold) const {
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
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix4)::
output(ostream &out) const {
  out << "[ "
      << MAYBE_ZERO(_m.m._00) << " "
      << MAYBE_ZERO(_m.m._01) << " "
      << MAYBE_ZERO(_m.m._02) << " "
      << MAYBE_ZERO(_m.m._03)
      << " ] [ "
      << MAYBE_ZERO(_m.m._10) << " "
      << MAYBE_ZERO(_m.m._11) << " "
      << MAYBE_ZERO(_m.m._12) << " "
      << MAYBE_ZERO(_m.m._13)
      << " ] [ "
      << MAYBE_ZERO(_m.m._20) << " "
      << MAYBE_ZERO(_m.m._21) << " "
      << MAYBE_ZERO(_m.m._22) << " "
      << MAYBE_ZERO(_m.m._23)
      << " ] [ "
      << MAYBE_ZERO(_m.m._30) << " "
      << MAYBE_ZERO(_m.m._31) << " "
      << MAYBE_ZERO(_m.m._32) << " "
      << MAYBE_ZERO(_m.m._33)
      << " ]";
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::python_repr
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void FLOATNAME(LMatrix4)::
python_repr(ostream &out, const string &class_name) const {
  out << class_name << "(" 
      << MAYBE_ZERO(_m.m._00) << ", "
      << MAYBE_ZERO(_m.m._01) << ", "
      << MAYBE_ZERO(_m.m._02) << ", "
      << MAYBE_ZERO(_m.m._03) << ", "

      << MAYBE_ZERO(_m.m._10) << ", "
      << MAYBE_ZERO(_m.m._11) << ", "
      << MAYBE_ZERO(_m.m._12) << ", "
      << MAYBE_ZERO(_m.m._13) << ", "

      << MAYBE_ZERO(_m.m._20) << ", "
      << MAYBE_ZERO(_m.m._21) << ", "
      << MAYBE_ZERO(_m.m._22) << ", "
      << MAYBE_ZERO(_m.m._23) << ", "

      << MAYBE_ZERO(_m.m._30) << ", "
      << MAYBE_ZERO(_m.m._31) << ", "
      << MAYBE_ZERO(_m.m._32) << ", "
      << MAYBE_ZERO(_m.m._33) << ")";
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix4)::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << MAYBE_ZERO(_m.m._00) << " "
    << MAYBE_ZERO(_m.m._01) << " "
    << MAYBE_ZERO(_m.m._02) << " "
    << MAYBE_ZERO(_m.m._03)
    << "\n";
  indent(out, indent_level)
    << MAYBE_ZERO(_m.m._10) << " "
    << MAYBE_ZERO(_m.m._11) << " "
    << MAYBE_ZERO(_m.m._12) << " "
    << MAYBE_ZERO(_m.m._13)
    << "\n";
  indent(out, indent_level)
    << MAYBE_ZERO(_m.m._20) << " "
    << MAYBE_ZERO(_m.m._21) << " "
    << MAYBE_ZERO(_m.m._22) << " "
    << MAYBE_ZERO(_m.m._23)
    << "\n";
  indent(out, indent_level)
    << MAYBE_ZERO(_m.m._30) << " "
    << MAYBE_ZERO(_m.m._31) << " "
    << MAYBE_ZERO(_m.m._32) << " "
    << MAYBE_ZERO(_m.m._33)
    << "\n";
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::generate_hash
//       Access: Public
//  Description: Adds the vector to the indicated hash generator.
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix4)::
generate_hash(ChecksumHashGenerator &hashgen, FLOATTYPE threshold) const {
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      hashgen.add_fp(get_cell(i,j), threshold);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::decompose_mat
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
bool FLOATNAME(LMatrix4)::
decompose_mat(int index[4]) {
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

    // We throw the value out only if it's smaller than our "small"
    // threshold squared.  This helps reduce overly-sensitive
    // rejections.
    if (IS_THRESHOLD_ZERO(big, (NEARLY_ZERO(FLOATTYPE) * NEARLY_ZERO(FLOATTYPE)))) {
      //    if (IS_NEARLY_ZERO(big)) {
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

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::back_sub_mat
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
bool FLOATNAME(LMatrix4)::
back_sub_mat(int index[4], FLOATNAME(LMatrix4) &inv, int row) const {
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

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::write_datagram
//  Description: Writes the matrix to the datagram
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix4)::
write_datagram(Datagram &destination) const {
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      destination.add_float32(get_cell(i,j));
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::read_datagram
//  Description: Reads itself out of the datagram
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix4)::
read_datagram(DatagramIterator &scan) {
  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      set_cell(i, j, scan.get_float32());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix4)::
init_type() {
  if (_type_handle == TypeHandle::none()) {
    // Format a string to describe the type.
    string name = "LMatrix4";
    name += FLOATTOKEN;
    register_type(_type_handle, name);
  }
}
