/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnAnalog.cxx
 * @author drose
 * @date 2001-01-26
 */

#include "vrpnAnalog.h"
#include "vrpnAnalogDevice.h"
#include "vrpnClient.h"
#include "config_vrpn.h"

#include "indent.h"

#include <algorithm>

/**
 *
 */
VrpnAnalog::
VrpnAnalog(const std::string &analog_name, vrpn_Connection *connection) :
  _analog_name(analog_name)
{
  _analog = new vrpn_Analog_Remote(_analog_name.c_str(), connection);

  _analog->register_change_handler((void*)this, &vrpn_analog_callback);
}

/**
 *
 */
VrpnAnalog::
~VrpnAnalog() {
  delete _analog;
}

/**
 * Adds the indicated VrpnAnalogDevice to the list of devices that are sharing
 * this VrpnAnalog.
 */
void VrpnAnalog::
mark(VrpnAnalogDevice *device) {
  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug() << *this << " marking " << *device << "\n";
  }
  _devices.push_back(device);
}

/**
 * Removes the indicated VrpnAnalogDevice from the list of devices that are
 * sharing this VrpnAnalog.
 */
void VrpnAnalog::
unmark(VrpnAnalogDevice *device) {
  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug() << *this << " unmarking " << *device << "\n";
  }

  Devices::iterator di =
    find(_devices.begin(), _devices.end(), device);

  if (di != _devices.end()) {
    _devices.erase(di);
  }
}

/**
 *
 */
void VrpnAnalog::
output(std::ostream &out) const {
  out << _analog_name;
}

/**
 *
 */
void VrpnAnalog::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_analog_name() << " ("
    << _devices.size() << " devices)\n";
}

/**
 * Receives the analog event data from the VRPN code and sends it to any
 * interested VrpnAnalogDevices.
 */
void VRPN_CALLBACK VrpnAnalog::
vrpn_analog_callback(void *userdata, const vrpn_ANALOGCB info) {
  VrpnAnalog *self = (VrpnAnalog *)userdata;

  Devices::iterator di;
  for (di = self->_devices.begin(); di != self->_devices.end(); ++di) {
    VrpnAnalogDevice *device = (*di);
    for (int i = 0; i < info.num_channel; i++) {
      device->set_axis_value(i, info.channel[i]);
    }
  }
}
