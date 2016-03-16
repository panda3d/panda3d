/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnTrackerDevice.I
 * @author drose
 * @date 2001-01-25
 */

/**
 * Returns the particular sensor index that this device wants to hear about
 * from the VrpnTracker.
 */
INLINE int VrpnTrackerDevice::
get_sensor() const {
  return _sensor;
}

/**
 * Returns the type of data this device represents from the VrpnTracker.  This
 * may be position, velocity, or acceleration.
 */
INLINE VrpnTrackerDevice::DataType VrpnTrackerDevice::
get_data_type() const {
  return _data_type;
}

/**
 * Returns a pointer to the particular VrpnTracker this device gets its data
 * from.  This pointer may be shared with other VrpnTrackerDevice objects
 * (each representing a different portion of the tracker data).
 */
INLINE VrpnTracker *VrpnTrackerDevice::
get_vrpn_tracker() const {
  return _vrpn_tracker;
}
