// Filename: modelNode.I
// Created by:  drose (16Mar02)
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
//     Function: ModelNode::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ModelNode::
ModelNode(const string &name) :
  PandaNode(name)
{
  _preserve_transform = PT_none;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::set_preserve_transform
//       Access: Public
//  Description: Sets the preserve_transform flag.  This restricts the
//               ability of a flatten operation to affect the
//               transform stored on this node.  If the flag is:
//
//               PT_none - the transform may be adjusted at will.
//
//               PT_local - the local (and net) transform should not
//               be changed in any way.  If necessary, an extra
//               transform will be left on the node above to guarantee
//               this.  This is the strongest restriction.
//
//               PT_net - preserve the net transform from the
//               root, but it's acceptable to modify the local
//               transform stored on this particular node if
//               necessary, so long as the net transform is not
//               changed.  This eliminates the need to drop an extra
//               transform on the node above.
////////////////////////////////////////////////////////////////////
INLINE void ModelNode::
set_preserve_transform(ModelNode::PreserveTransform preserve_transform) {
  _preserve_transform = preserve_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::get_preserve_transform
//       Access: Public
//  Description: Returns the current setting of the preserve_transform
//               flag.  See set_preserve_transform().
////////////////////////////////////////////////////////////////////
INLINE ModelNode::PreserveTransform ModelNode::
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
