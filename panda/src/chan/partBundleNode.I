// Filename: partBundleNode.I
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
//     Function: PartBundleNode::Constructor
//       Access: Public
//  Description: The PartBundle and its node should be constructed
//               together.  Generally, the derived classes of
//               PartBundleNode will automatically create a PartBundle
//               of the appropriate type, and pass it up to this
//               constructor.
////////////////////////////////////////////////////////////////////
INLINE PartBundleNode::
PartBundleNode(const string &name, PartBundle *bundle) :
  PandaNode(name),
  _bundle(bundle)
{
  _bundle->_node = this;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundleNode::Default Constructor
//       Access: Protected
//  Description: For internal use only.
////////////////////////////////////////////////////////////////////
INLINE PartBundleNode::
PartBundleNode() : PandaNode("") {
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundleNode::Copy Constructor
//       Access: Protected
//  Description: Use make_copy() or copy_subgraph() to copy one of
//               these.  
//
//               If the supplied PartBundle is non-null, it is
//               assigned to the new node; otherwise, a copy is made
//               of the complete PartGroup hierarchy.
////////////////////////////////////////////////////////////////////
INLINE PartBundleNode::
PartBundleNode(const PartBundleNode &copy, PartBundle *bundle) :
  PandaNode(copy)
{
  if (bundle != (PartBundle *)NULL) {
    _bundle = bundle;
  } else {
    _bundle = DCAST(PartBundle, copy._bundle->copy_subgraph());
  }
  _bundle->_node = this;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundleNode::get_bundle
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PartBundle *PartBundleNode::
get_bundle() const {
  return _bundle;
}
