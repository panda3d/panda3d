/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnButton.cxx
 * @author drose
 * @date 2001-01-26
 */

#include "vrpnButton.h"
#include "vrpnButtonDevice.h"
#include "vrpnClient.h"
#include "config_vrpn.h"

#include "indent.h"

#include <algorithm>

/**
 *
 */
VrpnButton::
VrpnButton(const std::string &button_name, vrpn_Connection *connection) :
  _button_name(button_name)
{
  _button = new vrpn_Button_Remote(_button_name.c_str(), connection);

  _button->register_change_handler((void*)this, &vrpn_button_callback);
}

/**
 *
 */
VrpnButton::
~VrpnButton() {
  delete _button;
}

/**
 * Adds the indicated VrpnButtonDevice to the list of devices that are sharing
 * this VrpnButton.
 */
void VrpnButton::
mark(VrpnButtonDevice *device) {
  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug() << *this << " marking " << *device << "\n";
  }
  _devices.push_back(device);
}

/**
 * Removes the indicated VrpnButtonDevice from the list of devices that are
 * sharing this VrpnButton.
 */
void VrpnButton::
unmark(VrpnButtonDevice *device) {
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
void VrpnButton::
output(std::ostream &out) const {
  out << _button_name;
}

/**
 *
 */
void VrpnButton::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_button_name() << " ("
    << _devices.size() << " devices)\n";
}

/**
 * Receives the button event data from the VRPN code and sends it to any
 * interested VrpnButtonDevices.
 */
void VRPN_CALLBACK VrpnButton::
vrpn_button_callback(void *userdata, const vrpn_BUTTONCB info) {
  VrpnButton *self = (VrpnButton *)userdata;
  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << *self << " got button " << info.button << " = " << info.state << "\n";
  }

  Devices::iterator di;
  for (di = self->_devices.begin(); di != self->_devices.end(); ++di) {
    VrpnButtonDevice *device = (*di);
    device->button_changed(info.button, info.state != 0);
  }
}
