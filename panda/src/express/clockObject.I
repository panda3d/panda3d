// Filename: clockObject.I
// Created by:  drose (17Feb00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ClockObject::
~ClockObject() {
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::set_mode
//       Access: Public
//  Description: Changes the mode of the clock, e.g. from normal
//               (real-time) to non-real-time.  In non-real-time mode,
//               tick() will add the value of dt to the value returned
//               by get_time(), regardless of how much time has
//               actually passed.  In normal, real-time mode, tick()
//               will set the value returned by get_time() to the
//               current real time.
////////////////////////////////////////////////////////////////////
INLINE void ClockObject::
set_mode(ClockObject::Mode mode) {
  _mode = mode;
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::get_mode
//       Access: Public
//  Description: Returns the current mode of the clock.  See
//               set_mode().
////////////////////////////////////////////////////////////////////
INLINE ClockObject::Mode ClockObject::
get_mode() const {
  return _mode;
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::get_time
//       Access: Public
//  Description: Returns the time in seconds as of the last time
//               tick() was called (typically, this will be as of the
//               start of the current frame).
//
//               This is generally the kind of time you want to ask
//               for in most rendering and animation contexts, since
//               it's important that all of the animation for a given
//               frame remains in sync with each other.
////////////////////////////////////////////////////////////////////
INLINE double ClockObject::
get_time() const {
  return _reported_time;
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::get_intra_frame_time
//       Access: Public
//  Description: Returns the number of seconds that have elapsed since
//               the start of the frame.  This might be useful for
//               certain specialty applications, for instance to
//               measure how much time we've spent working on this one
//               particular frame.
//
//               This function returns an honest value whether the
//               ClockObject is in normal, real-time mode or in
//               non-real-time mode.
////////////////////////////////////////////////////////////////////
INLINE double ClockObject::
get_intra_frame_time() const {
  return get_real_time() - _frame_time;
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::get_real_time
//       Access: Public
//  Description: Returns the actual number of seconds elapsed since
//               the ClockObject was created, or since it was last
//               reset.  This is useful for doing real timing
//               measurements, e.g. for performance statistics.
////////////////////////////////////////////////////////////////////
INLINE double ClockObject::
get_real_time() const {
  return (_true_clock->get_real_time() - _start_time);
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::reset
//       Access: Public
//  Description: Simultaneously resets both the time and the frame
//               count to zero.
////////////////////////////////////////////////////////////////////
INLINE void ClockObject::
reset() {
  set_time(0.0);
  set_frame_count(0);
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::set_frame_count
//       Access: Public
//  Description: Resets the number of frames counted to the indicated
//               number.  Also see reset() and set_time().
////////////////////////////////////////////////////////////////////
INLINE void ClockObject::
set_frame_count(int frame_count) {
  _frame_count = frame_count;
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::get_frame_count
//       Access: Public
//  Description: Returns the number of times tick() has been called
//               since the ClockObject was created, or since it was
//               last reset.  This is generally the number of frames
//               that have been rendered.
////////////////////////////////////////////////////////////////////
INLINE int ClockObject::
get_frame_count() const {
  return _frame_count;
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::get_frame_rate
//       Access: Public
//  Description: Returns the average frame rate since the last reset.
//               This is simply the total number of frames divided by
//               the total elapsed time.
////////////////////////////////////////////////////////////////////
INLINE double ClockObject::
get_frame_rate() const {
  return (double)get_frame_count() / get_time();
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::get_dt
//       Access: Public
//  Description: Returns the elapsed time for the previous frame: the
//               number of seconds elapsed between the last two calls
//               to tick().
////////////////////////////////////////////////////////////////////
INLINE double ClockObject::
get_dt() const {
  return _dt;
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::set_dt
//       Access: Public
//  Description: In non-real-time mode, sets the number of seconds
//               that should appear to elapse between frames.
////////////////////////////////////////////////////////////////////
INLINE void ClockObject::
set_dt(double dt) {
  if (_mode != M_non_real_time) {
    express_cat.error()
      << "ClockObject cannot set dt in real-time mode." << endl;
    return;
  }

  _dt = dt;
}

////////////////////////////////////////////////////////////////////
//     Function: ClockObject::get_global_clock
//       Access: Public
//  Description: Returns a pointer to the global ClockObject.  This is
//               the ClockObject that most code should use for
//               handling scene graph rendering and animation.
////////////////////////////////////////////////////////////////////
INLINE ClockObject *ClockObject::
get_global_clock() {
  if (_global_clock == (ClockObject *)NULL) {
    _global_clock = new ClockObject;
  }
  return _global_clock;
}
