// Filename: buttonThrower.I
// Created by:  drose (26Dec03)
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


////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::set_prefix
//       Access: Published
//  Description: Sets the prefix which is prepended to all event names
//               thrown by this object.
////////////////////////////////////////////////////////////////////
INLINE void ButtonThrower::
set_prefix(const string &prefix) {
  _prefix = prefix;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::has_prefix
//       Access: Published
//  Description: Returns true if the ButtonThrower has a prefix set,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ButtonThrower::
has_prefix() const {
  return !_prefix.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::get_prefix
//       Access: Published
//  Description: Returns the prefix that has been set on this
//               ButtonThrower.  See set_prefix().
////////////////////////////////////////////////////////////////////
INLINE string ButtonThrower::
get_prefix() const {
  return _prefix;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::set_time_flag
//       Access: Published
//  Description: Sets the flag that indicates whether the time of the
//               button event should be passed as a parameter or not.
//               When this is true, an additional parameter is
//               generated on each event (before all the parameters
//               named by add_parameter) that consists of a single
//               double value, and reflects the time the button was
//               pressed or released, as a value from
//               ClockObject::get_global_clock().
////////////////////////////////////////////////////////////////////
INLINE void ButtonThrower::
set_time_flag(bool time_flag) {
  _time_flag = time_flag;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::get_time_flag
//       Access: Published
//  Description: Returns the flag that indicates whether the time of
//               the button event should be passed as a parameter.
////////////////////////////////////////////////////////////////////
INLINE bool ButtonThrower::
get_time_flag() const {
  return _time_flag;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::get_modifier_buttons
//       Access: Published
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
INLINE const ModifierButtons &ButtonThrower::
get_modifier_buttons() const {
  return _mods;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::set_modifier_buttons
//       Access: Published
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
INLINE void ButtonThrower::
set_modifier_buttons(const ModifierButtons &mods) {
  _mods = mods;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::set_throw_buttons_active
//       Access: Published
//  Description: Sets the flag that indicates whether the
//               ButtonThrower will only process events for the
//               explicitly named buttons or not.  Normally this is
//               false, meaning all buttons are processed; set it true
//               to indicate that only some buttons should be
//               processed.  See add_throw_button().
////////////////////////////////////////////////////////////////////
INLINE void ButtonThrower::
set_throw_buttons_active(bool flag) {
  _throw_buttons_active = flag;
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonThrower::get_throw_buttons_active
//       Access: Published
//  Description: Returns the flag that indicates whether the
//               ButtonThrower will only process events for the
//               explicitly named buttons or not.  See
//               set_throw_buttons_active().
////////////////////////////////////////////////////////////////////
INLINE bool ButtonThrower::
get_throw_buttons_active() const {
  return _throw_buttons_active;
}
