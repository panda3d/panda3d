// Filename: eggPoint.I
// Created by:  drose (15Dec99)
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
//     Function: EggPoint::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggPoint::
EggPoint(const string &name) : 
  EggPrimitive(name),
  _has_thick(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggPoint::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggPoint::
EggPoint(const EggPoint &copy) : 
  EggPrimitive(copy),
  _thick(copy._thick),
  _has_thick(copy._has_thick)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggPoint::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggPoint &EggPoint::
operator = (const EggPoint &copy) {
  EggPrimitive::operator = (copy);
  _thick = copy._thick;
  _has_thick = copy._has_thick;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPoint::has_thick
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggPoint::
has_thick() const {
  return _has_thick;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPoint::get_thick
//       Access: Published
//  Description: Returns the thick set on this particular attribute.
//               If there is no thick set, returns white.
////////////////////////////////////////////////////////////////////
INLINE double EggPoint::
get_thick() const {
  if (has_thick()) {
    return _thick;
  } else {
    return 1.0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggPoint::
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggPoint::
set_thick(double thick) {
  _thick = thick;
  _has_thick = true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPoint::
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggPoint::
clear_thick() {
  _has_thick = false;
}
