// Filename: buttonNode.I
// Created by:  drose (31Dec69)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ButtonNode::is_valid
//       Access: Public
//  Description: Returns true if the ButtonNode is valid and
//               connected to a server, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool ButtonNode::
is_valid() const {
  return (_button != (ClientButtonDevice *)NULL) && _button->is_connected();
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonNode::get_num_buttons
//       Access: Public
//  Description: Returns the number of buttons known to the
//               ButtonNode.  This includes those buttons whose state
//               has been seen, as well as buttons that have been
//               associated with a ButtonHandle even if their state is
//               unknown.  This number may change as more buttons are
//               discovered.
////////////////////////////////////////////////////////////////////
INLINE int ButtonNode::
get_num_buttons() const {
  return _button->get_num_buttons();
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonNode::set_button_map
//       Access: Public
//  Description: Associates the indicated ButtonHandle with the button
//               of the indicated index number.  When the given button
//               index changes state, a corresponding ButtonEvent will
//               be generated with the given ButtonHandle.  Pass
//               ButtonHandle::none() to turn off any association.
//
//               It is not necessary to call this if you simply want
//               to query the state of the various buttons by index
//               number; this is only necessary in order to generate
//               ButtonEvents when the buttons change state.
////////////////////////////////////////////////////////////////////
INLINE void ButtonNode::
set_button_map(int index, ButtonHandle button) {
  _button->set_button_map(index, button);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonNode::get_button_map
//       Access: Public
//  Description: Returns the ButtonHandle that was previously
//               associated with the given index number by
//               a call to set_button_map(), or ButtonHandle::none()
//               if no button was associated.
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle ButtonNode::
get_button_map(int index) const {
  return _button->get_button_map(index);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonNode::get_button_state
//       Access: Public
//  Description: Returns true if the indicated button (identified by
//               its index number) is currently known to be down, or
//               false if it is up or unknown.
////////////////////////////////////////////////////////////////////
INLINE bool ButtonNode::
get_button_state(int index) const {
  return _button->get_button_state(index);
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonNode::is_button_known
//       Access: Public
//  Description: Returns true if the state of the indicated button is
//               known, or false if we have never heard anything about
//               this particular button.
////////////////////////////////////////////////////////////////////
INLINE bool ButtonNode::
is_button_known(int index) const {
  return _button->is_button_known(index);
}
