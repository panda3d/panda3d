// Filename: findApproxLevel.cxx
// Created by:  drose (13Mar02)
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

#include "findApproxLevel.h"

////////////////////////////////////////////////////////////////////
//     Function: FindApproxLevel::write
//       Access: Public
//  Description: Shows the entire contents of the level, one entry per
//               line.  For debugging only.
////////////////////////////////////////////////////////////////////
void FindApproxLevel::
write(ostream &out) const {
  Vec::const_iterator vi;
  for (vi = _v.begin(); vi != _v.end(); ++vi) {
    (*vi).output(out);
    out << "\n";
  }
}
