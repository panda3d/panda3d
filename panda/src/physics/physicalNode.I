/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physicalNode.I
 * @author charles
 * @date 2000-08-01
 */

/**

 */
INLINE void PhysicalNode::
clear() {
  LightMutexHolder holder(_lock);
  for (Physical *physical : _physicals) {
    nassertd(physical->_physical_node == this) continue;
    physical->_physical_node = nullptr;
  }
  _physicals.clear();
}

/**

 */
INLINE Physical *PhysicalNode::
get_physical(size_t index) const {
  LightMutexHolder holder(_lock);
  nassertr(index < _physicals.size(), nullptr);
  return _physicals[index];
}

/**

 */
INLINE size_t PhysicalNode::
get_num_physicals() const {
  LightMutexHolder holder(_lock);
  return _physicals.size();
}

/**
 * Adds a Physical to this PhysicalNode.  If it is already added to this node,
 * does nothing.  It is an error to add a Physical to multiple PhysicalNodes.
 */
INLINE void PhysicalNode::
add_physical(Physical *physical) {
  LightMutexHolder holder(_lock);
  if (physical->_physical_node != this) {
    nassertv(physical->_physical_node == nullptr);
    _physicals.push_back(physical);
    physical->_physical_node = this;
  }
}
