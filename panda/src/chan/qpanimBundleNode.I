// Filename: qpanimBundleNode.I
// Created by:  drose (06Mar02)
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
//     Function: qpAnimBundleNode::Constructor
//       Access: Public
//  Description: The AnimBundle and its node should be constructed
//               together.  Generally, the derived classes of
//               qpAnimBundleNode will automatically create a AnimBundle
//               of the appropriate type, and pass it up to this
//               constructor.
////////////////////////////////////////////////////////////////////
INLINE qpAnimBundleNode::
qpAnimBundleNode(const string &name, AnimBundle *bundle) :
  PandaNode(name),
  _bundle(bundle)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpAnimBundleNode::Default Constructor
//       Access: Protected
//  Description: For internal use only.
////////////////////////////////////////////////////////////////////
INLINE qpAnimBundleNode::
qpAnimBundleNode() : PandaNode("") {
}

////////////////////////////////////////////////////////////////////
//     Function: qpAnimBundleNode::Copy Constructor
//       Access: Protected
//  Description: Use make_copy() or copy_subgraph() to copy one of
//               these.  Copying a qpAnimBundleNode will always force a
//               deep copy of the PartGroup hierarchy.
////////////////////////////////////////////////////////////////////
INLINE qpAnimBundleNode::
qpAnimBundleNode(const qpAnimBundleNode &copy) :
  PandaNode(copy),
  _bundle(copy._bundle)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpAnimBundleNode::get_bundle
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE AnimBundle *qpAnimBundleNode::
get_bundle() const {
  return _bundle;
}
