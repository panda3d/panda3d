// Filename: character.I
// Created by:  drose (23Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "characterJointBundle.h"

////////////////////////////////////////////////////////////////////
//     Function: Character::get_bundle
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CharacterJointBundle *Character::
get_bundle() const {
  return DCAST(CharacterJointBundle, PartBundleNode::get_bundle());
}


////////////////////////////////////////////////////////////////////
//     Function: Character::get_computed_vertices
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ComputedVertices *Character::
get_computed_vertices() const {
  return _computed_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::get_num_parts
//       Access: Public
//  Description: Returns the total number of moving parts (e.g. joints
//               and sliders) associated with the character.
////////////////////////////////////////////////////////////////////
INLINE int Character::
get_num_parts() const {
  return _parts.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Character::get_part
//       Access: Public
//  Description: Returns the nth moving part associated with the
//               character.
////////////////////////////////////////////////////////////////////
INLINE PartGroup *Character::
get_part(int n) const {
  nassertr(n >= 0 && n < (int)_parts.size(), NULL);
  return _parts[n];
}

////////////////////////////////////////////////////////////////////
//     Function: Character::write_parts
//       Access: Public
//  Description: Writes a list of the character's joints and sliders,
//               in their hierchical structure, to the indicated
//               output stream.
////////////////////////////////////////////////////////////////////
INLINE void Character::
write_parts(ostream &out) const {
  get_bundle()->write(out, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: Character::write_part_values
//       Access: Public
//  Description: Writes a list of the character's joints and sliders,
//               along with each current position, in their hierchical
//               structure, to the indicated output stream.
////////////////////////////////////////////////////////////////////
INLINE void Character::
write_part_values(ostream &out) const {
  get_bundle()->write_with_value(out, 0);
}
