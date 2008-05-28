// Filename: auxSceneData.cxx
// Created by:  drose (27Sep04)
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

#include "auxSceneData.h"
#include "indent.h"

TypeHandle AuxSceneData::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AuxSceneData::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AuxSceneData::
output(ostream &out) const {
  out << get_type() << " expires " << get_expiration_time();
}

////////////////////////////////////////////////////////////////////
//     Function: AuxSceneData::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AuxSceneData::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}
