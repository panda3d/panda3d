// Filename: animBundleNode.I
// Created by:  drose (06Mar02)
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
//     Function: AnimBundleNode::Constructor
//       Access: Public
//  Description: The AnimBundle and its node should be constructed
//               together.  Generally, the derived classes of
//               AnimBundleNode will automatically create a AnimBundle
//               of the appropriate type, and pass it up to this
//               constructor.
////////////////////////////////////////////////////////////////////
INLINE AnimBundleNode::
AnimBundleNode(const string &name, AnimBundle *bundle) :
  PandaNode(name),
  _bundle(bundle)
{
}

////////////////////////////////////////////////////////////////////
//     Function: AnimBundleNode::Default Constructor
//       Access: Protected
//  Description: For internal use only.
////////////////////////////////////////////////////////////////////
INLINE AnimBundleNode::
AnimBundleNode() : PandaNode("") {
}

////////////////////////////////////////////////////////////////////
//     Function: AnimBundleNode::Copy Constructor
//       Access: Protected
//  Description: Use make_copy() or copy_subgraph() to copy one of
//               these.  Copying a AnimBundleNode will always force a
//               deep copy of the PartGroup hierarchy.
////////////////////////////////////////////////////////////////////
INLINE AnimBundleNode::
AnimBundleNode(const AnimBundleNode &copy) :
  PandaNode(copy),
  _bundle(copy._bundle)
{
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
