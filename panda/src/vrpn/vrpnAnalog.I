/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnAnalog.I
 * @author drose
 * @date 2001-01-26
 */

/**
 * Returns the name of the analog device that was used to create this
 * VrpnAnalog.
 */
INLINE const std::string &VrpnAnalog::
get_analog_name() const {
  return _analog_name;
}

/**
 * Returns true if no VrpnAnalogDevices reference this VrpnAnalog, or false
 * otherwise.
 */
INLINE bool VrpnAnalog::
is_empty() const {
  return _devices.empty();
}

/**
 * Polls the connected device.  Normally you should not call this directly;
 * this will be called by the VrpnClient.
 */
INLINE void VrpnAnalog::
poll() {
  _analog->mainloop();
}
