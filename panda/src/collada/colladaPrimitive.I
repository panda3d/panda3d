/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file colladaPrimitive.I
 * @author rdb
 * @date 2011-05-23
 */

/**
 * Adds a new ColladaInput to this primitive.
 */
INLINE void ColladaPrimitive::
add_input(ColladaInput *input) {
  if (input->get_offset() >= _stride) {
    _stride = input->get_offset() + 1;
  }
  _inputs.push_back(input);
}

/**
 * Returns the Geom associated with this primitive.
 */
INLINE PT(Geom) ColladaPrimitive::
get_geom() const {
  return _geom;
}

/**
 * Returns the name of this primitive's material, or the empty string if none
 * was assigned.
 */
INLINE const std::string &ColladaPrimitive::
get_material() const {
  return _material;
}
