// Filename: modifierButtons.I
// Created by:  drose (01Mar00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ModifierButtons::
operator = (const ModifierButtons &copy) {
  _button_list = copy._button_list;
  _state = copy._state;
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::Equality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ModifierButtons::
operator == (const ModifierButtons &other) const {
  return (_button_list == other._button_list &&
	  _state == other._state);
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::Inequality Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ModifierButtons::
operator != (const ModifierButtons &other) const {
  return !operator == (other);
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::Ordering Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool ModifierButtons::
operator < (const ModifierButtons &other) const {
  if (_button_list != other._button_list) {
    return _button_list < other._button_list;
  }
  return _state < other._state;
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::get_num_buttons
//       Access: Public
//  Description: Returns the number of buttons that the
//               ModifierButtons object is monitoring (e.g. the number
//               of buttons passed to add_button()).
////////////////////////////////////////////////////////////////////
INLINE int ModifierButtons::
get_num_buttons() const {
  return _button_list.size();
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::get_button
//       Access: Public
//  Description: Returns the nth button that the ModifierButtons
//               object is monitoring (the nth button passed to
//               add_button()).  This must be in the range 0 <= index
//               < get_num_buttons().
////////////////////////////////////////////////////////////////////
INLINE ButtonHandle ModifierButtons::
get_button(int index) const {
  nassertr(index >= 0 && index < (int)_button_list.size(), ButtonHandle::none());
  return _button_list[index];
}

////////////////////////////////////////////////////////////////////
//     Function: ModifierButtons::is_down
//       Access: Public
//  Description: Returns true if the indicated button is known to be
//               down, or false if it is known to be up.
////////////////////////////////////////////////////////////////////
INLINE bool ModifierButtons::
is_down(int index) const {
  nassertr(index >= 0 && index < (int)_button_list.size(), false);
  return ((_state & ((BitmaskType)1 << index)) != 0);
}
