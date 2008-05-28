// Filename: clientButtonDevice.I
// Created by:  drose (26Jan01)
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


////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::ButtonState::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ClientButtonDevice::ButtonState::
ButtonState() :
  _handle(ButtonHandle::none()),
  _state(S_unknown)
{
}


////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::get_num_buttons
//       Access: Public
//  Description: Returns the number of buttons known to the
//               ClientButtonDevice.  This includes those buttons
//               whose state has been seen, as well as buttons that
//               have been associated with a ButtonHandle even if
//               their state is unknown.  This number may change as
//               more buttons are discovered.
////////////////////////////////////////////////////////////////////
INLINE int ClientButtonDevice::
get_num_buttons() const {
  return _buttons.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::set_button_map
//       Access: Public
//  Description: Associates the indicated ButtonHandle with the button
//               of the indicated index number.  When the given button
//               index changes state, a corresponding ButtonEvent will
//               be generated with the given ButtonHandle.  Pass
//               ButtonHandle::none() to turn off any association.
//
//               It is not necessary to call this if you simply want
//               to query the state of the various buttons by index
//               number; this is only necessary in order to generate
//               ButtonEvents when the buttons change state.
////////////////////////////////////////////////////////////////////
INLINE void ClientButtonDevice::
set_button_map(int index, ButtonHandle button) {
  ensure_button_index(index);
  nassertv(index >= 0 && index < (int)_buttons.size());
  _buttons[index]._handle = button;
}

////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::get_button_map
//       Access: Public
//  Description: Returns the ButtonHandle that was previously
//               associated with the given index number by
//               a call to set_button_map(), or ButtonHandle::none()
//               if no button was associated.
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle ClientButtonDevice::
get_button_map(int index) const {
  if (index >= 0 && index < (int)_buttons.size()) {
    return _buttons[index]._handle;
  } else {
    return ButtonHandle::none();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::get_button_state
//       Access: Public
//  Description: Returns true if the indicated button (identified by
//               its index number) is currently known to be down, or
//               false if it is up or unknown.
////////////////////////////////////////////////////////////////////
INLINE bool ClientButtonDevice::
get_button_state(int index) const {
  if (index >= 0 && index < (int)_buttons.size()) {
    return (_buttons[index]._state == S_down);
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::is_button_known
//       Access: Public
//  Description: Returns true if the state of the indicated button is
//               known, or false if we have never heard anything about
//               this particular button.
////////////////////////////////////////////////////////////////////
INLINE bool ClientButtonDevice::
is_button_known(int index) const {
  if (index >= 0 && index < (int)_buttons.size()) {
    return _buttons[index]._state != S_unknown;
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::get_button_events
//       Access: Public
//  Description: Returns the list of recently-generated ButtonEvents.
//               This must be periodically cleared, or the buttons
//               will accumulate.
////////////////////////////////////////////////////////////////////
INLINE ButtonEventList *ClientButtonDevice::
get_button_events() const {
  return _button_events;
}
