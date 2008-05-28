// Filename: eggXfmAnimData.I
// Created by:  drose (19Feb99)
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


////////////////////////////////////////////////////////////////////
//     Function: EggXfmAnimData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggXfmAnimData::
EggXfmAnimData(const string &name, CoordinateSystem cs) : EggAnimData(name) {
  _coordsys = cs;
}


////////////////////////////////////////////////////////////////////
//     Function: EggXfmAnimData::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggXfmAnimData::
EggXfmAnimData(const EggXfmAnimData &copy)
  : EggAnimData(copy),
    _order(copy._order),
    _contents(copy._contents),
    _coordsys(copy._coordsys) {
}


////////////////////////////////////////////////////////////////////
//     Function: EggXfmAnimData::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggXfmAnimData &EggXfmAnimData::
operator = (const EggXfmAnimData &copy) {
  EggAnimData::operator = (copy);
  _order = copy._order;
  _contents = copy._contents;
  _coordsys = copy._coordsys;

  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: EggXfmAnimData::set_order
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggXfmAnimData::
set_order(const string &order) {
  _order = order;
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmAnimData::clear_order
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggXfmAnimData::
clear_order() {
  _order = "";
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmAnimData::has_order
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggXfmAnimData::
has_order() const {
  return !_order.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmAnimData::get_order
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &EggXfmAnimData::
get_order() const {
  if (has_order()) {
    return _order;
  } else {
    return get_standard_order();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmAnimData::get_standard_order
//       Access: Public, Static
//  Description: Returns the standard order of matrix component
//               composition.  This is what the order string must be
//               set to in order to use set_value() or add_data()
//               successfully.
////////////////////////////////////////////////////////////////////
INLINE const string &EggXfmAnimData::
get_standard_order() {
  return EggXfmSAnim::get_standard_order();
}


////////////////////////////////////////////////////////////////////
//     Function: EggXfmAnimData::set_contents
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggXfmAnimData::
set_contents(const string &contents) {
  _contents = contents;
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmAnimData::clear_contents
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggXfmAnimData::
clear_contents() {
  _contents = "";
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmAnimData::has_contents
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggXfmAnimData::
has_contents() const {
  return !_contents.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmAnimData::get_contents
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const string &EggXfmAnimData::
get_contents() const {
  return _contents;
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmAnimData::get_coordinate_system
//       Access: Public
//  Description: Returns the coordinate system this table believes it
//               is defined within.  This should always match the
//               coordinate system of the EggData structure that owns
//               it.  It is necessary to store it here because the
//               meaning of the h, p, and r columns depends on the
//               coordinate system.
////////////////////////////////////////////////////////////////////
INLINE CoordinateSystem EggXfmAnimData::
get_coordinate_system() const {
  return _coordsys;
}


////////////////////////////////////////////////////////////////////
//     Function: EggXfmAnimData::get_num_rows
//       Access: Public
//  Description: Returns the number of rows in the table.
////////////////////////////////////////////////////////////////////
INLINE int EggXfmAnimData::
get_num_rows() const {
  if (get_num_cols() == 0) {
    return 0;
  }
  return get_size() / get_num_cols();
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmAnimData::get_num_cols
//       Access: Public
//  Description: Returns the number of columns in the table.  This is
//               set according to the "contents" string, which defines
//               the meaning of each column.
////////////////////////////////////////////////////////////////////
INLINE int EggXfmAnimData::
get_num_cols() const {
  return _contents.length();
}


////////////////////////////////////////////////////////////////////
//     Function: EggXfmAnimData::get_value
//       Access: Public
//  Description: Returns the value at the indicated row.  Row must be
//               in the range 0 <= row < get_num_rows(); col must be
//               in the range 0 <= col < get_num_cols().
////////////////////////////////////////////////////////////////////
INLINE double EggXfmAnimData::
get_value(int row, int col) const {
  nassertr(get_num_cols() != 0, 0.0);
  nassertr(row >= 0 && row < get_num_rows(), 0.0);
  nassertr(col >= 0 && col < get_num_cols(), 0.0);
  return _data[row * get_num_cols() + col];
}


