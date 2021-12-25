/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerThrower.I
 * @author rdb
 * @date 2021-12-22
 */

/**
 * Specifies the event that is thrown when a pointer enters the active state.
 */
INLINE void PointerThrower::
set_down_event(const std::string &down_event) {
  _down_event = down_event;
}

/**
 *
 */
INLINE const std::string &PointerThrower::
get_down_event() const {
  return _down_event;
}

/**
 * Specifies the event that is thrown when a pointer leaves the active state.
 */
INLINE void PointerThrower::
set_up_event(const std::string &up_event) {
  _up_event = up_event;
}

/**
 *
 */
INLINE const std::string &PointerThrower::
get_up_event() const {
  return _up_event;
}

/**
 * Specifies the event that is thrown when a pointer leaves the active state.
 */
INLINE void PointerThrower::
set_move_event(const std::string &move_event) {
  _move_event = move_event;
}

/**
 *
 */
INLINE const std::string &PointerThrower::
get_move_event() const {
  return _move_event;
}

/**
 * Sets the prefix which is prepended to all specific event names (that is,
 * event names generated from the button name itself, as opposed to the
 * generic event names like set_down_event) thrown by this object.
 */
INLINE void PointerThrower::
set_prefix(const std::string &prefix) {
  _prefix = prefix;
}

/**
 * Returns the prefix that has been set on this PointerThrower.  See
 * set_prefix().
 */
INLINE const std::string &PointerThrower::
get_prefix() const {
  return _prefix;
}
