// Filename: trueClock.I
// Created by:  drose (04Jul00)
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


#ifdef WIN32_VC

////////////////////////////////////////////////////////////////////
//     Function: TrueClock::get_short_time, Win32 implementation
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE double TrueClock::
get_short_time() {
  bool paranoid_clock = _paranoid_clock;

  if (paranoid_clock) {
    _lock.lock();
  }

  double time = get_short_raw_time();

  if (paranoid_clock) {
    // Check for rollforwards, rollbacks, and compensate for Speed
    // Gear type programs by verifying against the time of day clock.
    time = correct_time(time);
    _lock.release();
  }

  return time;
}

#else  // WIN32_VC

////////////////////////////////////////////////////////////////////
//     Function: TrueClock::get_short_time, Posix implementation
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE double TrueClock::
get_short_time() {
  return get_short_raw_time();
}

#endif  // WIN32_VC


////////////////////////////////////////////////////////////////////
//     Function: TrueClock::get_error_count
//       Access: Published
//  Description: Returns the number of clock errors that have
//               been detected.  Each time a clock error is detected,
//               in which the value returned by either of the above
//               methods is suspect, the value returned by this method
//               will be incremented.  Applications can monitor this
//               value and react, for instance, by resynchronizing
//               their clocks each time this value changes.
////////////////////////////////////////////////////////////////////
INLINE int TrueClock::
get_error_count() const {
  return _error_count;
}

////////////////////////////////////////////////////////////////////
//     Function: TrueClock::get_global_ptr
//       Access: Published, Static
//  Description: Returns a pointer to the one TrueClock object in
//               the world.
////////////////////////////////////////////////////////////////////
INLINE TrueClock *TrueClock::
get_global_ptr() {
  if (_global_ptr == (TrueClock *)NULL) {
    _global_ptr = new TrueClock;
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: TrueClock::Destructor
//       Access: Protected
//  Description: A protected destructor because no one should try to
//               delete the global TrueClock.
////////////////////////////////////////////////////////////////////
INLINE TrueClock::
~TrueClock() {
}
