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
  _flags(0),
  _thick(1.0)
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
  _flags(copy._flags),
  _thick(copy._thick)
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
  _flags = copy._flags;
  _thick = copy._thick;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPoint::has_thick
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggPoint::
has_thick() const {
  return (_flags & F_has_thick) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPoint::get_thick
//       Access: Published
//  Description: Returns the thickness set on this particular point.
//               If there is no thickness set, returns 1.0.
////////////////////////////////////////////////////////////////////
INLINE double EggPoint::
get_thick() const {
  return _thick;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPoint::set_thick
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggPoint::
set_thick(double thick) {
  _thick = thick;
  _flags |= F_has_thick;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPoint::clear_thick
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggPoint::
clear_thick() {
  _thick = 1.0;
  _flags &= ~F_has_thick;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPoint::has_perspective
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool EggPoint::
has_perspective() const {
  return (_flags & F_has_perspective) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPoint::get_perspective
//       Access: Published
//  Description: Returns the perspective flag set on this particular
//               point.  If there is no perspective flag set, returns
//               false.
////////////////////////////////////////////////////////////////////
INLINE bool EggPoint::
get_perspective() const {
  return (_flags & F_perspective) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPoint::set_perspective
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggPoint::
set_perspective(bool perspective) {
  if (perspective) {
    _flags |= F_perspective;
  } else {
    _flags &= ~F_perspective;
  }
  _flags |= F_has_perspective;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPoint::clear_perspective
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggPoint::
clear_perspective() {
  _flags &= ~(F_has_perspective | F_perspective);
}
