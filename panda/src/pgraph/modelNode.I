// Filename: modelNode.I
// Created by:  drose (16Mar02)
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
//     Function: ModelNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ModelNode::
ModelNode(const string &name) :
  PandaNode(name)
{
  _preserve_transform = false;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::set_preserve_transform
//       Access: Public
//  Description: Sets the preserve_transform flag.  When this flag is
//               true, flattening the scene graph will not flatten out
//               any transformation assigned above this node;
//               otherwise, any transforms applying to this node may
//               or may not be flattened.
////////////////////////////////////////////////////////////////////
INLINE void ModelNode::
set_preserve_transform(bool preserve_transform) {
  _preserve_transform = preserve_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::get_preserve_transform
//       Access: Public
//  Description: Returns the current setting of the preserve_transform
//               flag.  See set_preserve_transform().
////////////////////////////////////////////////////////////////////
INLINE bool ModelNode::
get_preserve_transform() const {
  return _preserve_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ModelNode::
ModelNode(const ModelNode &copy) :
  PandaNode(copy),
  _preserve_transform(copy._preserve_transform)
{
}
