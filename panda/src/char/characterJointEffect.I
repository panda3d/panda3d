/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterJointEffect.I
 * @author drose
 * @date 2006-07-26
 */

/**
 * Use CharacterJointEffect::make() to construct a new CharacterJointEffect
 * object.
 */
INLINE CharacterJointEffect::
CharacterJointEffect() {
}

/**
 * Returns the Character that will get update() called on it when this node's
 * relative transform is queried, or NULL if there is no such character.
 */
INLINE PT(Character) CharacterJointEffect::
get_character() const {
  return _character.lock();
}
