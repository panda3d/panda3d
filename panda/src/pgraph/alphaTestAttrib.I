// Filename: alphaTestAttrib.I
// Created by:  drose (04Mar02)
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


////////////////////////////////////////////////////////////////////
//     Function: AlphaTestAttrib::Constructor
//       Access: Private
//  Description: Use AlphaTestAttrib::make() to construct a new
//               AlphaTestAttrib object.
////////////////////////////////////////////////////////////////////
INLINE AlphaTestAttrib::
AlphaTestAttrib(AlphaTestAttrib::PandaCompareFunc mode,
                PN_stdfloat reference_alpha) :
  _mode(mode), _reference_alpha(reference_alpha)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTestAttrib::get_mode
//       Access: Published
//  Description: Returns the alpha write mode.
////////////////////////////////////////////////////////////////////
INLINE AlphaTestAttrib::PandaCompareFunc AlphaTestAttrib::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: AlphaTestAttrib::get_reference_alpha
//       Access: Published
//  Description: Returns the alpha reference value.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat AlphaTestAttrib::
get_reference_alpha() const {
  return _reference_alpha;
}
