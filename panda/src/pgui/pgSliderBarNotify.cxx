// Filename: pgSliderBarNotify.cxx
// Created by:  drose (18Aug05)
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
