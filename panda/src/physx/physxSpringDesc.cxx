/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxSpringDesc.cxx
 * @author enn0x
 * @date 2009-09-28
 */

#include "physxSpringDesc.h"

/**

 */
void PhysxSpringDesc::
set_spring(float spring) {

  _desc.spring = spring;
}

/**

 */
void PhysxSpringDesc::
set_damper(float damper) {

  _desc.damper = damper;
}

/**

 */
void PhysxSpringDesc::
set_target_value(float targetValue) {

  _desc.targetValue = targetValue;
}

/**

 */
float PhysxSpringDesc::
get_spring() const {

  return _desc.spring;
}

/**

 */
float PhysxSpringDesc::
get_damper() const {

  return _desc.damper;
}

/**

 */
float PhysxSpringDesc::
get_target_value() const {

  return _desc.targetValue;
}
