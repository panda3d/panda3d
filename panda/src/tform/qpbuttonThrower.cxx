// Filename: qpbuttonThrower.cxx
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

#include "qpbuttonThrower.h"

#include "buttonEvent.h"
#include "buttonEventList.h"
#include "dataNodeTransmit.h"
#include "throw_event.h"
#include "indent.h"

TypeHandle qpButtonThrower::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: qpButtonThrower::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
qpButtonThrower::
qpButtonThrower(const string &name) :
  qpDataNode(name)
{
  _button_events_input = define_input("ButtonEvents", ButtonEventList::get_class_type());
  _button_events_output = define_output("ButtonEvents", ButtonEventList::get_class_type());

  _button_events = new ButtonEventList;

  _throw_buttons_active = false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonThrower::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
qpButtonThrower::
~qpButtonThrower() {
}


////////////////////////////////////////////////////////////////////
//     Function: qpButtonThrower::set_prefix
//       Access: Published
//  Description: Sets the prefix which is prepended to all event names
//               thrown by this object.
////////////////////////////////////////////////////////////////////
void qpButtonThrower::
set_prefix(const string &prefix) {
  _prefix = prefix;
}


////////////////////////////////////////////////////////////////////
//     Function: qpButtonThrower::has_prefix
//       Access: Published
//  Description: Returns true if the qpButtonThrower has a prefix set,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool qpButtonThrower::
has_prefix() const {
  return !_prefix.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonThrower::get_prefix
//       Access: Published
//  Description: Returns the prefix that has been set on this
//               qpButtonThrower.  See set_prefix().
////////////////////////////////////////////////////////////////////
string qpButtonThrower::
get_prefix() const {
  return _prefix;
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonThrower::get_modifier_buttons
//       Access: Published
//  Description: Returns the set of ModifierButtons that the
//               qpButtonThrower will consider important enough to
//               prepend the event name with.  Normally, this set will
//               be empty, and the qpButtonThrower will therefore ignore
//               all ModifierButtons attached to the key events, but
//               if one or more buttons have been added to this set,
//               and those modifier buttons are set on the button
//               event, then the event name will be prepended with the
//               names of the modifier buttons.
////////////////////////////////////////////////////////////////////
const ModifierButtons &qpButtonThrower::
get_modifier_buttons() const {
  return _mods;
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonThrower::set_modifier_buttons
//       Access: Published
//  Description: Changes the set of ModifierButtons that the
//               qpButtonThrower will consider important enough to
//               prepend the event name with.  Normally, this set will
//               be empty, and the qpButtonThrower will therefore ignore
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
void qpButtonThrower::
set_modifier_buttons(const ModifierButtons &mods) {
  _mods = mods;
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonThrower::set_throw_buttons_active
//       Access: Published
//  Description: Sets the flag that indicates whether the
//               qpButtonThrower will only process events for the
//               explicitly named buttons or not.  Normally this is
//               false, meaning all buttons are processed; set it true
//               to indicate that only some buttons should be
//               processed.  See add_throw_button().
////////////////////////////////////////////////////////////////////
void qpButtonThrower::
set_throw_buttons_active(bool flag) {
  _throw_buttons_active = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonThrower::get_throw_buttons_active
//       Access: Published
//  Description: Returns the flag that indicates whether the
//               qpButtonThrower will only process events for the
//               explicitly named buttons or not.  See
//               set_throw_buttons_active().
////////////////////////////////////////////////////////////////////
bool qpButtonThrower::
get_throw_buttons_active() const {
  return _throw_buttons_active;
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonThrower::add_throw_button
//       Access: Published
//  Description: Adds a new button to the set of buttons that the
//               qpButtonThrower explicitly processes.
//
//               If set_throw_buttons_active is false (which is the
//               default), the qpButtonThrower will process all buttons.
//               Otherwise, the qpButtonThrower will only process events
//               for the button(s) explicitly named by this function;
//               buttons not on the list will be ignored by this
//               object and passed on downstream to the child node(s)
//               in the data graph.  A button that *is* on the list
//               will be processed by the qpButtonThrower and not passed
//               on to the child node(s).
//
//               The return value is true if the button is added, or
//               false if it was already in the set.
////////////////////////////////////////////////////////////////////
bool qpButtonThrower::
add_throw_button(const ModifierButtons &mods, const ButtonHandle &button) {
  ThrowButtonDef &def = _throw_buttons[button];

  // This is a vector of ModifierButtons for which the indicated
  // button is handled.  Make sure the current ModifierButtons object
  // is not already on the list.
  ThrowButtonDef::iterator di;
  for (di = def.begin(); di != def.end(); ++di) {
    if (mods.matches(*di)) {
      return false;
    }
  }

  def.push_back(mods);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonThrower::remove_throw_button
//       Access: Published
//  Description: Removes the indicated button from the set of buttons
//               that the qpButtonThrower explicitly processes.  See
//               add_throw_button().
//
//               The return value is true if the button is removed, or
//               false if it was not on the set.
////////////////////////////////////////////////////////////////////
bool qpButtonThrower::
remove_throw_button(const ModifierButtons &mods, const ButtonHandle &button) {
  ThrowButtons::iterator ti = _throw_buttons.find(button);
  if (ti == _throw_buttons.end()) {
    // No buttons of this kind are in the set.
    return false;
  }

  ThrowButtonDef &def = (*ti).second;

  // This is a vector of ModifierButtons for which the indicated
  // button is handled.
  ThrowButtonDef::iterator di;
  for (di = def.begin(); di != def.end(); ++di) {
    if (mods.matches(*di)) {
      def.erase(di);
      if (def.empty()) {
        _throw_buttons.erase(ti);
      }
      return true;
    }
  }

  // The indicated ModifierButtons are not applied to this button in
  // the set.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonThrower::has_throw_button
//       Access: Published
//  Description: Returns true if the indicated button is on the set of
//               buttons that will be processed by the qpButtonThrower,
//               false otherwise.  See add_throw_button().
////////////////////////////////////////////////////////////////////
bool qpButtonThrower::
has_throw_button(const ModifierButtons &mods, const ButtonHandle &button) const {
  ThrowButtons::const_iterator ti = _throw_buttons.find(button);
  if (ti == _throw_buttons.end()) {
    // No buttons of this kind are in the set.
    return false;
  }

  const ThrowButtonDef &def = (*ti).second;

  // This is a vector of ModifierButtons for which the indicated
  // button is handled.
  ThrowButtonDef::const_iterator di;
  for (di = def.begin(); di != def.end(); ++di) {
    if (mods.matches(*di)) {
      return true;
    }
  }

  // The indicated ModifierButtons are not applied to this button in
  // the set.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonThrower::has_throw_button
//       Access: Published
//  Description: Returns true if the indicated button, in conjunction
//               with any nonspecified modifier buttons, is on the set
//               of buttons that will be processed by the
//               qpButtonThrower.  That is to say, returns true if this
//               button was ever passed as the second parameter
//               add_throw_button(), regardless of what the first
//               parameter was.
////////////////////////////////////////////////////////////////////
bool qpButtonThrower::
has_throw_button(const ButtonHandle &button) const {
  ThrowButtons::const_iterator ti = _throw_buttons.find(button);
  if (ti == _throw_buttons.end()) {
    // No buttons of this kind are in the set.
    return false;
  }

  const ThrowButtonDef &def = (*ti).second;
  return !def.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonThrower::clear_throw_buttons
//       Access: Published
//  Description: Empties the set of buttons that were added via
//               add_throw_button().  See add_throw_button().
////////////////////////////////////////////////////////////////////
void qpButtonThrower::
clear_throw_buttons() {
  _throw_buttons.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonThrower::write
//       Access: Public, Virtual
//  Description: Throw all events for button events found in the data
//               element.
////////////////////////////////////////////////////////////////////
void qpButtonThrower::
write(ostream &out, int indent_level) const {
  qpDataNode::write(out, indent_level);
  if (_throw_buttons_active) {
    indent(out, indent_level)
      << "Processing keys:\n";
    // Write the list of buttons that we're processing too.
    ThrowButtons::const_iterator ti;
    for (ti = _throw_buttons.begin(); ti != _throw_buttons.end(); ++ti) {
      ButtonHandle button = (*ti).first;
      const ThrowButtonDef &def = (*ti).second;
      ThrowButtonDef::const_iterator di;
      for (di = def.begin(); di != def.end(); ++di) {
        indent(out, indent_level + 2)
          << (*di).get_prefix() << button.get_name() << "\n";
      }
    }
  }    
}

////////////////////////////////////////////////////////////////////
//     Function: qpButtonThrower::do_transmit_data
//       Access: Protected, Virtual
//  Description: The virtual implementation of transmit_data().  This
//               function receives an array of input parameters and
//               should generate an array of output parameters.  The
//               input parameters may be accessed with the index
//               numbers returned by the define_input() calls that
//               were made earlier (presumably in the constructor);
//               likewise, the output parameters should be set with
//               the index numbers returned by the define_output()
//               calls.
////////////////////////////////////////////////////////////////////
void qpButtonThrower::
do_transmit_data(const DataNodeTransmit &input, DataNodeTransmit &output) {
  // Clear our outgoing button events.  We'll fill it up again with
  // just those events that want to carry on.
  _button_events->clear();

  if (input.has_data(_button_events_input)) {
    const ButtonEventList *button_events;
    DCAST_INTO_V(button_events, input.get_data(_button_events_input).get_ptr());

    int num_events = button_events->get_num_events();
    for (int i = 0; i < num_events; i++) {
      const ButtonEvent &be = button_events->get_event(i);
      string event_name = be._button.get_name();

      if (be._type == ButtonEvent::T_down) {
        // Button down.
        if (!_mods.button_down(be._button)) {
          // We only prepend modifier names on the button-down events,
          // and only for buttons which are not themselves modifiers.
          event_name = _mods.get_prefix() + event_name;
        }

        if (!_throw_buttons_active || has_throw_button(_mods, be._button)) {
          // Process this button.
          throw_event(_prefix + event_name);
          
        } else {
          // Don't process this button; instead, pass it down to future
          // generations.
          _button_events->add_event(be);
        }
          
      } else if (be._type == ButtonEvent::T_up) {
        // Button up.
        _mods.button_up(be._button);

        // We always throw button "up" events if we have any
        // definition for the button at all, regardless of the state
        // of the modifier keys.
        if (!_throw_buttons_active || has_throw_button(be._button)) {
          throw_event(_prefix + event_name + "-up");
        }
        if (_throw_buttons_active) {
          // Now pass the event on to future generations.  We always
          // pass "up" events, even if we are intercepting this
          // particular button; unless we're processing all buttons in
          // which case it doesn't matter.
          _button_events->add_event(be);
        }

      } else {
        // Some other kind of button event (e.g. keypress).  Don't
        // throw an event for this, but do pass it down.
        _button_events->add_event(be);
      }
    }
  }

  output.set_data(_button_events_output, EventParameter(_button_events));
}
