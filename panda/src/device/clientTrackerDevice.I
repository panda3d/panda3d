// Filename: clientTrackerDevice.I
// Created by:  drose (25Jan01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ClientTrackerDevice::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ClientTrackerDevice::
ClientTrackerDevice(ClientBase *client, const string &device_name):
  ClientDevice(client, get_class_type(), device_name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ClientTrackerDevice::get_data
//       Access: Public
//  Description: Returns the TrackerData that this device is
//               reporting.
////////////////////////////////////////////////////////////////////
INLINE const TrackerData &ClientTrackerDevice::
get_data() const {
  return _data;
}

