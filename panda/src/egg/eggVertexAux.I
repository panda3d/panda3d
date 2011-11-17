// Filename: eggVertexAux.I
// Created by:  jenes (15Nov11)
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
//     Function: EggVertexAux::set_name
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void EggVertexAux::
set_name(const string &name) {
  Namable::set_name(name);
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexAux::get_aux
//       Access: Published
//  Description: Returns the auxiliary data quadruple.
////////////////////////////////////////////////////////////////////
INLINE const LVecBase4d &EggVertexAux::
get_aux() const {
  return _aux;
}

////////////////////////////////////////////////////////////////////
//     Function: EggVertexAux::set_aux
//       Access: Published
//  Description: Sets the auxiliary data quadruple.
////////////////////////////////////////////////////////////////////
INLINE void EggVertexAux::
set_aux(const LVecBase4d &aux) {
  _aux = aux;
}
