/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnTracker.I
 * @author drose
 * @date 2001-01-25
 */

/**
 * Returns the name of the tracker device that was used to create this
 * VrpnTracker.
 */
INLINE const std::string &VrpnTracker::
get_tracker_name() const {
  return _tracker_name;
}

/**
 * Returns true if no VrpnTrackerDevices reference this VrpnTracker, or false
 * otherwise.
 */
INLINE bool VrpnTracker::
is_empty() const {
  return _devices.empty();
}

/**
 * Polls the connected device.  Normally you should not call this directly;
 * this will be called by the VrpnClient.
 */
INLINE void VrpnTracker::
poll() {
  _tracker->mainloop();
}
