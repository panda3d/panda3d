// Filename: characterJointBundle.I
// Created by:  drose (02Mar99)
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

// This comment tells ppremake that we know this is a circular
// #include reference, and please don't bother us about it.  The line
// must be exactly as shown.
/* okcircular */
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
