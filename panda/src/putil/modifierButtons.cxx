// Filename: modifierButtons.cxx
// Created by:  drose (01Mar00)
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

#include "modifierButtons.h"

#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
ModifierButtons::
ModifierButtons() :
  _button_list(0),
  _state(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
ModifierButtons::
ModifierButtons(const ModifierButtons &copy) :
  _button_list(copy._button_list),
  _state(copy._state)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
ModifierButtons::
~ModifierButtons() {
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::add_button
//       Access: Published
//  Description: Adds the indicated button to the set of buttons that
//               will be monitored for upness and downness.  Returns
//               true if the button was added, false if it was already
//               being monitored or if too many buttons are currently
//               being monitored.
////////////////////////////////////////////////////////////////////
bool ModifierButtons::
add_button(ButtonHandle button) {
  nassertr(button != ButtonHandle::none(), false);

  static const int max_buttons = sizeof(BitmaskType) * 8;

  if ((int)_button_list.size() >= max_buttons) {
    return false;
  }

  // First, check to see if the button is already being monitored.
  if (has_button(button)) {
    return false;
  }

  // Ok, it's not; add it.
  modify_button_list();
  _button_list.push_back(button);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::has_button
//       Access: Published
//  Description: Returns true if the indicated button is in the set of
//               buttons being monitored, false otherwise.
////////////////////////////////////////////////////////////////////
bool ModifierButtons::
has_button(ButtonHandle button) const {
  PTA(ButtonHandle)::const_iterator bi;
  for (bi = _button_list.begin(); bi != _button_list.end(); ++bi) {
    if (button == (*bi)) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::remove_button
//       Access: Published
//  Description: Removes the indicated button from the set of buttons
//               being monitored.  Returns true if the button was
//               removed, false if it was not being monitored in the
//               first place.
////////////////////////////////////////////////////////////////////
bool ModifierButtons::
remove_button(ButtonHandle button) {
  // We use i instead of an iterator, because we need to call
  // modify_button_list() just before we remove the button, and that
  // may invalidate all of the iterators.

  for (int i = 0; i < (int)_button_list.size(); i++) {
    if (button == _button_list[i]) {
      modify_button_list();
      _button_list.erase(_button_list.begin() + i);

      // Now remove the corresponding bit from the bitmask and shift
      // all the bits above it down.
      BitmaskType mask = ((BitmaskType)1 << i);
      BitmaskType below = mask - 1;
      BitmaskType above = (~below) & (~mask);

      _state = ((_state & above) >> 1) | (_state & below);
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::button_down
//       Access: Published
//  Description: Records that a particular button has been pressed.
//               If the given button is one of the buttons that is
//               currently being monitored, this will update the
//               internal state appropriately; otherwise, it will do
//               nothing.  Returns true if the button is one that was
//               monitored, or false otherwise.
////////////////////////////////////////////////////////////////////
bool ModifierButtons::
button_down(ButtonHandle button) {
  for (int i = 0; i < (int)_button_list.size(); i++) {
    if (button == _button_list[i]) {
      _state |= ((BitmaskType)1 << i);
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::button_up
//       Access: Published
//  Description: Records that a particular button has been released.
//               If the given button is one of the buttons that is
//               currently being monitored, this will update the
//               internal state appropriately; otherwise, it will do
//               nothing.  Returns true if the button is one that was
//               monitored, or false otherwise.
////////////////////////////////////////////////////////////////////
bool ModifierButtons::
button_up(ButtonHandle button) {
  for (int i = 0; i < (int)_button_list.size(); i++) {
    if (button == _button_list[i]) {
      _state &= ~((BitmaskType)1 << i);
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::is_down
//       Access: Published
//  Description: Returns true if the indicated button is known to be
//               down, or false if it is known to be up or if it is
//               not in the set of buttons being tracked.
////////////////////////////////////////////////////////////////////
bool ModifierButtons::
is_down(ButtonHandle button) const {
  for (int i = 0; i < (int)_button_list.size(); i++) {
    if (button == _button_list[i]) {
      return ((_state & ((BitmaskType)1 << i)) != 0);
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::get_prefix
//       Access: Published
//  Description: Returns a string which can be used to prefix any
//               button name or event name with the unique set of
//               modifier buttons currently being held.
////////////////////////////////////////////////////////////////////
string ModifierButtons::
get_prefix() const {
  string prefix;
  for (int i = 0; i < (int)_button_list.size(); i++) {
    if ((_state & ((BitmaskType)1 << i)) != 0) {
      prefix += _button_list[i].get_name();
      prefix += '-';
    }
  }

  return prefix;
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::output
//       Access: Published
//  Description: Writes a one-line summary of the buttons known to be
//               down.
////////////////////////////////////////////////////////////////////
void ModifierButtons::
output(ostream &out) const {
  out << "[";
  for (int i = 0; i < (int)_button_list.size(); i++) {
    if ((_state & ((BitmaskType)1 << i)) != 0) {
      out << " " << _button_list[i];
    }
  }
  out << " ]";
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::write
//       Access: Published
//  Description: Writes a multi-line summary including all of the
//               buttons being monitored and which ones are known to
//               be down.
////////////////////////////////////////////////////////////////////
void ModifierButtons::
write(ostream &out) const {
  out << "ModifierButtons:\n";
  for (int i = 0; i < (int)_button_list.size(); i++) {
    out << "  " << _button_list[i];
    if ((_state & ((BitmaskType)1 << i)) != 0) {
      out << " (down)";
    }
    out << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::modify_button_list
//       Access: Private
//  Description: Implements a poor-man's copy-on-write for the
//               ModifierButtons class.  If any reference counts are
//               held on our _button_list, besides ourselves, then
//               allocates and copies a brand new copy of the
//               _button_list.  This should be done in preparation for
//               any modifications to the _button_list, since multiple
//               instances of the ModifierButtons object may share the
//               same _button_list pointer.
////////////////////////////////////////////////////////////////////
void ModifierButtons::
modify_button_list() {
  if (_button_list.get_ref_count() > 1) {
    PTA(ButtonHandle) old_list = _button_list;
    _button_list = PTA(ButtonHandle)(0);

    // This forces a new allocation and memberwise copy, instead of
    // just a reference-counting pointer copy.
    _button_list.v() = old_list.v();
  }

  // Now we should be the only ones holding a count.
  nassertv(_button_list.get_ref_count() == 1);
}
