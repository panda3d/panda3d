// Filename: showBoundsEffect.I
// Created by:  drose (25Mar02)
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
//     Function: ShowBoundsEffect::Constructor
//       Access: Private
//  Description: Use ShowBoundsEffect::make() to construct a new
//               ShowBoundsEffect object.
////////////////////////////////////////////////////////////////////
INLINE ShowBoundsEffect::
ShowBoundsEffect() :
  _tight(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ShowBoundsEffect::get_tight
//       Access: Published
//  Description: Returns true if the "tight" flag was set, meaning the
//               effect should compute and draw the tight bounding-box
//               of the node's vertices every frame.
////////////////////////////////////////////////////////////////////
INLINE bool ShowBoundsEffect::
get_tight() const {
  return _tight;
}
