// Filename: characterJointEffect.I
// Created by:  drose (26Jul06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
