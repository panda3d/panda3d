// Filename: vrpnDialDevice.I
// Created by:  drose (26Jan01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: VrpnDialDevice::get_vrpn_dial
//       Access: Public
//  Description: Returns a pointer to the particular VrpnDial this
//               device gets its data from.  This pointer may be
//               shared with other VrpnDialDevice objects.
////////////////////////////////////////////////////////////////////
INLINE VrpnDial *VrpnDialDevice::
get_vrpn_dial() const {
  return _vrpn_dial;
}
