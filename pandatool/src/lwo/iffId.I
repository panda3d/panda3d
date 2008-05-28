// Filename: iffId.I
// Created by:  drose (23Apr01)
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
//     Function: IffId::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IffId::
IffId() {
  _id._c[0] = 0;
  _id._c[1] = 0;
  _id._c[2] = 0;
  _id._c[3] = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: IffId::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IffId::
IffId(const char id[4]) {
  _id._c[0] = id[0];
  _id._c[1] = id[1];
  _id._c[2] = id[2];
  _id._c[3] = id[3];
}

////////////////////////////////////////////////////////////////////
//     Function: IffId::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IffId::
IffId(const IffId &copy) {
  _id._n = copy._id._n;
}

////////////////////////////////////////////////////////////////////
//     Function: IffId::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void IffId::
operator = (const IffId &copy) {
  _id._n = copy._id._n;
}

////////////////////////////////////////////////////////////////////
//     Function: IffId::Equivalence Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool IffId::
operator == (const IffId &other) const {
  return (_id._n == other._id._n);
}

////////////////////////////////////////////////////////////////////
//     Function: IffId::Nonequivalence Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool IffId::
operator != (const IffId &other) const {
  return (_id._n != other._id._n);
}

////////////////////////////////////////////////////////////////////
//     Function: IffId::Ordering Operator
//       Access: Public
//  Description: The ordering is arbitrary, and may not even be
//               consistent between different architectures
//               (e.g. big-endian and little-endian).  It is useful
//               mainly for putting IffId's into a sorted container,
//               like sets and maps.
////////////////////////////////////////////////////////////////////
INLINE bool IffId::
operator < (const IffId &other) const {
  return (_id._n < other._id._n);
}

////////////////////////////////////////////////////////////////////
//     Function: IffId::get_name
//       Access: Public
//  Description: Returns the four-character name of the Id, for
//               outputting.
////////////////////////////////////////////////////////////////////
INLINE string IffId::
get_name() const {
  return string(_id._c, 4);
}
