// Filename: hashVal.I
// Created by:  drose (14Nov00)
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


////////////////////////////////////////////////////////////////////
//     Function: HashVal::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE HashVal::
HashVal(void) {
  hv[0] = hv[1] = hv[2] = hv[3] = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::operator ==
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool HashVal::
operator == (const HashVal &other) const {
  return (hv[0] == other.hv[0] &&
          hv[1] == other.hv[1] &&
          hv[2] == other.hv[2] &&
          hv[3] == other.hv[3]);
}

////////////////////////////////////////////////////////////////////
//     Function: HashVal::get_value
//       Access: Public
//  Description: Returns the integer value of the indicated component.
////////////////////////////////////////////////////////////////////
INLINE uint HashVal::
get_value(int val) const {
  nassertr(val >= 0 && val < 4, 0);
  return hv[val];
}


////////////////////////////////////////////////////////////////////
//     Function: HashVal::set_value
//       Access: Public
//  Description: Sets the hash value at index val
////////////////////////////////////////////////////////////////////
INLINE void HashVal::
set_value(int val, uint hash) {
  nassertv(val >= 0 && val < 4);
  hv[val] = hash;
}


////////////////////////////////////////////////////////////////////
//     Function: HashVal::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void HashVal::
output(ostream &out) const {
  out << "[" << hv[0] << " " << hv[1] << " " << hv[2] << " " << hv[3] << "]";
}
