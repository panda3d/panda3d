// Filename: pgMouseWatcherRegion.cxx
// Created by:  drose (02Jul01)
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

#include "pgMouseWatcherRegion.h"
#include "pgItem.h"

#include "string_utils.h"

int PGMouseWatcherRegion::_next_index = 0;
TypeHandle PGMouseWatcherRegion::_type_handle;

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
//     Function: PGMouseWatcherRegion::enter_region
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse enters the region.  The mouse is only
//               considered to be "entered" in one region at a time;
//               in the case of nested regions, it exits the outer
//               region before entering the inner one.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
enter_region(const MouseWatcherParameter &param) {
  if (_item != (PGItem *)NULL) {
    _item->enter_region(param);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::exit_region
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse exits the region.  The mouse is only considered
//               to be "entered" in one region at a time; in the case
//               of nested regions, it exits the outer region before
//               entering the inner one.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
exit_region(const MouseWatcherParameter &param) {
  if (_item != (PGItem *)NULL) {
    _item->exit_region(param);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::within_region
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse moves within the boundaries of the region, even
//               if it is also within the boundaries of a nested
//               region.  This is different from "enter", which is
//               only called whenever the mouse is within only that
//               region.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
within_region(const MouseWatcherParameter &param) {
  if (_item != (PGItem *)NULL) {
    _item->within_region(param);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::without_region
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse moves completely outside the boundaries of the
//               region.  See within_region().
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
without_region(const MouseWatcherParameter &param) {
  if (_item != (PGItem *)NULL) {
    _item->without_region(param);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::press
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button is depressed while the mouse
//               is within the region.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
press(const MouseWatcherParameter &param) {
  if (_item != (PGItem *)NULL) {
    _item->press(param, false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::release
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button previously depressed with
//               press() is released.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
release(const MouseWatcherParameter &param) {
  if (_item != (PGItem *)NULL) {
    _item->release(param, false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::keystroke
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever 
//               the user presses a key.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
keystroke(const MouseWatcherParameter &param) {
  if (_item != (PGItem *)NULL) {
    _item->keystroke(param, false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::candidate
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever 
//               the user selects an option from the IME menu.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
candidate(const MouseWatcherParameter &param) {
  if (_item != (PGItem *)NULL) {
    _item->candidate(param, false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::move
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever 
//               the user moves the mouse within the region
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
move(const MouseWatcherParameter &param) {
  if (_item != (PGItem *)NULL) {
    _item->move(param);
  }
}
