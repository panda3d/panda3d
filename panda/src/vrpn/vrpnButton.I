// Filename: vrpnButton.I
// Created by:  drose (26Jan01)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: VrpnButton::get_button_name
//       Access: Public
//  Description: Returns the name of the button device that was used
//               to create this VrpnButton.
////////////////////////////////////////////////////////////////////
INLINE const string &VrpnButton::
get_button_name() const {
  return _button_name;
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnButton::is_empty
//       Access: Public
//  Description: Returns true if no VrpnButtonDevices reference this
//               VrpnButton, or false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool VrpnButton::
is_empty() const {
  return _devices.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: VrpnButton::poll
//       Access: Public
//  Description: Polls the connected device.  Normally you should not
//               call this directly; this will be called by the
//               VrpnClient.
////////////////////////////////////////////////////////////////////
INLINE void VrpnButton::
poll() {
  _button->mainloop();
}
