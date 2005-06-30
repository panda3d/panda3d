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
get_bundle() const {
  return DCAST(CharacterJointBundle, PartBundleNode::get_bundle());
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

////////////////////////////////////////////////////////////////////
//     Function: Character::find_joint
//       Access: Published
//  Description: Returns a pointer to the joint with the given name,
//               if there is such a joint, or NULL if there is no such
//               joint.  This will not return a pointer to a slider.
////////////////////////////////////////////////////////////////////
INLINE CharacterJoint *Character::
find_joint(const string &name) const {
  PartGroup *part = get_bundle()->find_child(name);
  if (part != (PartGroup *)NULL &&
      part->is_of_type(CharacterJoint::get_class_type())) {
    return DCAST(CharacterJoint, part);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::find_slider
//       Access: Published
//  Description: Returns a pointer to the slider with the given name,
//               if there is such a slider, or NULL if there is no such
//               slider.  This will not return a pointer to a joint.
////////////////////////////////////////////////////////////////////
INLINE CharacterSlider *Character::
find_slider(const string &name) const {
  PartGroup *part = get_bundle()->find_child(name);
  if (part != (PartGroup *)NULL &&
      part->is_of_type(CharacterSlider::get_class_type())) {
    return DCAST(CharacterSlider, part);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::write_parts
//       Access: Published
//  Description: Writes a list of the Character's joints and sliders,
//               in their hierchical structure, to the indicated
//               output stream.
////////////////////////////////////////////////////////////////////
INLINE void Character::
write_parts(ostream &out) const {
  get_bundle()->write(out, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: Character::write_part_values
//       Access: Published
//  Description: Writes a list of the Character's joints and sliders,
//               along with each current position, in their hierchical
//               structure, to the indicated output stream.
////////////////////////////////////////////////////////////////////
INLINE void Character::
write_part_values(ostream &out) const {
  get_bundle()->write_with_value(out, 0);
}
