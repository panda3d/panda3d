// Filename: physicalNode.I
// Created by:  charles (01Aug00)
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
INLINE void PhysicalNode::
clear() {
  _physicals.erase(_physicals.begin(), _physicals.end());
}

////////////////////////////////////////////////////////////////////
//  Function : get_physical
//    Access : public
////////////////////////////////////////////////////////////////////
INLINE Physical *PhysicalNode::
get_physical(int index) const {
  nassertr(index >= 0 && index < (int)_physicals.size(),
           (Physical *) NULL);
  return _physicals[index];
}

////////////////////////////////////////////////////////////////////
//  Function : get_num_physicals
//    Access : public
////////////////////////////////////////////////////////////////////
INLINE int PhysicalNode::
get_num_physicals() const {
  return _physicals.size();
}

////////////////////////////////////////////////////////////////////
//  Function : add_physical
//    Access : public
////////////////////////////////////////////////////////////////////
INLINE void PhysicalNode::
add_physical(Physical *physical) {
  _physicals.push_back(physical);
  physical->_physical_node = this;
}
