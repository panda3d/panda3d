// Filename: lmatrix3_src.h
// Created by:  drose (29Jan99)
// 
////////////////////////////////////////////////////////////////////

TypeHandle FLOATNAME(LMatrix3)::_type_handle;

FLOATNAME(LMatrix3) FLOATNAME(LMatrix3)::_ident_mat =
  FLOATNAME(LMatrix3)(1.0, 0.0, 0.0,
		      0.0, 1.0, 0.0,
		      0.0, 0.0, 1.0);

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
    if (!IS_THRESHOLD_EQUAL(_m.data[i], other._m.data[i], threshold)) {
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


////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::write_datagram
//  Description: Writes the matrix to the datagram
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix3)::
write_datagram(Datagram &destination) const
{
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      destination.add_float32(get_cell(i,j));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LMatrix3::read_datagram
//  Description: Reads itself out of the datagram
////////////////////////////////////////////////////////////////////
void FLOATNAME(LMatrix3)::
read_datagram(DatagramIterator &scan) 
{
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      set_cell(i, j, scan.get_float32());
    }
  }
}

