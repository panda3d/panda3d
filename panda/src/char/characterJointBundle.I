// Filename: characterJointBundle.I
// Created by:  drose (02Mar99)
// 
////////////////////////////////////////////////////////////////////

#include "character.h"

////////////////////////////////////////////////////////////////////
//     Function: CharacterJointBundle::Copy Constructor
//       Access: Public
//  Description: Normally, you'd use make_copy() or copy_subgraph() to
//               make a copy of this.
////////////////////////////////////////////////////////////////////
INLINE CharacterJointBundle::
CharacterJointBundle(const CharacterJointBundle &copy) :
  PartBundle(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJointBundle::get_node
//       Access: Public
//  Description: Returns the Character node associated with this
//               PartBundle.
////////////////////////////////////////////////////////////////////
INLINE Character *CharacterJointBundle::
get_node() const {
  return DCAST(Character, PartBundle::get_node());
}
