/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxConstraintDominance.cxx
 * @author enn0x
 * @date 2009-12-22
 */

#include "physxConstraintDominance.h"

/**
 * Retruns the first dominance factor.
 */
float PhysxConstraintDominance::
get_0() const {

  return _dominance.dominance0;
}

/**
 * Returns the second dominance factor.
 */
float PhysxConstraintDominance::
get_1() const {

  return _dominance.dominance1;
}

/**
 * Sets the first dominance factor.
 */
void PhysxConstraintDominance::
set_0(float d0) {

  _dominance.dominance0 = d0;
}

/**
 * Sets the second dominance factor.
 */
void PhysxConstraintDominance::
set_1(float d1) {

  _dominance.dominance1 = d1;
}
