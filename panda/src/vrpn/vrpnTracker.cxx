/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnTracker.cxx
 * @author drose
 * @date 2001-01-25
 */

#include "vrpnTracker.h"
#include "vrpnTrackerDevice.h"
#include "vrpnClient.h"
#include "config_vrpn.h"

#include "indent.h"

#include <algorithm>

/**
 *
 */
VrpnTracker::
VrpnTracker(const std::string &tracker_name, vrpn_Connection *connection) :
  _tracker_name(tracker_name)
{
  _tracker = new vrpn_Tracker_Remote(_tracker_name.c_str(), connection);

  _tracker->register_change_handler((void*)this, &vrpn_position_callback);
  _tracker->register_change_handler((void*)this, &vrpn_velocity_callback);
  _tracker->register_change_handler((void*)this, &vrpn_acceleration_callback);
}

/**
 *
 */
VrpnTracker::
~VrpnTracker() {
  delete _tracker;
}

/**
 * Adds the indicated VrpnTrackerDevice to the list of devices that are
 * sharing this VrpnTracker.
 */
void VrpnTracker::
mark(VrpnTrackerDevice *device) {
  if (vrpn_cat.is_debug()) {
    vrpn_cat.debug() << *this << " marking " << *device << "\n";
  }
  _devices.push_back(device);
}

/**
 * Removes the indicated VrpnTrackerDevice from the list of devices that are
 * sharing this VrpnTracker.
 */
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

/**
 *
 */
void VrpnTracker::
output(std::ostream &out) const {
  out << _tracker_name;
}

/**
 *
 */
void VrpnTracker::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_tracker_name() << " ("
    << _devices.size() << " devices)\n";
}

/**
 * Receives the tracker positional data from the VRPN code and sends it to any
 * interested VrpnTrackerDevices.
 */
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
      device->tracker_changed(LPoint3(info.pos[0], info.pos[1], info.pos[2]),
                              LOrientation(info.quat[3], info.quat[0],
                                           info.quat[1], info.quat[2]),
                              VrpnClient::convert_to_secs(info.msg_time));
    }
  }
}

/**
 * Receives the tracker velocity data from the VRPN code and sends it to any
 * interested VrpnTrackerDevices.
 */
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
      device->tracker_changed(LPoint3(info.vel[0], info.vel[1], info.vel[2]),
                              LOrientation(info.vel_quat[3], info.vel_quat[0],
                                           info.vel_quat[1], info.vel_quat[2]),
                              VrpnClient::convert_to_secs(info.msg_time));
    }
  }
}

/**
 * Receives the tracker acceleration data from the VRPN code and sends it to
 * any interested VrpnTrackerDevices.
 */
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
      device->tracker_changed(LPoint3(info.acc[0], info.acc[1], info.acc[2]),
                              LOrientation(info.acc_quat[3], info.acc_quat[0],
                                           info.acc_quat[1], info.acc_quat[2]),
                              VrpnClient::convert_to_secs(info.msg_time));
    }
  }
}
