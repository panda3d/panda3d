// Filename: nurbsCurveDrawer.I
// Created by:  drose (05Mar01)
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
//     Function: NurbsCurveDrawer::hilight
//       Access: Published
//  Description: Hilights a particular CV by showing it and its knot
//               in a different color.  Returns true if the CV exists
//               and has been drawn, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool NurbsCurveDrawer::
hilight(int n) {
  return hilight(n, 1.0f, 1.0f, 0.0f);
}

