/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnButton.I
 * @author drose
 * @date 2001-01-26
 */

/**
 * Returns the name of the button device that was used to create this
 * VrpnButton.
 */
INLINE const std::string &VrpnButton::
get_button_name() const {
  return _button_name;
}

/**
 * Returns true if no VrpnButtonDevices reference this VrpnButton, or false
 * otherwise.
 */
INLINE bool VrpnButton::
is_empty() const {
  return _devices.empty();
}

/**
 * Polls the connected device.  Normally you should not call this directly;
 * this will be called by the VrpnClient.
 */
INLINE void VrpnButton::
poll() {
  _button->mainloop();
}
