// Filename: deferredArcProperty.cxx
// Created by:  drose (04Jul00)
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

#include "deferredArcProperty.h"

#include <collisionNode.h>

////////////////////////////////////////////////////////////////////
//     Function: DeferredArcProperty::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DeferredArcProperty::
DeferredArcProperty() {
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DeferredArcProperty::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DeferredArcProperty::
DeferredArcProperty(const DeferredArcProperty &copy) :
  _flags(copy._flags),
  _from_collide_mask(copy._from_collide_mask),
  _into_collide_mask(copy._into_collide_mask)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DeferredArcProperty::Copy Assignment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void DeferredArcProperty::
operator = (const DeferredArcProperty &copy) {
  _flags = copy._flags;
  _from_collide_mask = copy._from_collide_mask;
  _into_collide_mask = copy._into_collide_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: DeferredArcProperty::compose
//       Access: Public
//  Description: Composes this state with the next one encountered on
//               a lower arc during the apply traversal.
////////////////////////////////////////////////////////////////////
void DeferredArcProperty::
compose(const DeferredArcProperty &other) {
  _flags |= other._flags;

  if ((other._flags & F_has_from_collide_mask) != 0) {
    _from_collide_mask = other._from_collide_mask;
  }

  if ((other._flags & F_has_into_collide_mask) != 0) {
    _into_collide_mask = other._into_collide_mask;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DeferredArcProperty::apply_to_arc
//       Access: Public
//  Description: Applies whatever state is appropriate to the arc.
////////////////////////////////////////////////////////////////////
void DeferredArcProperty::
apply_to_arc(NodeRelation *) {
}

////////////////////////////////////////////////////////////////////
//     Function: DeferredArcProperty::apply_to_node
//       Access: Public
//  Description: Applies whatever state is appropriate to the node.
////////////////////////////////////////////////////////////////////
void DeferredArcProperty::
apply_to_node(Node *node) {
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

