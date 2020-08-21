/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physicalNode.cxx
 * @author charles
 * @date 2000-08-01
 */

#include "physicalNode.h"
#include "physicsManager.h"

// static stuff.
TypeHandle PhysicalNode::_type_handle;

/**
 * default constructor
 */
PhysicalNode::
PhysicalNode(const std::string &name) :
  PandaNode(name)
{
}

/**
 * copy constructor
 */
PhysicalNode::
PhysicalNode(const PhysicalNode &copy) :
  PandaNode(copy), _physicals(copy._physicals) {
}

/**
 * destructor
 */
PhysicalNode::
~PhysicalNode() {
  PhysicalsVector::iterator it;
  for (it = _physicals.begin(); it != _physicals.end(); ++it) {
    Physical *physical = *it;
    nassertd(physical->_physical_node == this) continue;
    physical->_physical_node = nullptr;
    if (physical->_physics_manager != nullptr) {
      physical->_physics_manager->remove_physical(physical);
    }
  }
}

/**
 * dynamic child copy
 */
PandaNode *PhysicalNode::
make_copy() const {
  return new PhysicalNode(*this);
}

/**
 * append operation
 */
void PhysicalNode::
add_physicals_from(const PhysicalNode &other) {
  pvector< PT(Physical) >::iterator last = _physicals.end() - 1;

  _physicals.insert(_physicals.end(),
                    other._physicals.begin(), other._physicals.end());

  for (; last != _physicals.end(); last++) {
    (*last)->_physical_node = this;
  }
}

/**
 * replace operation
 */
void PhysicalNode::
set_physical(size_t index, Physical *physical) {
  nassertv(index < _physicals.size());

  _physicals[index]->_physical_node = nullptr;
  _physicals[index] = physical;
  physical->_physical_node = this;
}

/**
 * insert operation
 */
void PhysicalNode::
insert_physical(size_t index, Physical *physical) {
  if (index > _physicals.size()) {
    index = _physicals.size();
  }

  _physicals.insert(_physicals.begin() + index, physical);
  physical->_physical_node = this;
}

/**
 * remove operation
 */
void PhysicalNode::
remove_physical(Physical *physical) {
  pvector< PT(Physical) >::iterator found;
  PT(Physical) ptp = physical;
  found = find(_physicals.begin(), _physicals.end(), ptp);
  if (found == _physicals.end()) {
    return;
  }
  _physicals.erase(found);

  nassertv(ptp->_physical_node == this);
  ptp->_physical_node = nullptr;
}

/**
 * remove operation
 */
void PhysicalNode::
remove_physical(size_t index) {
  nassertv(index <= _physicals.size());

  pvector< PT(Physical) >::iterator remove;
  remove = _physicals.begin() + index;
  (*remove)->_physical_node = nullptr;

  _physicals.erase(remove);
}

/**
 * Write a string representation of this instance to <out>.
 */
void PhysicalNode::
write(std::ostream &out, int indent) const {
  #ifndef NDEBUG //[
  out.width(indent); out<<""; out<<"PhysicalNode:\n";
  // PandaNode::write(out, indent+2);
  #endif //] NDEBUG
}
