// Filename: pgMouseWatcherBackground.cxx
// Created by:  drose (23Aug01)
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

#include "pgMouseWatcherBackground.h"
#include "pgItem.h"

TypeHandle PGMouseWatcherBackground::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherBackground::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PGMouseWatcherBackground::
PGMouseWatcherBackground() :
  MouseWatcherRegion("PGMouseWatcherBackground", 0, 0, 0, 0)
{
  set_active(false);
  set_keyboard(true);
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherBackground::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PGMouseWatcherBackground::
~PGMouseWatcherBackground() {
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherBackground::press
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button is depressed while the mouse
//               is within the background.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherBackground::
press(const MouseWatcherParameter &param) {
  PGItem::background_press(param);
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherBackground::release
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button previously depressed with
//               press() is released.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherBackground::
release(const MouseWatcherParameter &param) {
  PGItem::background_release(param);
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherBackground::keystroke
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever
//               the user presses a key.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherBackground::
keystroke(const MouseWatcherParameter &param) {
  PGItem::background_keystroke(param);
}
