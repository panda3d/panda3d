/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vrpnTrackerDevice.cxx
 * @author drose
 * @date 2001-01-25
 */

#include "vrpnTrackerDevice.h"
#include "vrpnClient.h"

TypeHandle VrpnTrackerDevice::_type_handle;

/**
 *
 */
VrpnTrackerDevice::
VrpnTrackerDevice(VrpnClient *client, const std::string &device_name,
                  int sensor, VrpnTrackerDevice::DataType data_type,
                  VrpnTracker *vrpn_tracker) :
  ClientTrackerDevice(client, device_name),
  _sensor(sensor),
  _data_type(data_type),
  _vrpn_tracker(vrpn_tracker)
{
}

/**
 *
 */
VrpnTrackerDevice::
~VrpnTrackerDevice() {
  disconnect();
}
