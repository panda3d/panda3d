// Filename: pgMouseWatcherRegion.cxx
// Created by:  drose (02Jul01)
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

#include "pgMouseWatcherRegion.h"
#include "pgItem.h"

#include "string_utils.h"

TypeHandle PGMouseWatcherRegion::_type_handle;
int PGMouseWatcherRegion::_next_index = 0;

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PGMouseWatcherRegion::
PGMouseWatcherRegion(PGItem *item) :
#ifndef CPPPARSER
  MouseWatcherRegion("pg" + format_string(_next_index++), 0, 0, 0, 0),
#endif
  _item(item)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PGMouseWatcherRegion::
~PGMouseWatcherRegion() {
}


////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::enter
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse enters the region.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
enter() {
  if (_item != (PGItem *)NULL) {
    _item->enter();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::exit
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse exits the region.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
exit() {
  if (_item != (PGItem *)NULL) {
    _item->exit();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::button_down
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button is depressed while the mouse
//               is within the region.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
button_down(ButtonHandle button) {
  if (_item != (PGItem *)NULL) {
    _item->button_down(button);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::button_up
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button previously depressed with
//               button_down() is release.  The bool is_within flag is
//               true if the button was released while the mouse was
//               still within the region, or false if it was released
//               outside the region.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
button_up(ButtonHandle button, bool is_within) {
  if (_item != (PGItem *)NULL) {
    _item->button_up(button, is_within);
  }
}
