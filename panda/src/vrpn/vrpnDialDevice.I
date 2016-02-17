/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnDialDevice.I
 * @author drose
 * @date 2001-01-26
 */

/**
 * Returns a pointer to the particular VrpnDial this device gets its data
 * from.  This pointer may be shared with other VrpnDialDevice objects.
 */
INLINE VrpnDial *VrpnDialDevice::
get_vrpn_dial() const {
  return _vrpn_dial;
}
