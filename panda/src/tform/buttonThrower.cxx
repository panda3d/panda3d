// Filename: buttonThrower.cxx
// Created by:  drose (09Feb99)
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

#include "buttonThrower.h"

#include <buttonEventDataTransition.h>
#include <buttonEventDataTransition.h>
#include <buttonEvent.h>
#include <allTransitionsWrapper.h>
#include <throw_event.h>

TypeHandle ButtonThrower::_type_handle;
TypeHandle ButtonThrower::_button_events_type;


////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ButtonThrower::
ButtonThrower(const string &name) : DataNode(name) {
}


////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::set_prefix
//       Access: Public
//  Description: Sets the prefix which is prepended to all event names
//               thrown by this object.
////////////////////////////////////////////////////////////////////
void ButtonThrower::
set_prefix(const string &prefix) {
  _prefix = prefix;
}


////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::has_prefix
//       Access: Public
//  Description: Returns true if the ButtonThrower has a prefix set,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool ButtonThrower::
has_prefix() const {
  return !_prefix.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::get_prefix
//       Access: Public
//  Description: Returns the prefix that has been set on this
//               ButtonThrower.  See set_prefix().
////////////////////////////////////////////////////////////////////
string ButtonThrower::
get_prefix() const {
  return _prefix;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::get_modifier_buttons
//       Access: Public
//  Description: Returns the set of ModifierButtons that the
//               ButtonThrower will consider important enough to
//               prepend the event name with.  Normally, this set will
//               be empty, and the ButtonThrower will therefore ignore
//               all ModifierButtons attached to the key events, but
//               if one or more buttons have been added to this set,
//               and those modifier buttons are set on the button
//               event, then the event name will be prepended with the
//               names of the modifier buttons.
////////////////////////////////////////////////////////////////////
const ModifierButtons &ButtonThrower::
get_modifier_buttons() const {
  return _mods;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::set_modifier_buttons
//       Access: Public
//  Description: Changes the set of ModifierButtons that the
//               ButtonThrower will consider important enough to
//               prepend the event name with.  Normally, this set will
//               be empty, and the ButtonThrower will therefore ignore
//               all ModifierButtons attached to the key events, but
//               if one or more buttons have been added to this set,
//               then the event name will be prepended with the names
//               of the modifier buttons.
//
//               It is recommended that you change this setting by
//               first calling get_modifier_buttons(), making
//               adjustments, and passing the new value to
//               set_modifier_buttons().  This way the current state
//               of the modifier buttons will not be lost.
////////////////////////////////////////////////////////////////////
void ButtonThrower::
set_modifier_buttons(const ModifierButtons &mods) {
  _mods = mods;
}


////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::transmit_data
//       Access: Public
//  Description: Throw all events for button events found in the data
//               element.
////////////////////////////////////////////////////////////////////
void ButtonThrower::
transmit_data(AllTransitionsWrapper &data) {
  const ButtonEventDataTransition *b;
  if (get_transition_into(b, data, _button_events_type)) {
    ButtonEventDataTransition::const_iterator bi;
    for (bi = b->begin(); bi != b->end(); ++bi) {
      const ButtonEvent &be = (*bi);
      string event_name = _prefix + be._button.get_name();
      if (be._down) {
        if (!_mods.button_down(be._button)) {
          // We only prepend modifier names on the button-down events,
          // and only for buttons which are not themselves modifiers.
          event_name = _mods.get_prefix() + event_name;
        }

      } else {
        _mods.button_up(be._button);
        event_name += "-up";
      }

      throw_event(event_name);
    }
  }

  // Clear the data going down the pipe.
  data.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::init_type
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
void ButtonThrower::
init_type() {
  DataNode::init_type();
  register_type(_type_handle, "ButtonThrower",
                DataNode::get_class_type());

  ButtonEventDataTransition::init_type();
  register_data_transition(_button_events_type, "ButtonEvents",
                           ButtonEventDataTransition::get_class_type());
}
