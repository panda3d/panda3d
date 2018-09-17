/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnDial.I
 * @author drose
 * @date 2001-01-26
 */

/**
 * Returns the name of the dial device that was used to create this VrpnDial.
 */
INLINE const std::string &VrpnDial::
get_dial_name() const {
  return _dial_name;
}

/**
 * Returns true if no VrpnDialDevices reference this VrpnDial, or false
 * otherwise.
 */
INLINE bool VrpnDial::
is_empty() const {
  return _devices.empty();
}

/**
 * Polls the connected device.  Normally you should not call this directly;
 * this will be called by the VrpnClient.
 */
INLINE void VrpnDial::
poll() {
  _dial->mainloop();
}
