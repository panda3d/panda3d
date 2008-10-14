// Filename: clientAnalogDevice.I
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
//     Function: ClientAnalogDevice::AnalogState::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ClientAnalogDevice::AnalogState::
AnalogState() :
  _state(0.0),
  _known(false)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ClientAnalogDevice::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ClientAnalogDevice::
ClientAnalogDevice(ClientBase *client, const string &device_name):
  ClientDevice(client, get_class_type(), device_name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ClientAnalogDevice::get_num_controls
//       Access: Public
//  Description: Returns the number of analog controls known to the
//               ClientAnalogDevice.  This number may change as
//               more controls are discovered.
////////////////////////////////////////////////////////////////////
INLINE int ClientAnalogDevice::
get_num_controls() const {
  return _controls.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ClientAnalogDevice::set_control_state
//       Access: Public
//  Description: Sets the state of the indicated analog index.  The
//               caller should ensure that acquire() is in effect while
//               this call is made.  This should be a number in the
//               range -1.0 to 1.0, representing the current position
//               of the control within its total range of movement.
////////////////////////////////////////////////////////////////////
INLINE void ClientAnalogDevice::
set_control_state(int index, double state) {
  ensure_control_index(index);
  nassertv(index >= 0 && index < (int)_controls.size());
  _controls[index]._state = state;
  _controls[index]._known = true;
}

////////////////////////////////////////////////////////////////////
//     Function: ClientAnalogDevice::get_control_state
//       Access: Public
//  Description: Returns the current position of indicated analog
//               control (identified by its index number), or 0.0 if
//               the control is unknown.  The normal range of a single
//               control is -1.0 to 1.0.
////////////////////////////////////////////////////////////////////
INLINE double ClientAnalogDevice::
get_control_state(int index) const {
  if (index >= 0 && index < (int)_controls.size()) {
    return _controls[index]._state;
  } else {
    return 0.0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClientAnalogDevice::is_control_known
//       Access: Public
//  Description: Returns true if the state of the indicated analog
//               control is known, or false if we have never heard
//               anything about this particular control.
////////////////////////////////////////////////////////////////////
INLINE bool ClientAnalogDevice::
is_control_known(int index) const {
  if (index >= 0 && index < (int)_controls.size()) {
    return _controls[index]._known;
  } else {
    return false;
  }
}
