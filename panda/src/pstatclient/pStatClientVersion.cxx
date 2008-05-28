// Filename: pStatClientVersion.cxx
// Created by:  drose (21May01)
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

#include "pStatClientVersion.h"
#include "pStatProperties.h"


////////////////////////////////////////////////////////////////////
//     Function: PStatClientVersion::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PStatClientVersion::
PStatClientVersion() {
  _major_version = get_current_pstat_major_version();
  _minor_version = get_current_pstat_minor_version();
}
