// Filename: vrpnDialDevice.cxx
// Created by:  drose (26Jan01)
// 
////////////////////////////////////////////////////////////////////

#include "vrpnDialDevice.h"
#include "vrpnClient.h"

TypeHandle VrpnDialDevice::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: VrpnDialDevice::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
VrpnDialDevice::
VrpnDialDevice(VrpnClient *client, const string &device_name,
                 VrpnDial *vrpn_dial) :
  ClientDialDevice(client, device_name),
  _vrpn_dial(vrpn_dial)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnDialDevice::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
VrpnDialDevice::
~VrpnDialDevice() {
  disconnect();
}
