// Filename: pgButtonEvent.cxx
// Created by:  drose (05Jul01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "pgButtonEvent.h"

TypeHandle PGButtonEvent::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PGButtonEvent::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PGButtonEvent::
~PGButtonEvent() {
}

////////////////////////////////////////////////////////////////////
//     Function: PGButtonEvent::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PGButtonEvent::
output(ostream &out) const {
  out << _button << " at (" << _mouse_x << ", " << _mouse_y << ")";
}
