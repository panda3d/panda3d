/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseData.I
 * @author drose
 * @date 2002-07-15
 */

/**
 *
 */
INLINE MouseData::
MouseData() {
  _in_window = false;
  _xpos = 0;
  _ypos = 0;
}

/**
 *
 */
INLINE MouseData::
MouseData(const MouseData &copy) :
  _in_window(copy._in_window),
  _xpos(copy._xpos),
  _ypos(copy._ypos)
{
}

/**
 *
 */
INLINE void MouseData::
operator = (const MouseData &copy) {
  _in_window = copy._in_window;
  _xpos = copy._xpos;
  _ypos = copy._ypos;
}

/**
 *
 */
INLINE double MouseData::
get_x() const {
  return _xpos;
}

/**
 *
 */
INLINE double MouseData::
get_y() const {
  return _ypos;
}

/**
 *
 */
INLINE bool MouseData::
get_in_window() const {
  return _in_window;
}


INLINE ostream &operator << (ostream &out, const MouseData &md) {
  md.output(out);
  return out;
}
