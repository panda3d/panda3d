// Filename: maya_funcs.I
// Created by:  drose (15Apr02)
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
