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
  PhysicalsVector::iterator it;
  for (it = _physicals.begin(); it != _physicals.end(); ++it) {
    nassertd((*it)->_physical_node == this) continue;
    (*it)->_physical_node = nullptr;
  }
  _physicals.erase(_physicals.begin(), _physicals.end());
}

/**

 */
INLINE Physical *PhysicalNode::
get_physical(size_t index) const {
  nassertr(index < _physicals.size(), nullptr);
  return _physicals[index];
}

/**

 */
INLINE size_t PhysicalNode::
get_num_physicals() const {
  return _physicals.size();
}

/**

 */
INLINE void PhysicalNode::
add_physical(Physical *physical) {
  _physicals.push_back(physical);
  physical->_physical_node = this;
}
