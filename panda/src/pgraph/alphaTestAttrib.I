// Filename: alphaTestAttrib.I
// Created by:  drose (04Mar02)
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


////////////////////////////////////////////////////////////////////
//     Function: AlphaTestAttrib::Constructor
//       Access: Private
//  Description: Use AlphaTestAttrib::make() to construct a new
//               AlphaTestAttrib object.
////////////////////////////////////////////////////////////////////
INLINE AlphaTestAttrib::
AlphaTestAttrib(AlphaTestAttrib::Mode mode,unsigned int reference_alpha) :
  _mode(mode), _reference_alpha(reference_alpha)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTestAttrib::get_mode
//       Access: Published
//  Description: Returns the alpha write mode.
////////////////////////////////////////////////////////////////////
INLINE AlphaTestAttrib::Mode AlphaTestAttrib::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTestAttrib::get_mode
//       Access: Published
//  Description: Returns the alpha reference value.
////////////////////////////////////////////////////////////////////
INLINE unsigned int AlphaTestAttrib::
get_reference_alpha(void) const {
  return _reference_alpha;
}
