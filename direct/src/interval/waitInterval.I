// Filename: waitInterval.I
// Created by:  drose (12Sep02)
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
//     Function: WaitInterval::Constructor
//       Access: Published
//  Description: All Wait intervals have the same name.  No one really
//               cares if their names are unique, after all.
////////////////////////////////////////////////////////////////////
INLINE WaitInterval::
WaitInterval(double duration) :
  CInterval("Wait", duration, true)
{
}
