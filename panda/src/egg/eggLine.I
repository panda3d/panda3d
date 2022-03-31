/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggLine.I
 * @author drose
 * @date 2003-10-14
 */

/**
 *
 */
INLINE EggLine::
EggLine(const std::string &name) :
  EggCompositePrimitive(name),
  _has_thick(false)
{
}

/**
 *
 */
INLINE EggLine::
EggLine(const EggLine &copy) :
  EggCompositePrimitive(copy),
  _thick(copy._thick),
  _has_thick(copy._has_thick)
{
}

/**
 *
 */
INLINE EggLine &EggLine::
operator = (const EggLine &copy) {
  EggCompositePrimitive::operator = (copy);
  _thick = copy._thick;
  _has_thick = copy._has_thick;
  return *this;
}

/**
 *
 */
INLINE bool EggLine::
has_thick() const {
  return _has_thick;
}

/**
 * Returns the thickness set on this particular line.  If there is no
 * thickness set, returns 1.0.
 */
INLINE double EggLine::
get_thick() const {
  if (has_thick()) {
    return _thick;
  } else {
    return 1.0;
  }
}

/**
 *
 */
INLINE void EggLine::
set_thick(double thick) {
  _thick = thick;
  _has_thick = true;
}

/**
 *
 */
INLINE void EggLine::
clear_thick() {
  _has_thick = false;
}
