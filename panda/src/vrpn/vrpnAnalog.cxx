// Filename: vrpnAnalog.cxx
// Created by:  drose (26Jan01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "vrpnAnalog.h"
#include "vrpnAnalogDevice.h"
#include "vrpnClient.h"
#include "config_vrpn.h"

#include "indent.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: VrpnAnalog::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
VrpnAnalog::
VrpnAnalog(const string &analog_name, vrpn_Connection *connection) :
  _analog_name(analog_name)
{
  _analog = new vrpn_Analog_Remote(_analog_name.c_str(), connection);

  _analog->register_change_handler((void*)this, &vrpn_analog_callback);
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnAnalog::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
VrpnAnalog::
~VrpnAnalog() {
  delete _analog;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnAnalog::mark
//       Access: Public
//  Description: Adds the indicated VrpnAnalogDevice to the list of
//               devices that are sharing this VrpnAnalog.
////////////////////////////////////////////////////////////////////
void VrpnAnalog::
mark(VrpnAnalogDevice *device) {
  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug() << *this << " marking " << *device << "\n";
  }
  _devices.push_back(device);
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnAnalog::unmark
//       Access: Public
//  Description: Removes the indicated VrpnAnalogDevice from the list
//               of devices that are sharing this VrpnAnalog.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: VrpnAnalog::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void VrpnAnalog::
output(ostream &out) const {
  out << _analog_name;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnAnalog::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void VrpnAnalog::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_analog_name() << " ("
    << _devices.size() << " devices)\n";
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnAnalog::vrpn_analog_callback
//       Access: Private, Static
//  Description: Receives the analog event data from the VRPN
//               code and sends it to any interested
//               VrpnAnalogDevices.
////////////////////////////////////////////////////////////////////
void VRPN_CALLBACK VrpnAnalog::
vrpn_analog_callback(void *userdata, const vrpn_ANALOGCB info) {
  VrpnAnalog *self = (VrpnAnalog *)userdata;

  Devices::iterator di;
  for (di = self->_devices.begin(); di != self->_devices.end(); ++di) {
    VrpnAnalogDevice *device = (*di);
    device->acquire();
    for (int i = 0; i < info.num_channel; i++) {
      if (vrpn_cat.is_debug()) {
        if (device->get_control_state(i) != info.channel[i]) {
          vrpn_cat.debug()
            << *self << " got analog " << i << " = " << info.channel[i] << "\n";
        }
      }
      device->set_control_state(i, info.channel[i]);
    }
    device->unlock();
  }
}
