// Filename: graphicsWindow.I
// Created by:  frang (07Mar99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Properties::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GraphicsWindow::Properties::
~Properties() {
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Properties::set_origin
//       Access: Published
//  Description: Specifies the origin on the screen (in pixels,
//               relative to the top-left corner) at which the window
//               should appear.  This is the origin of the top-left
//               corner of the useful part of the window, not
//               including decorations.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::Properties::
set_origin(int xorg, int yorg) {
  _xorg = xorg;
  _yorg = yorg;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Properties::set_size
//       Access: Published
//  Description: Specifies the requested size of the window, in
//               pixels.  This is the size of the useful part of the
//               window, not including decorations.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::Properties::
set_size(int xsize, int ysize) {
  _xsize = xsize;
  _ysize = ysize;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Properties::set_title
//       Access: Published
//  Description: Specifies the title that should be assigned to the
//               window.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::Properties::
set_title(const string &title) {
  _title = title;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Properties::set_border
//       Access: Published
//  Description: Specifies whether the window should be created with a
//               border (true, the default) or not (false).
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::Properties::
set_border(bool border) {
  _border = border;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Properties::set_fullscreen
//       Access: Published
//  Description: Specifies whether the window should be opened in
//               fullscreen mode (true) or normal windowed mode
//               (false, the default).
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::Properties::
set_fullscreen(bool fullscreen) {
  _fullscreen = fullscreen;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Properties::set_mask
//       Access: Published
//  Description: Specifies the set of graphics properties that are
//               required for the context associated with the window.
//               This should be the union of the appropriate bits
//               defined in WindowModeType.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::Properties::
set_mask(uint mask) {
  _mask = mask;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::Properties::set_bit_depth
//       Access: Published
//  Description: Specifies the minimum number of bits that are
//               required for the depth buffer and color buffer,
//               respectively.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::Properties::
set_bit_depth(int want_depth_bits, int want_color_bits) {
  _want_depth_bits = want_depth_bits;
  _want_color_bits = want_color_bits;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_properties
//       Access: Published
//  Description: Returns the full Properties structure that describes
//               the window.
////////////////////////////////////////////////////////////////////
INLINE const GraphicsWindow::Properties &GraphicsWindow::
get_properties() const {
  return _props;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_width
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int GraphicsWindow::
get_width() const {
  return (_props._xsize);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_height
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int GraphicsWindow::
get_height() const {
  return (_props._ysize);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_xorg
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int GraphicsWindow::
get_xorg() const {
  return (_props._xorg);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_yorg
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int GraphicsWindow::
get_yorg() const {
  return (_props._yorg);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_gsg
//       Access: Published
//  Description: Returns the GSG that is associated with this window.
//               There is a one-to-one association between windows and
//               GSG's.
//
//               It is invalid to call this if the window has been
//               closed; that is, if is_closed() returns true.  In
//               this case the GSG has been closed as well.
////////////////////////////////////////////////////////////////////
INLINE GraphicsStateGuardian *GraphicsWindow::
get_gsg() const {
  nassertr(!is_closed(), NULL);
  return _gsg;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_pipe
//       Access: Published
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
//     Function: GraphicsWindow::close_window
//       Access: Published
//  Description: Closes the window and frees all resources associated
//               with it, including the GSG.  The window may not be
//               opened again.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::
close_window() {
  if (!is_closed()) {
    do_close_window();
    release_gsg();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::is_closed
//       Access: Published
//  Description: Returns true if close_window() has been called on
//               this window, false otherwise.  If the window has been
//               closed, most of its interface is no longer valid.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindow::
is_closed() const {
  return (_gsg == (GraphicsStateGuardian *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::set_frame_number
//       Access: Published
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
//       Access: Published
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

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_num_input_devices
//       Access: Published
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
//       Access: Published
//  Description: Returns the name of the nth input device.
////////////////////////////////////////////////////////////////////
INLINE string GraphicsWindow::
get_input_device_name(int device) const {
  nassertr(device >= 0 && device < (int)_input_devices.size(), "");
  return _input_devices[device].get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::has_pointer
//       Access: Published
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
//       Access: Published
//  Description: Returns true if the nth input device has a keyboard,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindow::
has_keyboard(int device) const {
  nassertr(device >= 0 && device < (int)_input_devices.size(), false);
  return _input_devices[device].has_keyboard();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_mouse_data
//       Access: Published
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
//       Access: Published
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
//       Access: Published
//  Description: Assuming a previous call to has_button_event()
//               returned true, this returns the pending button event.
////////////////////////////////////////////////////////////////////
INLINE ButtonEvent GraphicsWindow::
get_button_event(int device) {
  nassertr(has_button_event(device), ButtonEvent());
  return _input_devices[device].get_button_event();
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::set_sync
//       Access: Public
//  Description: Sets flag for whether the buffer swap is done 
//               implicitely at the end of each frame, or done
//               explicitely for the purpose of synchronization.
////////////////////////////////////////////////////////////////////
INLINE void GraphicsWindow::
set_sync(const bool b) {
  _is_synced = b;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsWindow::get_sync
//       Access: Public
//  Description: Returns whether buffer swap is done explicitely by
//               call to swap() method.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsWindow::
get_sync() const {
  return _is_synced;
}

