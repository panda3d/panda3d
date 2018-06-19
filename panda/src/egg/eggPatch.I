/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggPatch.I
 * @author drose
 * @date 2012-04-27
 */

/**
 *
 */
INLINE EggPatch::
EggPatch(const std::string &name) : EggPrimitive(name) {
}

/**
 *
 */
INLINE EggPatch::
EggPatch(const EggPatch &copy) : EggPrimitive(copy) {
}

/**
 *
 */
INLINE EggPatch &EggPatch::
operator = (const EggPatch &copy) {
  EggPrimitive::operator = (copy);
  return *this;
}
