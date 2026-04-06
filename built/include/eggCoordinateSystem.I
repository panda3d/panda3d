/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggCoordinateSystem.I
 * @author drose
 * @date 1999-01-20
 */

/**
 *
 */
INLINE EggCoordinateSystem::
EggCoordinateSystem(CoordinateSystem value) {
  _value = value;
}


/**
 *
 */
INLINE EggCoordinateSystem::
EggCoordinateSystem(const EggCoordinateSystem &copy)
  : EggNode(copy), _value(copy._value) { }


/**
 *
 */
INLINE void EggCoordinateSystem::
set_value(CoordinateSystem value) {
  _value = value;
}


/**
 *
 */
INLINE CoordinateSystem EggCoordinateSystem::
get_value() const {
  return _value;
}
