// Filename: frameRateMeter.I
// Created by:  drose (23Dec03)
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
//     Function: FrameRateMeter::get_window
//       Access: Published
//  Description: Returns the GraphicsOutput that was passed to
//               setup_window(), or NULL if setup_window() has not
//               been called.
////////////////////////////////////////////////////////////////////
INLINE GraphicsOutput *FrameRateMeter::
get_window() const {
  return _window;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameRateMeter::get_display_region
//       Access: Published
//  Description: Returns the DisplayRegion that the meter has created
//               to render itself into the window to setup_window(),
//               or NULL if setup_window() has not been called.
////////////////////////////////////////////////////////////////////
INLINE DisplayRegion *FrameRateMeter::
get_display_region() const {
  return _display_region;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameRateMeter::set_update_interval
//       Access: Published
//  Description: Specifies the number of seconds that should elapse
//               between updates to the frame rate indication.  This
//               should be reasonably slow (e.g. 0.2 to 1.0) so that
//               the calculation of the frame rate text does not
//               itself dominate the frame rate.
////////////////////////////////////////////////////////////////////
INLINE void FrameRateMeter::
set_update_interval(double update_interval) {
  _update_interval = update_interval;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameRateMeter::get_update_interval
//       Access: Published
//  Description: Returns the number of seconds that will elapse
//               between updates to the frame rate indication.
////////////////////////////////////////////////////////////////////
INLINE double FrameRateMeter::
get_update_interval() const {
  return _update_interval;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameRateMeter::set_text_pattern
//       Access: Published
//  Description: Sets the sprintf() pattern that is used to format the
//               text.  The string "%f" or some variant will be
//               replaced with the current frame rate in frames per
//               second.
////////////////////////////////////////////////////////////////////
INLINE void FrameRateMeter::
set_text_pattern(const string &text_pattern) {
  _text_pattern = text_pattern;
  Thread *current_thread = Thread::get_current_thread();
  do_update(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: FrameRateMeter::get_text_pattern
//       Access: Published
//  Description: Returns the sprintf() pattern that is used to format the
//               text.
////////////////////////////////////////////////////////////////////
INLINE const string &FrameRateMeter::
get_text_pattern() const {
  return _text_pattern;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameRateMeter::set_clock_object
//       Access: Published
//  Description: Sets the clock that is used to determine the frame
//               rate.  The default is the application's global clock
//               (ClockObject::get_global_clock()).
////////////////////////////////////////////////////////////////////
INLINE void FrameRateMeter::
set_clock_object(ClockObject *clock_object) {
  _clock_object = clock_object;
  _last_update = 0.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameRateMeter::get_clock_object
//       Access: Published
//  Description: Returns the clock that is used to determine the frame
//               rate.
////////////////////////////////////////////////////////////////////
INLINE ClockObject *FrameRateMeter::
get_clock_object() const {
  return _clock_object;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameRateMeter::update
//       Access: Published
//  Description: You can call this to explicitly force the
//               FrameRateMeter to update itself with the latest frame
//               rate information.  Normally, it is not necessary to
//               call this explicitly.
////////////////////////////////////////////////////////////////////
INLINE void FrameRateMeter::
update() {
  Thread *current_thread = Thread::get_current_thread();
  do_update(current_thread);
}
