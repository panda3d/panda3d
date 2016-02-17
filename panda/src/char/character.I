/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file character.I
 * @author drose
 * @date 2002-03-06
 */

#include "characterJointBundle.h"

/**
 *
 */
INLINE CharacterJointBundle *Character::
get_bundle(int i) const {
  return DCAST(CharacterJointBundle, PartBundleNode::get_bundle(i));
}
