// Filename: vrpnAnalogDevice.cxx
// Created by:  drose (26Jan01)
// 
////////////////////////////////////////////////////////////////////

#include "vrpnAnalogDevice.h"
#include "vrpnClient.h"

TypeHandle VrpnAnalogDevice::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VrpnAnalogDevice::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
VrpnAnalogDevice::
VrpnAnalogDevice(VrpnClient *client, const string &device_name,
                 VrpnAnalog *vrpn_analog) :
  ClientAnalogDevice(client, device_name),
  _vrpn_analog(vrpn_analog)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnAnalogDevice::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
VrpnAnalogDevice::
~VrpnAnalogDevice() {
  disconnect();
}
