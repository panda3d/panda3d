// Filename: showBoundsEffect.I
// Created by:  drose (25Mar02)
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
