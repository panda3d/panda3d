/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterJointBundle.I
 * @author drose
 * @date 1999-03-02
 */

// This comment tells ppremake that we know this is a circular #include
// reference, and please don't bother us about it.  The line must be exactly
// as shown.
/* okcircular */
#include "character.h"

/**
 * Normally, you'd use make_copy() or copy_subgraph() to make a copy of this.
 */
INLINE CharacterJointBundle::
CharacterJointBundle(const CharacterJointBundle &copy) :
  PartBundle(copy)
{
}

/**
 * Returns the nth Character associated with this PartBundle.
 */
INLINE Character *CharacterJointBundle::
get_node(int n) const {
  return DCAST(Character, PartBundle::get_node(n));
}
