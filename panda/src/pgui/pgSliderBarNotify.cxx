// Filename: pgSliderBarNotify.cxx
// Created by:  drose (18Aug05)
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

#include "pgSliderBarNotify.h"
#include "pgSliderBar.h"

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBarNotify::slider_bar_adjust
//       Access: Protected, Virtual
//  Description: Called whenever a watched PGSliderBar's value
//               has been changed by the user or programmatically.
////////////////////////////////////////////////////////////////////
void PGSliderBarNotify::
slider_bar_adjust(PGSliderBar *) {
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderBarNotify::slider_bar_set_range
//       Access: Protected, Virtual
//  Description: Called whenever a watched PGSliderBar's overall range
//               has been changed.
////////////////////////////////////////////////////////////////////
void PGSliderBarNotify::
slider_bar_set_range(PGSliderBar *) {
}
