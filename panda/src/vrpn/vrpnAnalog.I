// Filename: vrpnAnalog.I
// Created by:  drose (26Jan01)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: VrpnAnalog::get_analog_name
//       Access: Public
//  Description: Returns the name of the analog device that was used
//               to create this VrpnAnalog.
////////////////////////////////////////////////////////////////////
INLINE const string &VrpnAnalog::
get_analog_name() const {
  return _analog_name;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnAnalog::is_empty
//       Access: Public
//  Description: Returns true if no VrpnAnalogDevices reference this
//               VrpnAnalog, or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool VrpnAnalog::
is_empty() const {
  return _devices.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnAnalog::poll
//       Access: Public
//  Description: Polls the connected device.  Normally you should not
//               call this directly; this will be called by the
//               VrpnClient.
////////////////////////////////////////////////////////////////////
INLINE void VrpnAnalog::
poll() {
  _analog->mainloop();
}
