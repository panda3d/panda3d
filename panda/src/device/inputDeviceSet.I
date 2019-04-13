/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file inputDeviceSet.I
 * @author rdb
 * @date 2015-12-16
 */

/**
 *
 */
INLINE InputDeviceSet::
~InputDeviceSet() {
}

/**
 * Returns the nth InputDevice in the collection.
 */
INLINE InputDevice *InputDeviceSet::
operator [] (size_t index) const {
  nassertr(index < _devices.size(), nullptr);
  return _devices[index];
}

/**
 * Returns the number of devices in the collection.
 */
INLINE size_t InputDeviceSet::
size() const {
  return _devices.size();
}
