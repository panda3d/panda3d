/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file standardMunger.I
 * @author drose
 * @date 2005-03-21
 */

/**
 * Returns a pointer to the GSG that created this munger.
 */
INLINE GraphicsStateGuardian *StandardMunger::
get_gsg() const {
  return (GraphicsStateGuardian *)GeomMunger::get_gsg();
}
