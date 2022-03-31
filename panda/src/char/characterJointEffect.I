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

/**
 * Returns true if this CharacterJointEffect contains the given Character.
 * This exists because it is faster to check than get_character() and can even
 * be called while the Character is destructing.
 */
INLINE bool CharacterJointEffect::
matches_character(Character *character) const {
  // This works because while the Character is destructing, the ref count will
  // be 0 but was_deleted() will still return false.  We cannot construct a
  // PointerTo to the character (via lock() or otherwise) when the reference
  // count is 0 since that will cause double deletion.
  return _character.get_orig() == character && !_character.was_deleted();
}
