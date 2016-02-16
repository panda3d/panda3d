/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file deferredNodeProperty.cxx
 * @author drose
 * @date 2002-03-20
 */

#include "deferredNodeProperty.h"

#include "collisionNode.h"
#include "pandaNode.h"
#include "dcast.h"

/**

 */
DeferredNodeProperty::
DeferredNodeProperty() {
  _flags = 0;
}

/**

 */
DeferredNodeProperty::
DeferredNodeProperty(const DeferredNodeProperty &copy) :
  _flags(copy._flags),
  _from_collide_mask(copy._from_collide_mask),
  _into_collide_mask(copy._into_collide_mask)
{
}

/**

 */
void DeferredNodeProperty::
operator = (const DeferredNodeProperty &copy) {
  _flags = copy._flags;
  _from_collide_mask = copy._from_collide_mask;
  _into_collide_mask = copy._into_collide_mask;
}

/**
 * Composes this state with the next one encountered on a lower node during the
 * apply traversal.
 */
void DeferredNodeProperty::
compose(const DeferredNodeProperty &other) {
  _flags |= other._flags;

  if ((other._flags & F_has_from_collide_mask) != 0) {
    _from_collide_mask = other._from_collide_mask;
  }

  if ((other._flags & F_has_into_collide_mask) != 0) {
    _into_collide_mask = other._into_collide_mask;
  }
}

/**
 * Applies whatever state is appropriate to the node.
 */
void DeferredNodeProperty::
apply_to_node(PandaNode *node) {
  if (node->is_of_type(CollisionNode::get_class_type())) {
    CollisionNode *cnode = DCAST(CollisionNode, node);
    if ((_flags & F_has_from_collide_mask) != 0) {
      cnode->set_from_collide_mask(_from_collide_mask);
    }
    if ((_flags & F_has_into_collide_mask) != 0) {
      cnode->set_into_collide_mask(_into_collide_mask);
    }
  }
}
