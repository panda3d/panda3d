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

////////////////////////////////////////////////////////////////////
//     Function: clear
//       Access: Public
////////////////////////////////////////////////////////////////////
INLINE void ForceNode::
clear() {
  _forces.erase(_forces.begin(), _forces.end());
}

////////////////////////////////////////////////////////////////////
//     Function: get_force
//       Access: Public
////////////////////////////////////////////////////////////////////
INLINE BaseForce *ForceNode::
get_force(int index) const {
  nassertr(index >= 0 && index < (int)_forces.size(),
           (BaseForce *) NULL);
  return _forces[index];
}

////////////////////////////////////////////////////////////////////
//     Function: get_num_forces
//       Access: Public
////////////////////////////////////////////////////////////////////
INLINE int ForceNode::
get_num_forces() const {
  return _forces.size();
}

////////////////////////////////////////////////////////////////////
//     Function: add_force
//       Access: Public
////////////////////////////////////////////////////////////////////
INLINE void ForceNode::
add_force(BaseForce *force) {
  _forces.push_back(force);
  force->_force_node = this;
  force->_force_node_path = NodePath(this);
}
