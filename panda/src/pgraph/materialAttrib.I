/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file materialAttrib.I
 * @author drose
 * @date 2002-03-04
 */

/**
 * Use MaterialAttrib::make() to construct a new MaterialAttrib object.
 */
INLINE MaterialAttrib::
MaterialAttrib() {
}

/**
 * Returns true if the MaterialAttrib is an 'off' MaterialAttrib, indicating
 * that it should disable the use of materials.
 */
INLINE bool MaterialAttrib::
is_off() const {
  return _material == nullptr;
}

/**
 * If the MaterialAttrib is not an 'off' MaterialAttrib, returns the material
 * that is associated.  Otherwise, return NULL.
 */
INLINE Material *MaterialAttrib::
get_material() const {
  return _material;
}
