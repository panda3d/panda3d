// Filename: eggSurface.I
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
//     Function: EggSurface::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggSurface::
EggSurface(const string &name) : EggPrimitive(name) {
  _u_subdiv = 0;
  _v_subdiv = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggSurface::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggSurface::
EggSurface(const EggSurface &copy) :
  EggPrimitive(copy),
  _u_subdiv(copy._u_subdiv),
  _v_subdiv(copy._v_subdiv)
{
}

////////////////////////////////////////////////////////////////////
//     Function: EggSurface::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE EggSurface &EggSurface::
operator = (const EggSurface &copy) {
  EggPrimitive::operator = (copy);
  _u_subdiv = copy._u_subdiv;
  _v_subdiv = copy._v_subdiv;
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: EggSurface::set_u_subdiv
//       Access: Public
//  Description: Sets the number of subdivisions in the U direction
//               that will be requested across the surface.  (This
//               doesn't necessary guarantee that this number of
//               subdivisions will be made; it's just a hint to any
//               surface renderer or quick tesselator.)  Set the
//               number to 0 to disable the hint.
////////////////////////////////////////////////////////////////////
INLINE void EggSurface::
set_u_subdiv(int subdiv) {
  _u_subdiv = subdiv;
}

////////////////////////////////////////////////////////////////////
//     Function: EggSurface::get_u_subdiv
//       Access: Public
//  Description: Returns the requested number of subdivisions in the U
//               direction, or 0 if no particular subdivisions have
//               been requested.
////////////////////////////////////////////////////////////////////
INLINE int EggSurface::
get_u_subdiv() const {
  return _u_subdiv;
}

////////////////////////////////////////////////////////////////////
//     Function: EggSurface::set_v_subdiv
//       Access: Public
//  Description: Sets the number of subdivisions in the U direction
//               that will be requested across the surface.  (This
//               doesn't necessary guarantee that this number of
//               subdivisions will be made; it's just a hint to any
//               surface renderer or quick tesselator.)  Set the
//               number to 0 to disable the hint.
////////////////////////////////////////////////////////////////////
INLINE void EggSurface::
set_v_subdiv(int subdiv) {
  _v_subdiv = subdiv;
}

////////////////////////////////////////////////////////////////////
//     Function: EggSurface::get_v_subdiv
//       Access: Public
//  Description: Returns the requested number of subdivisions in the U
//               direction, or 0 if no particular subdivisions have
//               been requested.
////////////////////////////////////////////////////////////////////
INLINE int EggSurface::
get_v_subdiv() const {
  return _v_subdiv;
}
