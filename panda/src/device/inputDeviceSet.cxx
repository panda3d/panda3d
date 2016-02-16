// Filename: inputDeviceSet.cxx
// Created by:  rdb (16Dec15)
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

#include "inputDeviceSet.h"
#include "indent.h"

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceSet::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
InputDeviceSet::
InputDeviceSet() {
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceSet::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
InputDeviceSet::
InputDeviceSet(const InputDeviceSet &copy) :
  _devices(copy._devices)
{
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceSet::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void InputDeviceSet::
operator = (const InputDeviceSet &copy) {
  _devices = copy._devices;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceSet::add_device
//       Access: Public
//  Description: Adds a new InputDevice to the collection.
////////////////////////////////////////////////////////////////////
void InputDeviceSet::
add_device(InputDevice *device) {
  _devices.insert(device);
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceSet::remove_device
//       Access: Public
//  Description: Removes the indicated InputDevice from the collection.
//               Returns true if the device was removed, false if it was
//               not a member of the collection.
////////////////////////////////////////////////////////////////////
bool InputDeviceSet::
remove_device(InputDevice *device) {
  return _devices.erase(device) > 0;
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceSet::add_devices_from
//       Access: Public
//  Description: Adds all the InputDevices indicated in the other
//               collection to this device.
////////////////////////////////////////////////////////////////////
void InputDeviceSet::
add_devices_from(const InputDeviceSet &other) {
  size_t other_num_devices = other.size();
  for (size_t i = 0; i < other_num_devices; i++) {
    _devices.push_back(other[i]);
  }
  _devices.sort();
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceSet::remove_devices_from
//       Access: Public
//  Description: Removes from this collection all of the InputDevices
//               listed in the other collection.
////////////////////////////////////////////////////////////////////
void InputDeviceSet::
remove_devices_from(const InputDeviceSet &other) {
  InputDevices new_devices;
  InputDevices::const_iterator it;
  for (it = _devices.begin(); it != _devices.end(); ++it) {
    if (!other.has_device(*it)) {
      new_devices.push_back(*it);
    }
  }
  new_devices.sort();
  _devices.swap(new_devices);
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceSet::has_device
//       Access: Public
//  Description: Returns true if the indicated InputDevice appears in
//               this collection, false otherwise.
////////////////////////////////////////////////////////////////////
bool InputDeviceSet::
has_device(InputDevice *device) const {
  InputDevices::const_iterator it = _devices.find(device);
  return (it != _devices.end());
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceSet::clear
//       Access: Published
//  Description: Removes all InputDevices from the collection.
////////////////////////////////////////////////////////////////////
void InputDeviceSet::
clear() {
  _devices.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceSet::reserve
//       Access: Published
//  Description: This is a hint to Panda to allocate enough memory
//               to hold the given number of NodePaths, if you know
//               ahead of time how many you will be adding.
////////////////////////////////////////////////////////////////////
void InputDeviceSet::
reserve(size_t num) {
  _devices.reserve(num);
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceSet::output
//       Access: Published
//  Description: Writes a brief one-line description of the
//               InputDeviceSet to the indicated output stream.
////////////////////////////////////////////////////////////////////
void InputDeviceSet::
output(ostream &out) const {
  if (_devices.size() == 1) {
    out << "1 input device";
  } else {
    out << _devices.size() << " input devices";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: InputDeviceSet::write
//       Access: Published
//  Description: Writes a complete multi-line description of the
//               InputDeviceSet to the indicated output stream.
////////////////////////////////////////////////////////////////////
void InputDeviceSet::
write(ostream &out, int indent_level) const {
  output(indent(out, indent_level));
  out << ":\n";
  indent_level += 2;

  InputDevices::const_iterator it;
  for (it = _devices.begin(); it != _devices.end(); ++it) {
    (*it)->output(indent(out, indent_level));
    out << '\n';
  }
}
