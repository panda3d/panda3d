// Filename: characterJointEffect.I
// Created by:  drose (26Jul06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CharacterJointEffect::Constructor
//       Access: Private
//  Description: Use CharacterJointEffect::make() to construct a new
//               CharacterJointEffect object.
////////////////////////////////////////////////////////////////////
INLINE CharacterJointEffect::
CharacterJointEffect() {
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJointEffect::get_character
//       Access: Published
//  Description: Returns the Character that will get update() called
//               on it when this node's relative transform is queried,
//               or NULL if there is no such character.
////////////////////////////////////////////////////////////////////
INLINE Character *CharacterJointEffect::
get_character() const {
  if (_character.is_valid_pointer()) {
    return _character;
  } else {
    return NULL;
  }
}
