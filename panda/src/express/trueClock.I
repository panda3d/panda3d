/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file trueClock.I
 * @author drose
 * @date 2000-07-04
 */

#if defined(WIN32_VC) || defined(WIN64_VC)

/**
 *
 */
INLINE double TrueClock::
get_short_time() {
  bool is_paranoid_clock = get_paranoid_clock();

  if (is_paranoid_clock) {
    _lock.lock();
  }

  double time = get_short_raw_time();

  if (is_paranoid_clock) {
    // Check for rollforwards, rollbacks, and compensate for Speed Gear type
    // programs by verifying against the time of day clock.
    time = correct_time(time);
    _lock.unlock();
  }

  return time;
}

#else  // WIN32_VC

/**
 *
 */
INLINE double TrueClock::
get_short_time() {
  return get_short_raw_time();
}

#endif  // WIN32_VC


/**
 * Returns the number of clock errors that have been detected.  Each time a
 * clock error is detected, in which the value returned by either of the above
 * methods is suspect, the value returned by this method will be incremented.
 * Applications can monitor this value and react, for instance, by
 * resynchronizing their clocks each time this value changes.
 */
INLINE int TrueClock::
get_error_count() const {
  return _error_count;
}

/**
 * Returns a pointer to the one TrueClock object in the world.
 */
INLINE TrueClock *TrueClock::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new TrueClock;
  }
  return _global_ptr;
}

/**
 * A protected destructor because no one should try to delete the global
 * TrueClock.
 */
INLINE TrueClock::
~TrueClock() {
}
