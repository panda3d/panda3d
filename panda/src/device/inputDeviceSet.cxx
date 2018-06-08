/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file inputDeviceSet.cxx
 * @author rdb
 * @date 2015-12-16
 */

#include "inputDeviceSet.h"
#include "indent.h"

/**
 *
 */
InputDeviceSet::
InputDeviceSet() {
}

/**
 *
 */
InputDeviceSet::
InputDeviceSet(const InputDeviceSet &copy) :
  _devices(copy._devices)
{
}

/**
 *
 */
void InputDeviceSet::
operator = (const InputDeviceSet &copy) {
  _devices = copy._devices;
}

/**
 * Adds a new InputDevice to the collection.
 */
void InputDeviceSet::
add_device(InputDevice *device) {
  _devices.insert(device);
}

/**
 * Removes the indicated InputDevice from the collection.
 * @return true if the device was removed, false if it was not a member.
 */
bool InputDeviceSet::
remove_device(InputDevice *device) {
  return _devices.erase(device) > 0;
}

/**
 * Adds all the InputDevices indicated in the other collection to this set.
 */
void InputDeviceSet::
add_devices_from(const InputDeviceSet &other) {
  size_t other_num_devices = other.size();
  for (size_t i = 0; i < other_num_devices; i++) {
    _devices.push_back(other[i]);
  }
  _devices.sort();
}

/**
 * Removes from this collection all of the devices listed in the other set.
 */
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

/**
 * Returns true if the indicated InputDevice appears in this collection, false
 * otherwise.
 */
bool InputDeviceSet::
has_device(InputDevice *device) const {
  InputDevices::const_iterator it = _devices.find(device);
  return (it != _devices.end());
}

/**
 * Removes all InputDevices from the collection.
 */
void InputDeviceSet::
clear() {
  _devices.clear();
}

/**
 * This is a hint to Panda to allocate enough memory to hold the given number
 * of InputDevices, if you know ahead of time how many you will be adding.
 */
void InputDeviceSet::
reserve(size_t num) {
  _devices.reserve(num);
}

/**
 * Writes a brief one-line description of the InputDeviceSet to the indicated
 * output stream.
 */
void InputDeviceSet::
output(std::ostream &out) const {
  if (_devices.size() == 1) {
    out << "1 input device";
  } else {
    out << _devices.size() << " input devices";
  }
}

/**
 * Writes a complete multi-line description of the InputDeviceSet to the
 * indicated output stream.
 */
void InputDeviceSet::
write(std::ostream &out, int indent_level) const {
  output(indent(out, indent_level));
  out << ":\n";
  indent_level += 2;

  InputDevices::const_iterator it;
  for (it = _devices.begin(); it != _devices.end(); ++it) {
    (*it)->output(indent(out, indent_level));
    out << '\n';
  }
}
