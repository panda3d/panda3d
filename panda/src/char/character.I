// Filename: character.I
// Created by:  drose (23Feb99)
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
  nassertr(n >= 0 && n < _parts.size(), NULL);
  return _parts[n];
}
