// Filename: eggNamedObject.I
// Created by:  drose (10Feb99)
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
//     Function: EggNamedObject::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggNamedObject::
EggNamedObject(const string &name) : Namable(name) {
}


////////////////////////////////////////////////////////////////////
//     Function: EggNamedObject::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggNamedObject::
EggNamedObject(const EggNamedObject &copy) : EggObject(copy), Namable(copy) {
}


////////////////////////////////////////////////////////////////////
//     Function: EggNamedObject::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggNamedObject &EggNamedObject::
operator = (const EggNamedObject &copy) {
  EggObject::operator = (copy);
  Namable::operator = (copy);
  return *this;
}

INLINE ostream &operator << (ostream &out, const EggNamedObject &n) {
  n.output(out);
  return out;
}
