// Filename: buttonEventList.cxx
// Created by:  drose (12Mar02)
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

#include "buttonEventList.h"
#include "modifierButtons.h"
#include "indent.h"

TypeHandle ButtonEventList::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventList::update_mods
//       Access: Public
//  Description: Updates the indicated ModifierButtons object with all
//               of the button up/down transitions indicated in the
//               list.
////////////////////////////////////////////////////////////////////
void ButtonEventList::
update_mods(ModifierButtons &mods) const {
  Events::const_iterator ei;
  for (ei = _events.begin(); ei != _events.end(); ++ei) {
    mods.add_event(*ei);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventList::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void ButtonEventList::
output(ostream &out) const {
  out << get_type() << " (" << get_num_events() << " events)";
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEventList::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void ButtonEventList::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << ":\n";
  Events::const_iterator ei;
  for (ei = _events.begin(); ei != _events.end(); ++ei) {
    indent(out, indent_level + 2) << (*ei) << "\n";
  }
}
