/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTriangleFan.I
 * @author drose
 * @date 2005-03-23
 */

/**
 *
 */
INLINE EggTriangleFan::
EggTriangleFan(const std::string &name) : EggCompositePrimitive(name) {
}

/**
 *
 */
INLINE EggTriangleFan::
EggTriangleFan(const EggTriangleFan &copy) : EggCompositePrimitive(copy) {
}

/**
 *
 */
INLINE EggTriangleFan &EggTriangleFan::
operator = (const EggTriangleFan &copy) {
  EggCompositePrimitive::operator = (copy);
  return *this;
}
