// Filename: lmatrix4_src.I
// Created by:  drose (15Jan99)
// 
////////////////////////////////////////////////////////////////////

TypeHandle FLOATNAME(LMatrix4)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::Equality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////

bool FLOATNAME(LMatrix4)::
operator == (const FLOATNAME(LMatrix4) &other) const {
  return ((*this)(0, 0) == other(0, 0) &&
	  (*this)(0, 1) == other(0, 1) &&
	  (*this)(0, 2) == other(0, 2) &&
	  (*this)(0, 3) == other(0, 3) &&
	  (*this)(1, 0) == other(1, 0) &&
	  (*this)(1, 1) == other(1, 1) &&
	  (*this)(1, 2) == other(1, 2) &&
	  (*this)(1, 3) == other(1, 3) &&
	  (*this)(2, 0) == other(2, 0) &&
	  (*this)(2, 1) == other(2, 1) &&
	  (*this)(2, 2) == other(2, 2) &&
	  (*this)(2, 3) == other(2, 3) &&
	  (*this)(3, 0) == other(3, 0) &&
	  (*this)(3, 1) == other(3, 1) &&
	  (*this)(3, 2) == other(3, 2) &&
	  (*this)(3, 3) == other(3, 3));
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix::convert_mat
//       Access: Public, Static
//  Description: Returns a matrix that transforms from the indicated
//               coordinate system to the indicated coordinate system.
////////////////////////////////////////////////////////////////////

FLOATNAME(LMatrix4) FLOATNAME(LMatrix4)::
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
    case CS_zup_left: return ident_mat();
    case CS_yup_left: return z_to_y_up_mat();
    case CS_zup_right: return scale_mat(1.0, -1.0, 1.0);
    case CS_yup_right: return scale_mat(1.0, -1.0, 1.0) * z_to_y_up_mat();
    default: break;
    }
    break;

  case CS_yup_left:
    switch (to) {
    case CS_zup_left: return y_to_z_up_mat();
    case CS_yup_left: return ident_mat();
    case CS_zup_right: return scale_mat(1.0, 1.0, -1.0) * y_to_z_up_mat();
    case CS_yup_right: return scale_mat(1.0, 1.0, -1.0);
    default: break;
    }
    break;

  case CS_zup_right:
    switch (to) {
    case CS_zup_left: return scale_mat(1.0, -1.0, 1.0);
    case CS_yup_left: return scale_mat(1.0, -1.0, 1.0) * z_to_y_up_mat();
    case CS_zup_right: return ident_mat();
    case CS_yup_right: return z_to_y_up_mat();
    default: break;
    }
    break;

  case CS_yup_right:
    switch (to) {
    case CS_zup_left: return scale_mat(1.0, 1.0, -1.0) * y_to_z_up_mat();
    case CS_yup_left: return scale_mat(1.0, 1.0, -1.0);
    case CS_zup_right: return y_to_z_up_mat();
    case CS_yup_right: return ident_mat();
    default: break;
    }
    break;

  default:
    break;
  }

  linmath_cat.error()
    << "Invalid coordinate system value!\n";
  return ident_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::almost_equal
//       Access: Public
//  Description: Returns true if two matrices are memberwise equal
//               within a specified tolerance.
////////////////////////////////////////////////////////////////////

bool FLOATNAME(LMatrix4)::
almost_equal(const FLOATNAME(LMatrix4) &other, FLOATTYPE1 threshold) const {
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
//     Function: LMatrix4::decompose_mat
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////

bool FLOATNAME(LMatrix4)::
decompose_mat(int index[4]) {
  int i, j, k;
  FLOATTYPE1 vv[4];
  for (i = 0; i < 4; i++) {
    FLOATTYPE1 big = 0.0;
    for (j = 0; j < 4; j++) {
      FLOATTYPE1 temp = fabs((*this)(i,j));
      if (temp > big) {
	big = temp;
      }
    }

    if (IS_NEARLY_ZERO(big)) {
      return false;
    }
    vv[i] = 1.0 / big;
  }

  for (j = 0; j < 4; j++) {
    for (i = 0; i < j; i++) {
      FLOATTYPE1 sum = (*this)(i,j);
      for (k = 0; k < i; k++) {
	sum -= (*this)(i,k) * (*this)(k,j);
      }
      (*this)(i,j) = sum;
    }

    FLOATTYPE1 big = 0.0;
    int imax = -1;
    for (i = j; i < 4; i++) {
      FLOATTYPE1 sum = (*this)(i,j);
      for (k = 0; k < j; k++) {
	sum -= (*this)(i,k) * (*this)(k,j);
      }
      (*this)(i,j) = sum;

      FLOATTYPE1 dum = vv[i] * fabs(sum);
      if (dum >= big) {
	big = dum;
	imax = i;
      }
    }
    nassertr(imax >= 0, false);
    if (j != imax) {
      for (k = 0; k < 4; k++) {
	FLOATTYPE1 dum = (*this)(imax,k);
	(*this)(imax,k) = (*this)(j,k);
	(*this)(j,k) = dum;
      }
      vv[imax] = vv[j];
    }
    index[j] = imax;

    if ((*this)(j,j) == 0.0) {
      (*this)(j,j) = NEARLY_ZERO(FLOATTYPE1);
    }

    if (j != 4 - 1) {
      FLOATTYPE1 dum = 1.0 / (*this)(j,j);
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
    FLOATTYPE1 sum = inv(row, ip);
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
    FLOATTYPE1 sum = inv(row, i);
    for (j = i + 1; j < 4; j++) {
      sum -= (*this)(i,j) * inv(row, j);
    }
    inv(row, i) = sum / (*this)(i,i);
  }

  return true;
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
    do_init_type(FLOATTYPE1);
    string name =
      "LMatrix4<" + get_type_handle(FLOATTYPE1).get_name() + ">";
    register_type(_type_handle, name);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::write_datagram
//  Description: Writes the matrix to the datagram
////////////////////////////////////////////////////////////////////

void FLOATNAME(LMatrix4)::
write_datagram(Datagram &destination) const
{
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      destination.add_float32(get_cell(i,j));
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: LMatrix4::read_datagram
//  Description: Reads itself out of the datagram
////////////////////////////////////////////////////////////////////

void FLOATNAME(LMatrix4)::
read_datagram(DatagramIterator &scan) 
{
  for(int i = 0; i < 4; i++)
  {
    for(int j = 0; j < 4; j++)
    {
      set_cell(i, j, scan.get_float32());
    }
  }
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
compare_to(const FLOATNAME(LMatrix4) &other, FLOATTYPE1 threshold) const {
  for (int i = 0; i < 16; i++) {
    if (!IS_THRESHOLD_EQUAL(_data[i], other._data[i], threshold)) {
      return (_data[i] < other._data[i]) ? -1 : 1;
    }
  }
  return 0;
}
