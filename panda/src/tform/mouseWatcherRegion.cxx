// Filename: mouseWatcherRegion.cxx
// Created by:  drose (13Jul00)
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

#include "mouseWatcherRegion.h"

#include "indent.h"


TypeHandle MouseWatcherRegion::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
output(ostream &out) const {
  out << get_name() << " lrbt = " << _frame;
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::write
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_name() << " lrbt = " << _frame
    << ", sort = " << _sort << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::enter
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse enters the region.  The mouse is only
//               considered to be "entered" in one region at a time;
//               in the case of nested regions, it exits the outer
//               region before entering the inner one.
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
enter(const MouseWatcherParameter &) {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::exit
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse exits the region.  The mouse is only considered
//               to be "entered" in one region at a time; in the case
//               of nested regions, it exits the outer region before
//               entering the inner one.
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
exit(const MouseWatcherParameter &) {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::within
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse moves within the boundaries of the region, even
//               if it is also within the boundaries of a nested
//               region.  This is different from "enter", which is
//               only called whenever the mouse is within only that
//               region.
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
within(const MouseWatcherParameter &) {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::without
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse moves completely outside the boundaries of the
//               region.  See within().
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
without(const MouseWatcherParameter &) {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::press
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button is depressed while the mouse
//               is within the region.
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
press(const MouseWatcherParameter &) {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::release
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button previously depressed with
//               press() is released.
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
release(const MouseWatcherParameter &) {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::keystroke
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               keystroke is generated by the user.
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
keystroke(const MouseWatcherParameter &) {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::candidate
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever an
//               IME candidate is highlighted by the user.
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
candidate(const MouseWatcherParameter &) {
}
