/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTriangleStrip.I
 * @author drose
 * @date 2005-03-13
 */

/**
 *
 */
INLINE EggTriangleStrip::
EggTriangleStrip(const std::string &name) : EggCompositePrimitive(name) {
}

/**
 *
 */
INLINE EggTriangleStrip::
EggTriangleStrip(const EggTriangleStrip &copy) : EggCompositePrimitive(copy) {
}

/**
 *
 */
INLINE EggTriangleStrip &EggTriangleStrip::
operator = (const EggTriangleStrip &copy) {
  EggCompositePrimitive::operator = (copy);
  return *this;
}
