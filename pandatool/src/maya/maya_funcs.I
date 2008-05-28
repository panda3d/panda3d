// Filename: maya_funcs.I
// Created by:  drose (15Apr02)
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
//     Function: MString output operator
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ostream &operator << (ostream &out, const MString &str) {
  return out << str.asChar();
}

////////////////////////////////////////////////////////////////////
//     Function: MVector output operator
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ostream &operator << (ostream &out, const MVector &vec) {
  return out << vec.x << " " << vec.y << " " << vec.z;
}
