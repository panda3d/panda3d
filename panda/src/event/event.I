/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file event.I
 * @author drose
 * @date 2001-05-15
 */

/**
 *
 */
INLINE void Event::
set_name(const std::string &name) {
  _name = name;
}

/**
 * Resets the Event's name to empty.
 */
INLINE void Event::
clear_name() {
  _name = "";
}

/**
 * Returns true if the Event has a nonempty name set, false if the name is
 * empty.
 */
INLINE bool Event::
has_name() const {
  return !_name.empty();
}

/**
 *
 */
INLINE const std::string &Event::
get_name() const {
  return _name;
}


INLINE std::ostream &operator << (std::ostream &out, const Event &n) {
  n.output(out);
  return out;
}
