// Filename: eggLoader.I
// Created by:  drose (13Mar05)
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
//     Function: EggLoader::PrimitiveUnifier::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggLoader::PrimitiveUnifier::
PrimitiveUnifier(const GeomPrimitive *prim) :
  _type(prim->get_type()),
  _shade_model(prim->get_shade_model())
{
}


////////////////////////////////////////////////////////////////////
//     Function: EggLoader::PrimitiveUnifier::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool EggLoader::PrimitiveUnifier::
operator < (const PrimitiveUnifier &other) const {
  if (_type != other._type) {
    return _type < other._type;
  }
  return _shade_model < other._shade_model;
}
