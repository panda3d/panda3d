// Filename: vrpnButtonDevice.cxx
// Created by:  drose (26Jan01)
// 
////////////////////////////////////////////////////////////////////

#include "vrpnButtonDevice.h"
#include "vrpnClient.h"

TypeHandle VrpnButtonDevice::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VrpnButtonDevice::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
VrpnButtonDevice::
VrpnButtonDevice(VrpnClient *client, const string &device_name,
                 VrpnButton *vrpn_button) :
  ClientButtonDevice(client, device_name),
  _vrpn_button(vrpn_button)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnButtonDevice::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
VrpnButtonDevice::
~VrpnButtonDevice() {
  disconnect();
}
