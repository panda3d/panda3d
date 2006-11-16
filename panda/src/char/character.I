// Filename: character.I
// Created by:  drose (06Mar02)
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

#include "characterJointBundle.h"

////////////////////////////////////////////////////////////////////
//     Function: Character::get_bundle
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CharacterJointBundle *Character::
get_bundle(int i) const {
  return DCAST(CharacterJointBundle, PartBundleNode::get_bundle(i));
}

////////////////////////////////////////////////////////////////////
//     Function: Character::get_num_parts
//       Access: Published
//  Description: Returns the total number of moving parts (e.g. joints
//               and sliders) associated with the Character.
////////////////////////////////////////////////////////////////////
INLINE int Character::
get_num_parts() const {
  return _parts.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Character::get_part
//       Access: Published
//  Description: Returns the nth moving part associated with the
//               Character.
////////////////////////////////////////////////////////////////////
INLINE PartGroup *Character::
get_part(int n) const {
  nassertr(n >= 0 && n < (int)_parts.size(), NULL);
  return _parts[n];
}
