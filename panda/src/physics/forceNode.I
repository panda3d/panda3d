// Filename: forceNode.I
// Created by:  charles (02Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//  Function : clear
//    Access : public
////////////////////////////////////////////////////////////////////
INLINE void ForceNode::
clear(void) {
  _forces.erase(_forces.begin(), _forces.end());
}

////////////////////////////////////////////////////////////////////
//  Function : get_force
//    Access : public
////////////////////////////////////////////////////////////////////
INLINE BaseForce *ForceNode::
get_force(int index) const {
  nassertr(index >= 0 && index < (int)_forces.size(),
           (BaseForce *) NULL);
  return _forces[index];
}

////////////////////////////////////////////////////////////////////
//  Function : get_num_forces
//    Access : public
////////////////////////////////////////////////////////////////////
INLINE int ForceNode::
get_num_forces() const {
  return _forces.size();
}

////////////////////////////////////////////////////////////////////
//  Function : add_force
//    Access : public
////////////////////////////////////////////////////////////////////
INLINE void ForceNode::
add_force(BaseForce *force) {
  _forces.push_back(force);
  force->_force_node = this;
}
