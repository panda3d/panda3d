// Filename: animBundleNode.I
// Created by:  drose (22Feb99)
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


////////////////////////////////////////////////////////////////////
//     Function: AnimBundleNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE AnimBundleNode::
AnimBundleNode(AnimBundle *bundle) :
  NamedNode(bundle->get_name()),
  _bundle(bundle)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AnimBundleNode::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE AnimBundleNode::
AnimBundleNode(void)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AnimBundleNode::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE AnimBundleNode::
AnimBundleNode(const AnimBundleNode &copy) :
  NamedNode(copy),
  _bundle(copy._bundle)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AnimBundleNode::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void AnimBundleNode::
operator = (const AnimBundleNode &copy) {
  NamedNode::operator = (copy);
  _bundle = copy._bundle;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimBundleNode::get_bundle
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE AnimBundle *AnimBundleNode::
get_bundle() const {
  return _bundle;
}
