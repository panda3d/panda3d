// Filename: switchNode.cxx
// Created by:  drose (15May01)
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

#include "switchNode.h"

TypeHandle SwitchNode::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: SwitchNode::safe_to_combine
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine
//               this particular kind of Node with other kinds of
//               Nodes, adding children or whatever.  For instance, an
//               LODNode should not be combined with any other node,
//               because its set of children is meaningful.
////////////////////////////////////////////////////////////////////
bool SwitchNode::
safe_to_combine() const {
  return false;
}
