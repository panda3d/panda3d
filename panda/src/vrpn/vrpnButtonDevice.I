// Filename: vrpnButtonDevice.I
// Created by:  drose (26Jan01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: VrpnButtonDevice::get_vrpn_button
//       Access: Public
//  Description: Returns a pointer to the particular VrpnButton this
//               device gets its data from.  This pointer may be
//               shared with other VrpnButtonDevice objects.
////////////////////////////////////////////////////////////////////
INLINE VrpnButton *VrpnButtonDevice::
get_vrpn_button() const {
  return _vrpn_button;
}
