// Filename: showHideTransition.I
// Created by:  drose (26Apr00)
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
//     Function: ShowHideTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ShowHideTransition::
ShowHideTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: ShowHideTransition::all_off
//       Access: Public
//  Description: This named constructor returns a ShowHideTransition
//               that's preconfigured to disable visibility for all
//               cameras.
////////////////////////////////////////////////////////////////////
INLINE ShowHideTransition ShowHideTransition::
all_off() {
  ShowHideTransition t;
  t.set_default_dir(TD_off);
  return t;
}

////////////////////////////////////////////////////////////////////
//     Function: ShowHideTransition::all_on
//       Access: Public
//  Description: This named constructor returns a ShowHideTransition
//               that's preconfigured to enable visibility for all
//               cameras.
////////////////////////////////////////////////////////////////////
INLINE ShowHideTransition ShowHideTransition::
all_on() {
  ShowHideTransition t;
  t.set_default_dir(TD_on);
  return t;
}
