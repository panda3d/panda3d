// Filename: graphicsWindowInputDevice.I
// Created by:  drose (24May00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::get_name
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE string GraphicsWindowInputDevice::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::has_pointer
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindowInputDevice::
has_pointer() const {
  return ((_flags & IDF_has_pointer) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::has_keyboard
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindowInputDevice::
has_keyboard() const {
  return ((_flags & IDF_has_keyboard) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::get_mouse_data
//       Access: Public
//  Description: Returns the MouseData associated with the nth input
//               device.
////////////////////////////////////////////////////////////////////
INLINE const MouseData &GraphicsWindowInputDevice::
get_mouse_data() const {
  return _mouse_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::get_modifier_buttons
//       Access: Public
//  Description: Returns the set of ModifierButtons currently being
//               monitored for the nth input device.
////////////////////////////////////////////////////////////////////
INLINE const ModifierButtons &GraphicsWindowInputDevice::
get_modifier_buttons() const {
  return _mods;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::set_modifier_buttons
//       Access: Public
//  Description: Changes the set of ModifierButtons that will be
//               monitored for the nth input device.  It is
//               recommended that you first retrieve the existing set
//               via get_modifier_buttons(), so that any current
//               button state will not be lost.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
set_modifier_buttons(const ModifierButtons &mods) {
  _mods = mods;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::set_pointer_in_window
//       Access: Public
//  Description: To be called by a particular kind of GraphicsWindow
//               to indicate that the pointer is within the window, at
//               the given pixel coordinates.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
set_pointer_in_window(int x, int y) {
  _mouse_data._in_window = true;
  _mouse_data._xpos = x;
  _mouse_data._ypos = y;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::set_pointer_out_of_window
//       Access: Public
//  Description: To be called by a particular kind of GraphicsWindow
//               to indicate that the pointer is no longer within the
//               window.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
set_pointer_out_of_window() {
  _mouse_data._in_window = false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::operator == 
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindowInputDevice::
operator == (const GraphicsWindowInputDevice &) const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::operator != 
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindowInputDevice::
operator != (const GraphicsWindowInputDevice &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindowInputDevice::
operator < (const GraphicsWindowInputDevice &) const {
  return false;
}
