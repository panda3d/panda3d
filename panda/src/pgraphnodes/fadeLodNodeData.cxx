// Filename: fadeLodNodeData.cxx
// Created by:  drose (29Sep04)
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

#include "fadeLodNodeData.h"

TypeHandle FadeLODNodeData::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: FadeLODNodeData::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void FadeLODNodeData::
output(ostream &out) const {
  AuxSceneData::output(out);
  if (_fade_mode != FM_solid) {
    out << " fading " << _fade_out << " to " << _fade_in << " since "
        << _fade_start;
  } else {
    out << " showing " << _fade_in;
  }
}
