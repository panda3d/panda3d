// Filename: inputDevice.I
// Created by:  drose (24May00)
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
//     Function: InputDevice::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE InputDevice::
InputDevice() :
  _flags(0),
  _device_class(DC_unknown),
  _vendor_id(0),
  _product_id(0),
  _is_connected(false),
  _event_sequence(0),
  _enable_pointer_events(false),
  _battery_level(-1),
  _max_battery_level(-1)
{
  _button_events = new ButtonEventList;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::get_name
//       Access: Public
//  Description: Returns a human-readable name for the device.  May
//               not necessarily be unique.
////////////////////////////////////////////////////////////////////
INLINE string InputDevice::
get_name() const {
  LightMutexHolder holder(_lock);
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::get_manufacturer
//       Access: Public
//  Description: Returns a string containing the manufacturer of
//               the device, if this information is known.
////////////////////////////////////////////////////////////////////
INLINE string InputDevice::
get_manufacturer() const {
  LightMutexHolder holder(_lock);
  return _manufacturer;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::get_vendor_id
//       Access: Public
//  Description: Returns a string containing the USB vendor ID of
//               the device, if this information is known.
////////////////////////////////////////////////////////////////////
INLINE unsigned short InputDevice::
get_vendor_id() const {
  LightMutexHolder holder(_lock);
  return _vendor_id;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::get_product_id
//       Access: Public
//  Description: Returns a string containing the USB product ID of
//               the device, if this information is known.
////////////////////////////////////////////////////////////////////
INLINE unsigned short InputDevice::
get_product_id() const {
  LightMutexHolder holder(_lock);
  return _product_id;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::is_connected
//       Access: Public
//  Description: Returns true if the device is still connected and
//               able to receive data, false otherwise.  May return
//               false positives.
////////////////////////////////////////////////////////////////////
INLINE bool InputDevice::
is_connected() const {
  LightMutexHolder holder(_lock);
  return _is_connected;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::get_device_class
//       Access: Public
//  Description: Returns an identification of the general type of
//               device.  If this could not be determined, returns
//               DC_unknown.
////////////////////////////////////////////////////////////////////
INLINE InputDevice::DeviceClass InputDevice::
get_device_class() const {
  LightMutexHolder holder(_lock);
  return _device_class;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::has_pointer
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InputDevice::
has_pointer() const {
  LightMutexHolder holder(_lock);
  return ((_flags & IDF_has_pointer) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::has_keyboard
//       Access: Public
//  Description: Returns true if the device has a physical keyboard
//               designed for text entry.
////////////////////////////////////////////////////////////////////
INLINE bool InputDevice::
has_keyboard() const {
  LightMutexHolder holder(_lock);
  return ((_flags & IDF_has_keyboard) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::has_tracker
//       Access: Public
//  Description: Returns true if the device features a tracker that
//               can track position and/or orientation in 3D space.
////////////////////////////////////////////////////////////////////
INLINE bool InputDevice::
has_tracker() const {
  LightMutexHolder holder(_lock);
  return ((_flags & IDF_has_tracker) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::has_battery
//       Access: Public
//  Description: Returns true if the device may be able to provide
//               information about its battery life.
////////////////////////////////////////////////////////////////////
INLINE bool InputDevice::
has_battery() const {
  LightMutexHolder holder(_lock);
  return ((_flags & IDF_has_battery) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::has_axis
//       Access: Public
//  Description: Returns true if the device has the given axis.
////////////////////////////////////////////////////////////////////
/*
INLINE bool InputDevice::
has_axis(Axis axis) const {
  LightMutexHolder holder(_lock);
  return ((_has_axes & (1 << axis)) != 0);
}*/

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::get_pointer
//       Access: Public
//  Description: Returns the PointerData associated with the input
//               device's pointer.  This only makes sense if
//               has_pointer() also returns true.
////////////////////////////////////////////////////////////////////
INLINE PointerData InputDevice::
get_pointer() const {
  LightMutexHolder holder(_lock);
  return _pointer_data;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::get_tracker
//       Access: Public
//  Description: Returns the TrackerData associated with the input
//               device's tracker.  This only makes sense if
//               has_tracker() also returns true.
////////////////////////////////////////////////////////////////////
INLINE TrackerData InputDevice::
get_tracker() const {
  LightMutexHolder holder(_lock);
  return _tracker_data;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::get_battery_level
//       Access: Public
//  Description: Returns a rough indication of the battery level,
//               ranging from 0 (completely empty battery) to the
//               number reported by get_max_battery_level(), which
//               represents a full battery.
//               If this information is not known, returns -1.
////////////////////////////////////////////////////////////////////
INLINE short InputDevice::
get_battery_level() const {
  LightMutexHolder holder(_lock);
  return _battery_level;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::get_max_battery_level
//       Access: Public
//  Description: Returns the maximum value that may be reported by
//               get_battery_level(), representing a full battery.
//               Returns -1 if no battery information is known.
////////////////////////////////////////////////////////////////////
INLINE short InputDevice::
get_max_battery_level() const {
  LightMutexHolder holder(_lock);
  return _max_battery_level;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::get_num_buttons
//       Access: Public
//  Description: Returns the number of buttons known to the
//               ClientButtonDevice.  This includes those buttons
//               whose state has been seen, as well as buttons that
//               have been associated with a ButtonHandle even if
//               their state is unknown.  This number may change as
//               more buttons are discovered.
////////////////////////////////////////////////////////////////////
INLINE int InputDevice::
get_num_buttons() const {
  LightMutexHolder holder(_lock);
  return _buttons.size();
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::set_button_map
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
INLINE void InputDevice::
set_button_map(int index, ButtonHandle button) {
  LightMutexHolder holder(_lock);
  nassertv(index >= 0);
  if (index >= (int)_buttons.size()) {
    _buttons.resize(index + 1, ButtonState());
  }

  _buttons[index]._handle = button;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::get_button_map
//       Access: Public
//  Description: Returns the ButtonHandle that was previously
//               associated with the given index number by
//               a call to set_button_map(), or ButtonHandle::none()
//               if no button was associated.
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle InputDevice::
get_button_map(int index) const {
  if (index >= 0 && index < (int)_buttons.size()) {
    return _buttons[index]._handle;
  } else {
    return ButtonHandle::none();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::get_button_state
//       Access: Public
//  Description: Returns true if the indicated button (identified by
//               its index number) is currently known to be down, or
//               false if it is up or unknown.
////////////////////////////////////////////////////////////////////
INLINE bool InputDevice::
get_button_state(int index) const {
  if (index >= 0 && index < (int)_buttons.size()) {
    return (_buttons[index]._state == S_down);
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::is_button_known
//       Access: Public
//  Description: Returns true if the state of the indicated button is
//               known, or false if we have never heard anything about
//               this particular button.
////////////////////////////////////////////////////////////////////
INLINE bool InputDevice::
is_button_known(int index) const {
  if (index >= 0 && index < (int)_buttons.size()) {
    return _buttons[index]._state != S_unknown;
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::get_num_controls
//       Access: Public
//  Description: Returns the number of analog controls known to the
//               InputDevice.  This number may change as
//               more controls are discovered.
////////////////////////////////////////////////////////////////////
INLINE int InputDevice::
get_num_controls() const {
  return _controls.size();
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::set_control_map
//       Access: Public
//  Description: Associates the indicated ControlAxis with the
//               control of the indicated index number.  Pass
//               C_none to turn off any association.
//
//               It is not necessary to call this if you simply want
//               to query the state of the various controls by index
//               number.
////////////////////////////////////////////////////////////////////
INLINE void InputDevice::
set_control_map(int index, ControlAxis axis) {
  LightMutexHolder holder(_lock);
  nassertv(index >= 0);
  if (index >= (int)_controls.size()) {
    _controls.resize(index + 1, AnalogState());
  }

  _controls[index]._axis = axis;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::get_control_map
//       Access: Public
//  Description: Returns the ControlAxis that was previously
//               associated with the given index number by
//               a call to set_control_map(), or C_none
//               if no control was associated.
////////////////////////////////////////////////////////////////////
INLINE InputDevice::ControlAxis InputDevice::
get_control_map(int index) const {
  if (index >= 0 && index < (int)_controls.size()) {
    return _controls[index]._axis;
  } else {
    return C_none;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::get_control_state
//       Access: Public
//  Description: Returns the current position of indicated analog
//               control (identified by its index number), or 0.0 if
//               the control is unknown.  The normal range of a single
//               control is -1.0 to 1.0.
////////////////////////////////////////////////////////////////////
INLINE double InputDevice::
get_control_state(int index) const {
  if (index >= 0 && index < (int)_controls.size()) {
    return _controls[index]._state;
  } else {
    return 0.0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::is_control_known
//       Access: Public
//  Description: Returns true if the state of the indicated analog
//               control is known, or false if we have never heard
//               anything about this particular control.
////////////////////////////////////////////////////////////////////
INLINE bool InputDevice::
is_control_known(int index) const {
  if (index >= 0 && index < (int)_controls.size()) {
    return _controls[index]._known;
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::enable_pointer_events
//       Access: Public
//  Description: Enables the generation of mouse-movement events.
////////////////////////////////////////////////////////////////////
INLINE void InputDevice::
enable_pointer_events() {
  LightMutexHolder holder(_lock);
  _enable_pointer_events = true;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::disable_pointer_events
//       Access: Public
//  Description: Disables the generation of mouse-movement events.
////////////////////////////////////////////////////////////////////
INLINE void InputDevice::
disable_pointer_events() {
  LightMutexHolder holder(_lock);
  _enable_pointer_events = false;
  _pointer_events.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::set_connected
//       Access: Protected
//  Description: Called to indicate that the device has been
//               disconnected or connected from its host.
////////////////////////////////////////////////////////////////////
INLINE void InputDevice::
set_connected(bool connected) {
  LightMutexHolder holder(_lock);
  _is_connected = connected;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::operator ==
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InputDevice::
operator == (const InputDevice &) const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::operator !=
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InputDevice::
operator != (const InputDevice &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::operator <
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool InputDevice::
operator < (const InputDevice &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::ButtonState::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE InputDevice::ButtonState::
ButtonState() :
  _handle(ButtonHandle::none()),
  _state(S_unknown)
{
}

////////////////////////////////////////////////////////////////////
//     Function: InputDevice::AnalogState::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE InputDevice::AnalogState::
AnalogState() :
  _axis(C_none),
  _state(0.0),
  _known(false)
{
}
