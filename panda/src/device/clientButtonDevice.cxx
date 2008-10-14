// Filename: clientButtonDevice.cxx
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


#include "clientButtonDevice.h"

#include "indent.h"

TypeHandle ClientButtonDevice::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
ClientButtonDevice::
ClientButtonDevice(ClientBase *client, const string &device_name):
  ClientDevice(client, get_class_type(), device_name)
{
  _button_events = new ButtonEventList();
}


////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::set_button_state
//       Access: Public
//  Description: Sets the state of the indicated button index, where
//               true indicates down, and false indicates up.  This
//               may generate a ButtonEvent if the button has an
//               associated ButtonHandle.  The caller should ensure
//               that acquire() is in effect while this call is made.
////////////////////////////////////////////////////////////////////
void ClientButtonDevice::
set_button_state(int index, bool down) {
  ensure_button_index(index);
  nassertv(index >= 0 && index < (int)_buttons.size());
  _buttons[index]._state = down ? S_down : S_up;

  ButtonHandle handle = _buttons[index]._handle;
  if (handle != ButtonHandle::none()) {
    _button_events->add_event(ButtonEvent(handle, down ? ButtonEvent::T_down : ButtonEvent::T_up));
  }
}


////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::ensure_button_index
//       Access: Private
//  Description: Guarantees that there is a slot in the array for the
//               indicated index number, by filling the array up to
//               that index if necessary.
////////////////////////////////////////////////////////////////////
void ClientButtonDevice::
ensure_button_index(int index) {
  nassertv(index >= 0);

  _buttons.reserve(index + 1);
  while ((int)_buttons.size() <= index) {
    _buttons.push_back(ButtonState());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ClientButtonDevice::
output(ostream &out) const {
  out << get_type() << " " << get_device_name() << " (";
  output_buttons(out);
  out << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void ClientButtonDevice::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << get_type() << " " << get_device_name() << ":\n";
  write_buttons(out, indent_level + 2);
}

////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::output_buttons
//       Access: Public
//  Description: Writes a one-line string of all of the current button
//               states.
////////////////////////////////////////////////////////////////////
void ClientButtonDevice::
output_buttons(ostream &out) const {
  bool any_buttons = false;
  Buttons::const_iterator bi;
  for (bi = _buttons.begin(); bi != _buttons.end(); ++bi) {
    const ButtonState &state = (*bi);
    if (state._state != S_unknown) {
      if (any_buttons) {
        out << ", ";
      }
      any_buttons = true;
      out << (int)(bi - _buttons.begin()) << "=";
      if (state._state == S_up) {
        out << "up";
      } else {
        out << "down";
      }
    }
  }

  if (!any_buttons) {
    out << "no known buttons";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClientButtonDevice::write_buttons
//       Access: Public
//  Description: Writes a multi-line description of the current button
//               states.
////////////////////////////////////////////////////////////////////
void ClientButtonDevice::
write_buttons(ostream &out, int indent_level) const {
  bool any_buttons = false;
  Buttons::const_iterator bi;
  for (bi = _buttons.begin(); bi != _buttons.end(); ++bi) {
    const ButtonState &state = (*bi);
    if (state._state != S_unknown) {
      any_buttons = true;

      indent(out, indent_level)
        << (int)(bi - _buttons.begin()) << ". ";

      if (state._handle != ButtonHandle::none()) {
        out << "(" << state._handle << ") ";
      }

      if (state._state == S_up) {
        out << "up";
      } else {
        out << "down";
      }
      out << "\n";
    }
  }

  if (!any_buttons) {
    indent(out, indent_level)
      << "(no known buttons)\n";
  }
}
