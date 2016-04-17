/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletBaseCharacterControllerNode.cxx
 * @author enn0x
 * @date 2010-11-21
 */

#include "bulletBaseCharacterControllerNode.h"

TypeHandle BulletBaseCharacterControllerNode::_type_handle;

/**
 *
 */
BulletBaseCharacterControllerNode::
BulletBaseCharacterControllerNode(const char *name) : PandaNode(name) {

  // Default collide mask
  set_into_collide_mask(CollideMask::all_on());
}

/**
 * Returns the subset of CollideMask bits that may be set for this particular
 * type of PandaNode.  For CharacterControllerNodes this returns all bits on.
 */
CollideMask BulletBaseCharacterControllerNode::
get_legal_collide_mask() const {

  return CollideMask::all_on();
}

/**
 * Returns true if it is generally safe to flatten out this particular kind of
 * Node by duplicating instances, false otherwise (for instance, a Camera
 * cannot be safely flattened, because the Camera pointer itself is
 * meaningful).
 */
bool BulletBaseCharacterControllerNode::
safe_to_flatten() const {

  return false;
}

/**
 * Returns true if it is safe to automatically adjust the transform on this
 * kind of node.  Usually, this is only a bad idea if the user expects to find
 * a particular transform on the node.
 *
 * ModelNodes with the preserve_transform flag set are presently the only
 * kinds of nodes that should not have their transform even adjusted.
 */
bool BulletBaseCharacterControllerNode::
safe_to_modify_transform() const {

  return false;
}

/**
 * Returns true if it is generally safe to combine this particular kind of
 * PandaNode with other kinds of PandaNodes of compatible type, adding
 * children or whatever.  For instance, an LODNode should not be combined with
 * any other PandaNode, because its set of children is meaningful.
 */
bool BulletBaseCharacterControllerNode::
safe_to_combine() const {

  return false;
}

/**
 * Returns true if it is generally safe to combine the children of this
 * PandaNode with each other.  For instance, an LODNode's children should not
 * be combined with each other, because the set of children is meaningful.
 */
bool BulletBaseCharacterControllerNode::
safe_to_combine_children() const {

  return false;
}

/**
 * Returns true if a flatten operation may safely continue past this node, or
 * false if nodes below this node may not be molested.
 */
bool BulletBaseCharacterControllerNode::
safe_to_flatten_below() const {

  return false;
}

/**
 * Returns true if it is generally safe to transform this particular kind of
 * Node by calling the xform() method, false otherwise.  For instance, it's
 * usually a bad idea to attempt to xform a Character.
 */
bool BulletBaseCharacterControllerNode::
safe_to_transform() const {

  return false;
}
