// Filename: cLerpInterval.I
// Created by:  drose (27Aug02)
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
//     Function: CLerpInterval::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CLerpInterval::
CLerpInterval(const string &name, double duration, 
              CLerpInterval::BlendType blend_type) :
  CInterval(name, duration, true),
  _blend_type(blend_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CLerpInterval::get_blend_type
//       Access: Published
//  Description: Returns the blend type specified for the interval.
//               This controls how the linear interpolation behaves
//               near the beginning and end of the lerp period.
////////////////////////////////////////////////////////////////////
INLINE CLerpInterval::BlendType CLerpInterval::
get_blend_type() const {
  return _blend_type;
}
