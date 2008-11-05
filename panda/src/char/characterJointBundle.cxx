// Filename: characterJointBundle.cxx
// Created by:  drose (23Feb99)
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

#include "characterJointBundle.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle CharacterJointBundle::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CharacterJointBundle::Constructor
//       Access: Public
//  Description: Normally, there is no need to create a
//               CharacterJointBundle directly.  The Character node
//               will automatically create one for itself.
////////////////////////////////////////////////////////////////////
CharacterJointBundle::
CharacterJointBundle(const string &name) : PartBundle(name) {
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJointBundle::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CharacterJointBundle::
~CharacterJointBundle() {
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJointBundle::make_copy
//       Access: Protected, Virtual
//  Description: Allocates and returns a new copy of the node.
//               Children are not copied, but see copy_subgraph().
////////////////////////////////////////////////////////////////////
PartGroup *CharacterJointBundle::
make_copy() const {
  return new CharacterJointBundle(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJointBundle::add_node
//       Access: Protected, Virtual
//  Description: Adds the PartBundleNode pointer to the set of nodes
//               associated with the PartBundle.  Normally called only
//               by the PartBundleNode itself, for instance when the
//               bundle is flattened with another node.
////////////////////////////////////////////////////////////////////
void CharacterJointBundle::
add_node(PartBundleNode *node) {
  PartBundle::add_node(node);
  if (node->is_of_type(Character::get_class_type())) {
    Character *character = DCAST(Character, node);
    r_set_character(this, character);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJointBundle::remove_node
//       Access: Protected, Virtual
//  Description: Removes the PartBundleNode pointer from the set of
//               nodes associated with the PartBundle.  Normally
//               called only by the PartBundleNode itself, for
//               instance when the bundle is flattened with another
//               node.
////////////////////////////////////////////////////////////////////
void CharacterJointBundle::
remove_node(PartBundleNode *node) {
  PartBundle::remove_node(node);

  // If there is still a Character on the list, assign that one to all
  // of the joints.
  if (get_num_nodes() > 0) {
    r_set_character(this, get_node(get_num_nodes() - 1));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJointBundle::r_set_character
//       Access: Private
//  Description: Recursively sets the Character on each joint in the
//               hierarchy.
////////////////////////////////////////////////////////////////////
void CharacterJointBundle::
r_set_character(PartGroup *group, Character *character) {
  if (group == (PartGroup *)NULL) {
    // This might happen if we are in the middle of reading the
    // Character's hierarchy from the bam file.
    return;
  }

  if (group->is_of_type(CharacterJoint::get_class_type())) {
    DCAST(CharacterJoint, group)->set_character(character);
  }

  Children::const_iterator ci;
  for (ci = group->_children.begin(); ci != group->_children.end(); ++ci) {
    r_set_character((*ci), character);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJointBundle::make_CharacterJointBundle
//       Access: Protected
//  Description: Factory method to generate a CharacterJointBundle object
////////////////////////////////////////////////////////////////////
TypedWritable* CharacterJointBundle::
make_CharacterJointBundle(const FactoryParams &params)
{
  CharacterJointBundle *me = new CharacterJointBundle;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  manager->register_finalize(me);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: CharacterJointBundle::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a CharacterJointBundle object
////////////////////////////////////////////////////////////////////
void CharacterJointBundle::
register_with_read_factory()
{
  BamReader::get_factory()->register_factory(get_class_type(), make_CharacterJointBundle);
}

