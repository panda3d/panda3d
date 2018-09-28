/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerData.I
 * @author drose
 * @date 2002-07-15
 */

/**
 *
 */
INLINE PointerData::
PointerData() {
  _in_window = false;
  _xpos = 0;
  _ypos = 0;
}

/**
 *
 */
INLINE PointerData::
PointerData(const PointerData &copy) :
  _in_window(copy._in_window),
  _xpos(copy._xpos),
  _ypos(copy._ypos)
{
}

/**
 *
 */
INLINE void PointerData::
operator = (const PointerData &copy) {
  _in_window = copy._in_window;
  _xpos = copy._xpos;
  _ypos = copy._ypos;
}

/**
 *
 */
INLINE double PointerData::
get_x() const {
  return _xpos;
}

/**
 *
 */
INLINE double PointerData::
get_y() const {
  return _ypos;
}

/**
 *
 */
INLINE bool PointerData::
get_in_window() const {
  return _in_window;
}


INLINE std::ostream &operator << (std::ostream &out, const PointerData &md) {
  md.output(out);
  return out;
}
