// Filename: stitchImageOutputter.cxx
// Created by:  drose (09Nov99)
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

#include "stitchImageOutputter.h"


////////////////////////////////////////////////////////////////////
//     Function: StitchImageOutputter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
StitchImageOutputter::
StitchImageOutputter() {
  _screen = new StitchMultiScreen;
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImageOutputter::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
StitchImageOutputter::
~StitchImageOutputter() {
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImageOutputter::add_screen
//       Access: Public
//  Description: Adds a screen to the list of screens projected onto.
//               If there are no screens, the default is to project
//               the images to infinity; otherwise, the screens are
//               used.
////////////////////////////////////////////////////////////////////
void StitchImageOutputter::
add_screen(StitchScreen *screen) {
  _screen->add_screen(screen);
}

////////////////////////////////////////////////////////////////////
//     Function: StitchImageOutputter::set_eyepoint
//       Access: Public, Virtual
//  Description: Sets the eye point to the indicated coordinate frame,
//               if it makes sense to this kind of outputter.
////////////////////////////////////////////////////////////////////
void StitchImageOutputter::
set_eyepoint(const LMatrix4d &) {
}
