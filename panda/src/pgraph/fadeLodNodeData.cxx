// Filename: fadeLodNodeData.cxx
// Created by:  drose (29Sep04)
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
  if (_fade_mode) {
    out << " fading " << _fade_out << " to " << _fade_in << " since "
        << _fade_start;
  } else {
    out << " showing " << _fade_in;
  }
}
