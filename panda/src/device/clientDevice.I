/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file clientDevice.I
 * @author drose
 * @date 2001-01-25
 */

/**
 * Returns the ClientBase this device is associated with.
 */
INLINE ClientBase *ClientDevice::
get_client() const {
  return _client;
}

/**
 * Returns the type of device this is considered to be to the ClientBase: a
 * ClientTrackerDevice, ClientAnalogDevice, or what have you.  This is not
 * exactly the same thing as get_type(), because it does not return the exact
 * type of the ClientDevice (e.g.  it reports ClientTrackerDevice, not
 * VrpnTrackerDevice).
 */
INLINE TypeHandle ClientDevice::
get_device_type() const {
  return _device_type;
}
