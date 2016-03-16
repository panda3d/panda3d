/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgWaitBar.I
 * @author drose
 * @date 2002-03-14
 */

/**
 * Sets the value at which the WaitBar indicates 100%.
 */
INLINE void PGWaitBar::
set_range(PN_stdfloat range) {
  LightReMutexHolder holder(_lock);
  _range = range;
  _bar_state = -1;
}

/**
 * Returns the value at which the WaitBar indicates 100%.
 */
INLINE PN_stdfloat PGWaitBar::
get_range() const {
  LightReMutexHolder holder(_lock);
  return _range;
}

/**
 * Sets the current value of the bar.  This should range between 0 and
 * get_range().
 */
INLINE void PGWaitBar::
set_value(PN_stdfloat value) {
  LightReMutexHolder holder(_lock);
  _value = value;
  _bar_state = -1;
}

/**
 * Returns the current value of the bar.
 */
INLINE PN_stdfloat PGWaitBar::
get_value() const {
  LightReMutexHolder holder(_lock);
  return _value;
}

/**
 * Returns the percentage complete.
 */
INLINE PN_stdfloat PGWaitBar::
get_percent() const {
  LightReMutexHolder holder(_lock);
  return (_value / _range) * 100.0f;
}

/**
 * Sets the kind of frame that is drawn on top of the WaitBar to represent the
 * amount completed.
 */
INLINE void PGWaitBar::
set_bar_style(const PGFrameStyle &style) {
  LightReMutexHolder holder(_lock);
  _bar_style = style;
  _bar_state = -1;
}

/**
 * Returns the kind of frame that is drawn on top of the WaitBar to represent
 * the amount completed.
 */
INLINE PGFrameStyle PGWaitBar::
get_bar_style() const {
  LightReMutexHolder holder(_lock);
  return _bar_style;
}
