// Filename: pgMouseWatcherRegion.cxx
// Created by:  drose (02Jul01)
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
//     Function: PGMouseWatcherRegion::enter
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse enters the region.  The mouse is only
//               considered to be "entered" in one region at a time;
//               in the case of nested regions, it exits the outer
//               region before entering the inner one.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
enter(const MouseWatcherParameter &param) {
  if (_item != (PGItem *)NULL) {
    _item->enter(param);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::exit
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse exits the region.  The mouse is only considered
//               to be "entered" in one region at a time; in the case
//               of nested regions, it exits the outer region before
//               entering the inner one.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
exit(const MouseWatcherParameter &param) {
  if (_item != (PGItem *)NULL) {
    _item->exit(param);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::within
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse moves within the boundaries of the region, even
//               if it is also within the boundaries of a nested
//               region.  This is different from "enter", which is
//               only called whenever the mouse is within only that
//               region.
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
within(const MouseWatcherParameter &param) {
  if (_item != (PGItem *)NULL) {
    _item->within(param);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherRegion::without
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse moves completely outside the boundaries of the
//               region.  See within().
////////////////////////////////////////////////////////////////////
void PGMouseWatcherRegion::
without(const MouseWatcherParameter &param) {
  if (_item != (PGItem *)NULL) {
    _item->without(param);
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
