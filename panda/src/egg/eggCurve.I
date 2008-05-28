// Filename: eggCurve.I
// Created by:  drose (15Feb00)
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
//     Function: EggCurve::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggCurve::
EggCurve(const string &name) : EggPrimitive(name) {
  _subdiv = 0;
  _type = CT_none;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCurve::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggCurve::
EggCurve(const EggCurve &copy) :
  EggPrimitive(copy),
  _subdiv(copy._subdiv),
  _type(copy._type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggCurve::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggCurve &EggCurve::
operator = (const EggCurve &copy) {
  EggPrimitive::operator = (copy);
  _subdiv = copy._subdiv;
  _type = copy._type;
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: EggCurve::set_subdiv
//       Access: Public
//  Description: Sets the number of subdivisions that will be
//               requested across the curve.  (This doesn't necessary
//               guarantee that this number of subdivisions will be
//               made; it's just a hint to any curve renderer or quick
//               tesselator.)  Set the number to 0 to disable the
//               hint.
////////////////////////////////////////////////////////////////////
INLINE void EggCurve::
set_subdiv(int subdiv) {
  _subdiv = subdiv;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCurve::get_subdiv
//       Access: Public
//  Description: Returns the requested number of subdivisions, or 0 if
//               no particular subdivisions have been requested.
////////////////////////////////////////////////////////////////////
INLINE int EggCurve::
get_subdiv() const {
  return _subdiv;
}


////////////////////////////////////////////////////////////////////
//     Function: EggCurve::set_curve_type
//       Access: Public
//  Description: Sets the type of the curve.  This is primarily used
//               as a hint to any code that may need to deal with this
//               curve.
////////////////////////////////////////////////////////////////////
INLINE void EggCurve::
set_curve_type(EggCurve::CurveType type) {
  _type = type;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCurve::get_curve_type
//       Access: Public
//  Description: Returns the indicated type of the curve.
////////////////////////////////////////////////////////////////////
INLINE EggCurve::CurveType EggCurve::
get_curve_type() const {
  return _type;
}
