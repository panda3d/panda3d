// Filename: graphicsWindow.I
// Created by:  frang (07Mar99)
// 
////////////////////////////////////////////////////////////////////

#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_properties
//       Access: Public
//  Description: Returns the full Properties structure that describes
//               the window.
////////////////////////////////////////////////////////////////////
INLINE const GraphicsWindow::Properties &GraphicsWindow::
get_properties() const {
  return _props;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_width
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int GraphicsWindow::
get_width() const {
  return (_props._xsize);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_height
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int GraphicsWindow::
get_height() const {
  return (_props._ysize);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_xorg
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int GraphicsWindow::
get_xorg() const {
  return (_props._xorg);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_yorg
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int GraphicsWindow::
get_yorg() const {
  return (_props._yorg);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_gsg
//       Access: Public
//  Description: Returns the GSG that is associated with this window.
//               There is a one-to-one association between windows and
//               GSG's.
////////////////////////////////////////////////////////////////////
INLINE GraphicsStateGuardian *GraphicsWindow::
get_gsg() const {
  return _gsg;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_pipe
//       Access: Public
//  Description: Returns the GraphicsPipe that this window is
//               associated with.  It is possible that the
//               GraphicsPipe might have been deleted while an
//               outstanding PT(GraphicsWindow) prevented all of its
//               children windows from also being deleted; in this
//               unlikely case, get_pipe() may return NULL.
////////////////////////////////////////////////////////////////////
INLINE GraphicsPipe *GraphicsWindow::
get_pipe() const {
  return _pipe;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_num_input_devices
//       Access: Public
//  Description: Returns the number of separate input devices
//               associated with the window.  Typically, a window will
//               have exactly one input device: the keyboard/mouse
//               pair.  However, some windows may have no input
//               devices, and others may add additional devices, for
//               instance for a joystick.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsWindow::
get_num_input_devices() const {
  return _input_devices.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_input_device_name
//       Access: Public
//  Description: Returns the name of the nth input device.
////////////////////////////////////////////////////////////////////
INLINE string GraphicsWindow::
get_input_device_name(int device) const {
  nassertr(device >= 0 && device < (int)_input_devices.size(), "");
  return _input_devices[device].get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::has_pointer
//       Access: Public
//  Description: Returns true if the nth input device has a
//               screen-space pointer (for instance, a mouse), false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindow::
has_pointer(int device) const {
  nassertr(device >= 0 && device < (int)_input_devices.size(), false);
  return _input_devices[device].has_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::has_keyboard
//       Access: Public
//  Description: Returns true if the nth input device has a keyboard,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindow::
has_keyboard(int device) const {
  nassertr(device >= 0 && device < (int)_input_devices.size(), false);
  return _input_devices[device].has_keyboard();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_modifier_buttons
//       Access: Public
//  Description: Returns the set of ModifierButtons currently being
//               monitored for the nth input device.
////////////////////////////////////////////////////////////////////
INLINE const ModifierButtons &GraphicsWindow::
get_modifier_buttons(int device) const {
  nassertr(device >= 0 && device < (int)_input_devices.size(), 
	   *(new ModifierButtons));
  return _input_devices[device].get_modifier_buttons();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::set_modifier_buttons
//       Access: Public
//  Description: Changes the set of ModifierButtons that will be
//               monitored for the nth input device.  It is
//               recommended that you first retrieve the existing set
//               via get_modifier_buttons(), so that any current
//               button state will not be lost.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::
set_modifier_buttons(int device, const ModifierButtons &mods) {
  nassertv(device >= 0 && device < (int)_input_devices.size());
  _input_devices[device].set_modifier_buttons(mods);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_mouse_data
//       Access: Public
//  Description: Returns the MouseData associated with the nth input
//               device.
////////////////////////////////////////////////////////////////////
INLINE const MouseData &GraphicsWindow::
get_mouse_data(int device) const {
  nassertr(device >= 0 && device < (int)_input_devices.size(), 
	   *(new MouseData));
  return _input_devices[device].get_mouse_data();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::has_button_event
//       Access: Public
//  Description: Returns true if the indicated device has a pending
//               button event (a mouse button or keyboard button
//               down/up), false otherwise.  If this returns true, the
//               particular event may be extracted via
//               get_button_event().
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindow::
has_button_event(int device) const {
  nassertr(device >= 0 && device < (int)_input_devices.size(), false);
  return _input_devices[device].has_button_event();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_button_event
//       Access: Public
//  Description: Assuming a previous call to has_button_event()
//               returned true, this returns the pending button event.
////////////////////////////////////////////////////////////////////
INLINE ButtonEvent GraphicsWindow::
get_button_event(int device) {
  nassertr(has_button_event(device), ButtonEvent());
  return _input_devices[device].get_button_event();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::set_frame_number
//       Access: Public
//  Description: Sets the current frame number of the window.  This
//               affects the frame numbers written for %f in a RIB or
//               image filename template.  The frame number is
//               initially zero, and it increments at each call to
//               end_frame().
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::
set_frame_number(const int f) {
  _frame_number = f;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_frame_number
//       Access: Public
//  Description: Returns the current frame number of the window.  See
//               set_frame_number().
////////////////////////////////////////////////////////////////////
INLINE int GraphicsWindow::
get_frame_number() const {
  return _frame_number;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::set_draw_callback
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::
set_draw_callback(Callback *c) {
  _draw_callback = c;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::set_idle_callback
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::
set_idle_callback(Callback *c) {
  _idle_callback = c;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::call_draw_callback
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::
call_draw_callback(bool force_redraw) {
  if (_draw_callback) {
    _draw_callback->draw(force_redraw);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::call_idle_callback
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::
call_idle_callback() {
  if (_idle_callback) {
    _idle_callback->idle();
  }
}

