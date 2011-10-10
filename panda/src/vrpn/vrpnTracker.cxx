// Filename: vrpnTracker.cxx
// Created by:  drose (25Jan01)
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

#include "vrpnTracker.h"
#include "vrpnTrackerDevice.h"
#include "vrpnClient.h"
#include "config_vrpn.h"

#include "indent.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: VrpnTracker::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
VrpnTracker::
VrpnTracker(const string &tracker_name, vrpn_Connection *connection) :
  _tracker_name(tracker_name)
{
  _tracker = new vrpn_Tracker_Remote(_tracker_name.c_str(), connection);

  _tracker->register_change_handler((void*)this, &vrpn_position_callback);
  _tracker->register_change_handler((void*)this, &vrpn_velocity_callback);
  _tracker->register_change_handler((void*)this, &vrpn_acceleration_callback);
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnTracker::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
VrpnTracker::
~VrpnTracker() {
  delete _tracker;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnTracker::mark
//       Access: Public
//  Description: Adds the indicated VrpnTrackerDevice to the list of
//               devices that are sharing this VrpnTracker.
////////////////////////////////////////////////////////////////////
void VrpnTracker::
mark(VrpnTrackerDevice *device) {
  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug() << *this << " marking " << *device << "\n";
  }
  _devices.push_back(device);
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnTracker::unmark
//       Access: Public
//  Description: Removes the indicated VrpnTrackerDevice from the list
//               of devices that are sharing this VrpnTracker.
////////////////////////////////////////////////////////////////////
void VrpnTracker::
unmark(VrpnTrackerDevice *device) {
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
//     Function: VrpnTracker::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void VrpnTracker::
output(ostream &out) const {
  out << _tracker_name;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnTracker::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void VrpnTracker::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_tracker_name() << " ("
    << _devices.size() << " devices)\n";
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnTracker::vrpn_position_callback
//       Access: Private, Static
//  Description: Receives the tracker positional data from the VRPN
//               code and sends it to any interested
//               VrpnTrackerDevices.
////////////////////////////////////////////////////////////////////
void VRPN_CALLBACK VrpnTracker::
vrpn_position_callback(void *userdata, const vrpn_TRACKERCB info) {
  VrpnTracker *self = (VrpnTracker *)userdata;
  if (vrpn_cat.is_spam()) {
    vrpn_cat.spam()
      << *self << " position_callback\n";
  }

  Devices::iterator di;
  for (di = self->_devices.begin(); di != self->_devices.end(); ++di) {
    VrpnTrackerDevice *device = (*di);
    if (device->get_sensor() == info.sensor &&
        device->get_data_type() == VrpnTrackerDevice::DT_position) {
      device->acquire();
      device->_data.set_time(VrpnClient::convert_to_secs(info.msg_time));
      device->_data.set_pos(LPoint3(info.pos[0], info.pos[1], info.pos[2]));
      device->_data.set_orient(LOrientation(info.quat[3], info.quat[0], info.quat[1], info.quat[2]));
      device->unlock();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnTracker::vrpn_velocity_callback
//       Access: Private, Static
//  Description: Receives the tracker velocity data from the VRPN
//               code and sends it to any interested
//               VrpnTrackerDevices.
////////////////////////////////////////////////////////////////////
void VRPN_CALLBACK VrpnTracker::
vrpn_velocity_callback(void *userdata, const vrpn_TRACKERVELCB info) {
  VrpnTracker *self = (VrpnTracker *)userdata;
  if (vrpn_cat.is_spam()) {
    vrpn_cat.spam()
      << *self << " velocity_callback\n";
  }

  Devices::iterator di;
  for (di = self->_devices.begin(); di != self->_devices.end(); ++di) {
    VrpnTrackerDevice *device = (*di);
    if (device->get_sensor() == info.sensor &&
        device->get_data_type() == VrpnTrackerDevice::DT_velocity) {
      device->acquire();
      device->_data.set_time(VrpnClient::convert_to_secs(info.msg_time));
      device->_data.set_pos(LPoint3(info.vel[0], info.vel[1], info.vel[2]));
      device->_data.set_orient(LOrientation(info.vel_quat[3], info.vel_quat[0],
                                             info.vel_quat[1], info.vel_quat[2]));
      device->_data.set_dt(info.vel_quat_dt);
      device->unlock();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnTracker::vrpn_acceleration_callback
//       Access: Private, Static
//  Description: Receives the tracker acceleration data from the VRPN
//               code and sends it to any interested
//               VrpnTrackerDevices.
////////////////////////////////////////////////////////////////////
void VRPN_CALLBACK VrpnTracker::
vrpn_acceleration_callback(void *userdata, const vrpn_TRACKERACCCB info) {
  VrpnTracker *self = (VrpnTracker *)userdata;
  if (vrpn_cat.is_spam()) {
    vrpn_cat.spam()
      << *self << " acceleration_callback\n";
  }

  Devices::iterator di;
  for (di = self->_devices.begin(); di != self->_devices.end(); ++di) {
    VrpnTrackerDevice *device = (*di);
    if (device->get_sensor() == info.sensor &&
        device->get_data_type() == VrpnTrackerDevice::DT_acceleration) {
      device->acquire();
      device->_data.set_time(VrpnClient::convert_to_secs(info.msg_time));
      device->_data.set_pos(LPoint3(info.acc[0], info.acc[1], info.acc[2]));
      device->_data.set_orient(LOrientation(info.acc_quat[3], info.acc_quat[0],
                                             info.acc_quat[1], info.acc_quat[2]));
      device->_data.set_dt(info.acc_quat_dt);
      device->unlock();
    }
  }
}
