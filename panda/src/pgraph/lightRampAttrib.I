// Filename: lightRampAttrib.I
// Created by:  drose (04Mar02)
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
//     Function: LightRampAttrib::Constructor
//       Access: Private
//  Description: Use LightRampAttrib::make() to construct a new
//               LightRampAttrib object.
////////////////////////////////////////////////////////////////////
INLINE LightRampAttrib::
LightRampAttrib() {
  _mode = LRT_identity;
  _level[0] = 0.0;
  _level[1] = 0.0;
  _level[2] = 0.0;
  _threshold[0] = 0.0;
  _threshold[1] = 0.0;
}

////////////////////////////////////////////////////////////////////
//     Function: LightRampAttrib::get_mode
//       Access: Published
//  Description: Returns the LightRampAttrib mode.
////////////////////////////////////////////////////////////////////
INLINE LightRampAttrib::LightRampMode LightRampAttrib::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: LightRampAttrib::get_level
//       Access: Published
//  Description: Returns the nth lighting level.
////////////////////////////////////////////////////////////////////
INLINE float LightRampAttrib::
get_level(int n) const {
  if ((n<0)||(n>2)) return 0.0;
  return _level[n];
}

////////////////////////////////////////////////////////////////////
//     Function: LightRampAttrib::get_threshold
//       Access: Published
//  Description: Returns the nth threshold level.
////////////////////////////////////////////////////////////////////
INLINE float LightRampAttrib::
get_threshold(int n) const {
  if ((n<0)||(n>1)) return 0.0;
  return _threshold[n];
}
