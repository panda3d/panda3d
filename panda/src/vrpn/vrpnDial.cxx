// Filename: vrpnDial.cxx
// Created by:  drose (26Jan01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "vrpnDial.h"
#include "vrpnDialDevice.h"
#include "vrpnClient.h"
#include "config_vrpn.h"

#include "indent.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: VrpnDial::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
VrpnDial::
VrpnDial(const string &dial_name, vrpn_Connection *connection) :
  _dial_name(dial_name)
{
  _dial = new vrpn_Dial_Remote(_dial_name.c_str(), connection);

  _dial->register_change_handler((void*)this, &vrpn_dial_callback);
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnDial::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
VrpnDial::
~VrpnDial() {
  delete _dial;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnDial::mark
//       Access: Public
//  Description: Adds the indicated VrpnDialDevice to the list of
//               devices that are sharing this VrpnDial.
////////////////////////////////////////////////////////////////////
void VrpnDial::
mark(VrpnDialDevice *device) {
  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug() << *this << " marking " << *device << "\n";
  }
  _devices.push_back(device);
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnDial::unmark
//       Access: Public
//  Description: Removes the indicated VrpnDialDevice from the list
//               of devices that are sharing this VrpnDial.
////////////////////////////////////////////////////////////////////
void VrpnDial::
unmark(VrpnDialDevice *device) {
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
//     Function: VrpnDial::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void VrpnDial::
output(ostream &out) const {
  out << _dial_name;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnDial::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void VrpnDial::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_dial_name() << " ("
    << _devices.size() << " devices)\n";
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnDial::vrpn_dial_callback
//       Access: Private, Static
//  Description: Receives the dial event data from the VRPN
//               code and sends it to any interested
//               VrpnDialDevices.
////////////////////////////////////////////////////////////////////
void VrpnDial::
vrpn_dial_callback(void *userdata, const vrpn_DIALCB info) {
  VrpnDial *self = (VrpnDial *)userdata;

  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug()
      << *self << " got dial " << info.dial << " = " << info.change << "\n";
  }

  Devices::iterator di;
  for (di = self->_devices.begin(); di != self->_devices.end(); ++di) {
    VrpnDialDevice *device = (*di);
    device->lock();
    device->push_dial(info.dial, info.change);
    device->unlock();
  }
}
