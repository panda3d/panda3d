// Filename: frameRateMeter.I
// Created by:  drose (23Dec03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: FrameRateMeter::setup_layer
//       Access: Published
//  Description: Sets up the frame rate meter to create a layer to
//               render itself into the indicated window.
////////////////////////////////////////////////////////////////////
INLINE void FrameRateMeter::
setup_layer(GraphicsOutput *window) {
  setup_layer(window->get_channel(0));
}

////////////////////////////////////////////////////////////////////
//     Function: FrameRateMeter::get_layer
//       Access: Published
//  Description: Returns the GraphicsLayer that the meter has created
//               to render itself into the window or channel supplied
//               to setup_layer(), or NULL if setup_layer() has not
//               been called.
////////////////////////////////////////////////////////////////////
INLINE GraphicsLayer *FrameRateMeter::
get_layer() const {
  return _layer;
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
  do_update();
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
