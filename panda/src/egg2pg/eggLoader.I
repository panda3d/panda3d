// Filename: eggLoader.I
// Created by:  drose (13Mar05)
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
