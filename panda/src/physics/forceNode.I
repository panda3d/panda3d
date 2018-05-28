/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file forceNode.I
 * @author charles
 * @date 2000-08-02
 */

/**

 */
INLINE void ForceNode::
clear() {
  _forces.erase(_forces.begin(), _forces.end());
}

/**

 */
INLINE BaseForce *ForceNode::
get_force(size_t index) const {
  nassertr(index < _forces.size(), nullptr);
  return _forces[index];
}

/**

 */
INLINE size_t ForceNode::
get_num_forces() const {
  return _forces.size();
}

/**

 */
INLINE void ForceNode::
add_force(BaseForce *force) {
  _forces.push_back(force);
  force->_force_node = this;
  force->_force_node_path = NodePath(this);
}
