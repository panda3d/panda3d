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
 * Returns true if the device is still connected to its ClientBase, false
 * otherwise.
 */
INLINE bool ClientDevice::
is_connected() const {
  return _is_connected;
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

/**
 * Returns the device name reported to the ClientBase.  This has some
 * implementation-defined meaning to identify particular devices.
 */
INLINE const string &ClientDevice::
get_device_name() const {
  return _device_name;
}

/**
 * Grabs the mutex associated with this particular device.  The device will
 * not update asynchronously while the mutex is held, allowing the user to
 * copy the data out without fear of getting a partial update during the copy.
 */
INLINE void ClientDevice::
acquire() {
#ifdef OLD_HAVE_IPC
  _lock.acquire();
#endif
}

/**
 * Releases the mutex associated with this particular device.  This should be
 * called after all the data has been successfully copied out.  See acquire().
 */
INLINE void ClientDevice::
unlock() {
#ifdef OLD_HAVE_IPC
  _lock.unlock();
#endif
}
