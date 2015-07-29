// Filename: graphicsWindowInputDevice.I
// Created by:  drose (24May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::Default Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GraphicsWindowInputDevice::
GraphicsWindowInputDevice() {
  LightMutexHolder holder(_lock);
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::get_name
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE string GraphicsWindowInputDevice::
get_name() const {
  LightMutexHolder holder(_lock);
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::has_pointer
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindowInputDevice::
has_pointer() const {
  LightMutexHolder holder(_lock);
  return ((_flags & IDF_has_pointer) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::has_keyboard
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindowInputDevice::
has_keyboard() const {
  LightMutexHolder holder(_lock);
  return ((_flags & IDF_has_keyboard) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::get_pointer
//       Access: Public
//  Description: Returns the MouseData associated with the input
//               device's pointer.
////////////////////////////////////////////////////////////////////
INLINE  MouseData GraphicsWindowInputDevice::
get_pointer() const {
  LightMutexHolder holder(_lock);
  return _mouse_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::get_raw_pointer
//       Access: Public
//  Description: Returns the MouseData associated with the input
//               device's pointer, in raw form (ie, prior to any
//               pointer_mode interpretation).
////////////////////////////////////////////////////////////////////
INLINE  MouseData GraphicsWindowInputDevice::
get_raw_pointer() const {
  LightMutexHolder holder(_lock);
  return _true_mouse_data;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::set_device_index
//       Access: Public
//  Description: Set the device index.  This is reported in pointer
//               events.  The device index will be equal to the position
//               of the GraphicsWindowInputDevice in the window's list.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
set_device_index(int index) {
  LightMutexHolder holder(_lock);
  _device_index = index;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::enable_pointer_events
//       Access: Public
//  Description: Enables the generation of mouse-movement events.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
enable_pointer_events() {
  LightMutexHolder holder(_lock);
  _enable_pointer_events = true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::disable_pointer_events
//       Access: Public
//  Description: Disables the generation of mouse-movement events.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
disable_pointer_events() {
  LightMutexHolder holder(_lock);
  _enable_pointer_events = false;
  _pointer_events.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::button_down
//       Access: Published
//  Description: Records that the indicated button has been depressed.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
button_down(ButtonHandle button) {
  button_down(button, ClockObject::get_global_clock()->get_frame_time());
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::button_resume_down
//       Access: Published
//  Description: Records that the indicated button was depressed
//               earlier, and we only just detected the event after
//               the fact.  This is mainly useful for tracking the
//               state of modifier keys.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
button_resume_down(ButtonHandle button) {
  button_resume_down(button, ClockObject::get_global_clock()->get_frame_time());
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::button_up
//       Access: Published
//  Description: Records that the indicated button has been released.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
button_up(ButtonHandle button) {
  button_up(button, ClockObject::get_global_clock()->get_frame_time());
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::keystroke
//       Access: Published
//  Description: Records that the indicated keystroke has been
//               generated.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
keystroke(int keycode) {
  keystroke(keycode, ClockObject::get_global_clock()->get_frame_time());
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::focus_lost
//       Access: Published
//  Description: This should be called when the window focus is lost,
//               so that we may miss upcoming button events
//               (especially "up" events) for the next period of time.
//               It generates keyboard and mouse "up" events for those
//               buttons that we previously sent unpaired "down"
//               events, so that the Panda application will believe
//               all buttons are now released.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
focus_lost() {
  focus_lost(ClockObject::get_global_clock()->get_frame_time());
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::raw_button_down
//       Access: Published
//  Description: Records that the indicated button has been depressed.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
raw_button_down(ButtonHandle button) {
  raw_button_down(button, ClockObject::get_global_clock()->get_frame_time());
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::raw_button_up
//       Access: Published
//  Description: Records that the indicated button has been released.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
raw_button_up(ButtonHandle button) {
  raw_button_up(button, ClockObject::get_global_clock()->get_frame_time());
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::set_pointer_in_window
//       Access: Published
//  Description: To be called by a particular kind of GraphicsWindow
//               to indicate that the pointer is within the window, at
//               the given pixel coordinates.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
set_pointer_in_window(double x, double y) {
  // mutex is handled in set pointer .. convience function
  set_pointer(true, x, y, ClockObject::get_global_clock()->get_frame_time());
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::set_pointer_out_of_window
//       Access: Published
//  Description: To be called by a particular kind of GraphicsWindow
//               to indicate that the pointer is no longer within the
//               window.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
set_pointer_out_of_window() {
 // mutex is handled in set pointer .. convience function
  set_pointer(false, _mouse_data._xpos, _mouse_data._ypos,
              ClockObject::get_global_clock()->get_frame_time());
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::set_pointer_in_window
//       Access: Published
//  Description: To be called by a particular kind of GraphicsWindow
//               to indicate that the pointer is within the window, at
//               the given pixel coordinates.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
set_pointer_in_window(double x, double y, double time) {
 // mutex is handled in set pointer .. convience function
  set_pointer(true, x, y, time);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindowInputDevice::set_pointer_out_of_window
//       Access: Published
//  Description: To be called by a particular kind of GraphicsWindow
//               to indicate that the pointer is no longer within the
//               window.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindowInputDevice::
set_pointer_out_of_window(double time) {
 // mutex is handled in set pointer .. convience function
  set_pointer(false, _mouse_data._xpos, _mouse_data._ypos, time);
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
