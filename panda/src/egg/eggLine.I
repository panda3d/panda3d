// Filename: eggLine.I
// Created by:  drose (14Oct03)
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
//     Function: EggLine::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggLine::
EggLine(const string &name) : 
  EggCompositePrimitive(name),
  _has_thick(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggLine::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggLine::
EggLine(const EggLine &copy) : 
  EggCompositePrimitive(copy),
  _thick(copy._thick),
  _has_thick(copy._has_thick)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggLine::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggLine &EggLine::
operator = (const EggLine &copy) {
  EggCompositePrimitive::operator = (copy);
  _thick = copy._thick;
  _has_thick = copy._has_thick;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLine::has_thick
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggLine::
has_thick() const {
  return _has_thick;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLine::get_thick
//       Access: Published
//  Description: Returns the thick set on this particular attribute.
//               If there is no thick set, returns white.
////////////////////////////////////////////////////////////////////
INLINE double EggLine::
get_thick() const {
  if (has_thick()) {
    return _thick;
  } else {
    return 1.0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggLine::
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggLine::
set_thick(double thick) {
  _thick = thick;
  _has_thick = true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggLine::
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggLine::
clear_thick() {
  _has_thick = false;
}
