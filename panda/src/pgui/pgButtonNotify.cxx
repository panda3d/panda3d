/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgButtonNotify.cxx
 * @author drose
 * @date 2005-08-18
 */

#include "pgButtonNotify.h"
#include "pgButton.h"

////////////////////////////////////////////////////////////////////
//     Function: PGButtonNotify::button_click
//       Access: Protected, Virtual
//  Description: Called whenever a watched PGButton has been clicked.
////////////////////////////////////////////////////////////////////
void PGButtonNotify::
button_click(PGButton *, const MouseWatcherParameter &) {
}
