// Filename: colladaPrimitive.I
// Created by:  rdb (23May11)
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
//     Function: ColladaPrimitive::add_input
//  Description: Adds a new ColladaInput to this primitive.
////////////////////////////////////////////////////////////////////
INLINE void ColladaPrimitive::
add_input(ColladaInput *input) {
  if (input->get_offset() >= _stride) {
    _stride = input->get_offset() + 1;
  }
  _inputs.push_back(input);
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaPrimitive::get_geom
//  Description: Returns the Geom associated with this primitive.
////////////////////////////////////////////////////////////////////
INLINE PT(Geom) ColladaPrimitive::
get_geom() const {
  return _geom;
}

////////////////////////////////////////////////////////////////////
//     Function: ColladaPrimitive::get_material
//  Description: Returns the name of this primitive's material, or
//               the empty string if none was assigned.
////////////////////////////////////////////////////////////////////
INLINE const string &ColladaPrimitive::
get_material() const {
  return _material;
}
