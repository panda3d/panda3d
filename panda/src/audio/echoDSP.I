/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file echoDSP.I
 * @author lachbr
 * @date 2020-10-04
 */

/**
 *
 */
INLINE EchoDSP::
EchoDSP(float delay, float feedback, float drylevel, float wetlevel) :
  DSP(DSP::DT_echo),
  _delay(delay),
  _feedback(feedback),
  _drylevel(drylevel),
  _wetlevel(wetlevel) {
}

/**
 * Sets the echo delay in milliseconds.
 */
INLINE void EchoDSP::
set_delay(float delay) {
  _delay = delay;
  on_values_changed();
}

/**
 * Returns the echo delay in milliseconds.
 */
INLINE float EchoDSP::
get_delay() const {
  return _delay;
}

/**
 * Sets the echo decay per delay. 100.0 = No decay, 0.0 = total decay.
 */
INLINE void EchoDSP::
set_feedback(float feedback) {
  _feedback = feedback;
  on_values_changed();
}

/**
 * Returns the echo decay per delay.
 */
INLINE float EchoDSP::
get_feedback() const {
  return _feedback;
}

/**
 * Sets the original sound volume in Decibels.
 */
INLINE void EchoDSP::
set_drylevel(float drylevel) {
  _drylevel = drylevel;
  on_values_changed();
}

/**
 * Returns the original sound volume in Decibels.
 */
INLINE float EchoDSP::
get_drylevel() const {
  return _drylevel;
}

/**
 * Sets the volume of the echo signal to pass to the output.
 */
INLINE void EchoDSP::
set_wetlevel(float wetlevel) {
  _wetlevel = wetlevel;
  on_values_changed();
}

/**
 * Returns the volume of the echo signal to pass to the output.
 */
INLINE float EchoDSP::
get_wetlevel() const {
  return _wetlevel;
}
