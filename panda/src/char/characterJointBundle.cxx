/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterJointBundle.cxx
 * @author drose
 * @date 1999-02-23
 */

#include "characterJointBundle.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle CharacterJointBundle::_type_handle;

/**
 * Normally, there is no need to create a CharacterJointBundle directly.  The
 * Character node will automatically create one for itself.
 */
CharacterJointBundle::
CharacterJointBundle(const std::string &name) : PartBundle(name) {
}

/**
 *
 */
CharacterJointBundle::
~CharacterJointBundle() {
}

/**
 * Allocates and returns a new copy of the node.  Children are not copied, but
 * see copy_subgraph().
 */
PartGroup *CharacterJointBundle::
make_copy() const {
  return new CharacterJointBundle(*this);
}

/**
 * Adds the PartBundleNode pointer to the set of nodes associated with the
 * PartBundle.  Normally called only by the PartBundleNode itself, for
 * instance when the bundle is flattened with another node.
 */
void CharacterJointBundle::
add_node(PartBundleNode *node) {
  PartBundle::add_node(node);
  if (node->is_of_type(Character::get_class_type())) {
    Character *character = DCAST(Character, node);
    r_set_character(this, character);
  }
}

/**
 * Removes the PartBundleNode pointer from the set of nodes associated with
 * the PartBundle.  Normally called only by the PartBundleNode itself, for
 * instance when the bundle is flattened with another node.
 */
void CharacterJointBundle::
remove_node(PartBundleNode *node) {
  PartBundle::remove_node(node);

  // If there is still a Character on the list, assign that one to all of the
  // joints.
  if (get_num_nodes() > 0) {
    r_set_character(this, get_node(get_num_nodes() - 1));
  }
}

/**
 * Recursively sets the Character on each joint in the hierarchy.
 */
void CharacterJointBundle::
r_set_character(PartGroup *group, Character *character) {
  if (group == nullptr) {
    // This might happen if we are in the middle of reading the Character's
    // hierarchy from the bam file.
    return;
  }

  if (group->is_character_joint()) {
    ((CharacterJoint *)group)->set_character(character);
  }

  Children::const_iterator ci;
  for (ci = group->_children.begin(); ci != group->_children.end(); ++ci) {
    r_set_character((*ci), character);
  }
}

/**
 * Factory method to generate a CharacterJointBundle object
 */
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

/**
 * Factory method to generate a CharacterJointBundle object
 */
void CharacterJointBundle::
register_with_read_factory()
{
  BamReader::get_factory()->register_factory(get_class_type(), make_CharacterJointBundle);
}
