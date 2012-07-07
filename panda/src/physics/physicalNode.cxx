// Filename: physicalNode.cxx
// Created by:  charles (01Aug00)
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

#include "physicalNode.h"

// static stuff.
TypeHandle PhysicalNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function : PhysicalNode
//       Access : public
//  Description : default constructor
////////////////////////////////////////////////////////////////////
PhysicalNode::
PhysicalNode(const string &name) :
  PandaNode(name) 
{
}

////////////////////////////////////////////////////////////////////
//     Function : PhysicalNode
//       Access : protected
//  Description : copy constructor
////////////////////////////////////////////////////////////////////
PhysicalNode::
PhysicalNode(const PhysicalNode &copy) :
  PandaNode(copy), _physicals(copy._physicals) {
}

////////////////////////////////////////////////////////////////////
//     Function : ~PhysicalNode
//       Access : protected, virtual
//  Description : destructor
////////////////////////////////////////////////////////////////////
PhysicalNode::
~PhysicalNode() {
}

////////////////////////////////////////////////////////////////////
//     Function : make_copy
//       Access : public, virtual
//  Description : dynamic child copy
////////////////////////////////////////////////////////////////////
PandaNode *PhysicalNode::
make_copy() const {
  return new PhysicalNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function : add_physicals_from
//       Access : public
//  Description : append operation
////////////////////////////////////////////////////////////////////
void PhysicalNode::
add_physicals_from(const PhysicalNode &other) {
  pvector< PT(Physical) >::iterator last = _physicals.end() - 1;

  _physicals.insert(_physicals.end(),
                    other._physicals.begin(), other._physicals.end());

  for (; last != _physicals.end(); last++) {
    (*last)->_physical_node = this;
  }
}

////////////////////////////////////////////////////////////////////
//     Function : remove_physical
//       Access : public
//  Description : remove operation
////////////////////////////////////////////////////////////////////
void PhysicalNode::
remove_physical(Physical *physical) {
  pvector< PT(Physical) >::iterator found;
  PT(Physical) ptp = physical;
  found = find(_physicals.begin(), _physicals.end(), ptp);
  if (found == _physicals.end())
    return;
  _physicals.erase(found);
}

////////////////////////////////////////////////////////////////////
//     Function : remove_physical
//       Access : public
//  Description : remove operation
////////////////////////////////////////////////////////////////////
void PhysicalNode::
remove_physical(int index) {
  nassertv(index >= 0 && index <= (int)_physicals.size());

  pvector< PT(Physical) >::iterator remove;
  remove = _physicals.begin() + index;
  (*remove)->_physical_node = (PhysicalNode *) NULL;

  _physicals.erase(remove);
}

////////////////////////////////////////////////////////////////////
//     Function : write
//       Access : Public
//  Description : Write a string representation of this instance to
//                <out>.
////////////////////////////////////////////////////////////////////
void PhysicalNode::
write(ostream &out, unsigned int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"PhysicalNode:\n";
  //PandaNode::write(out, indent+2);
  #endif //] NDEBUG
}
