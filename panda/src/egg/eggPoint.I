/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggPoint.I
 * @author drose
 * @date 1999-12-15
 */

/**
 *
 */
INLINE EggPoint::
EggPoint(const std::string &name) :
  EggPrimitive(name),
  _flags(0),
  _thick(1.0)
{
}

/**
 *
 */
INLINE EggPoint::
EggPoint(const EggPoint &copy) :
  EggPrimitive(copy),
  _flags(copy._flags),
  _thick(copy._thick)
{
}

/**
 *
 */
INLINE EggPoint &EggPoint::
operator = (const EggPoint &copy) {
  EggPrimitive::operator = (copy);
  _flags = copy._flags;
  _thick = copy._thick;
  return *this;
}

/**
 *
 */
INLINE bool EggPoint::
has_thick() const {
  return (_flags & F_has_thick) != 0;
}

/**
 * Returns the thickness set on this particular point.  If there is no
 * thickness set, returns 1.0.
 */
INLINE double EggPoint::
get_thick() const {
  return _thick;
}

/**
 *
 */
INLINE void EggPoint::
set_thick(double thick) {
  _thick = thick;
  _flags |= F_has_thick;
}

/**
 *
 */
INLINE void EggPoint::
clear_thick() {
  _thick = 1.0;
  _flags &= ~F_has_thick;
}

/**
 *
 */
INLINE bool EggPoint::
has_perspective() const {
  return (_flags & F_has_perspective) != 0;
}

/**
 * Returns the perspective flag set on this particular point.  If there is no
 * perspective flag set, returns false.
 */
INLINE bool EggPoint::
get_perspective() const {
  return (_flags & F_perspective) != 0;
}

/**
 *
 */
INLINE void EggPoint::
set_perspective(bool perspective) {
  if (perspective) {
    _flags |= F_perspective;
  } else {
    _flags &= ~F_perspective;
  }
  _flags |= F_has_perspective;
}

/**
 *
 */
INLINE void EggPoint::
clear_perspective() {
  _flags &= ~(F_has_perspective | F_perspective);
}
