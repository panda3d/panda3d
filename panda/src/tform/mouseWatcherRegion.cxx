// Filename: mouseWatcherRegion.cxx
// Created by:  drose (13Jul00)
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

#include "mouseWatcherRegion.h"

#include <indent.h>


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
  indent(out, indent_level) << get_name() << " lrbt = " << _frame << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::enter
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse enters the region.
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
enter() {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::exit
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               mouse exits the region.
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
exit() {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::button_down
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button is depressed while the mouse
//               is within the region.
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
button_down(ButtonHandle, float, float) {
}

////////////////////////////////////////////////////////////////////
//     Function: MouseWatcherRegion::button_up
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever a
//               mouse or keyboard button previously depressed with
//               button_down() is release.  The bool is_within flag is
//               true if the button was released while the mouse was
//               still within the region, or false if it was released
//               outside the region.
////////////////////////////////////////////////////////////////////
void MouseWatcherRegion::
button_up(ButtonHandle, float, float, bool) {
}
