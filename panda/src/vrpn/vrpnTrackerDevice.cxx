// Filename: vrpnTrackerDevice.cxx
// Created by:  drose (25Jan01)
// 
////////////////////////////////////////////////////////////////////

#include "vrpnTrackerDevice.h"
#include "vrpnClient.h"

TypeHandle VrpnTrackerDevice::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VrpnTrackerDevice::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
VrpnTrackerDevice::
VrpnTrackerDevice(VrpnClient *client, const string &device_name,
                  int sensor, VrpnTrackerDevice::DataType data_type,
                  VrpnTracker *vrpn_tracker) :
  ClientTrackerDevice(client, device_name),
  _sensor(sensor),
  _data_type(data_type),
  _vrpn_tracker(vrpn_tracker)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnTrackerDevice::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
VrpnTrackerDevice::
~VrpnTrackerDevice() {
  disconnect();
}
