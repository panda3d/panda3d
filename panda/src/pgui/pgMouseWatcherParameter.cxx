// Filename: pgMouseWatcherParameter.cxx
// Created by:  drose (05Jul01)
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

#include "pgMouseWatcherParameter.h"

TypeHandle PGMouseWatcherParameter::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherParameter::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PGMouseWatcherParameter::
~PGMouseWatcherParameter() {
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherParameter::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PGMouseWatcherParameter::
output(ostream &out) const {
  MouseWatcherParameter::output(out);
}
