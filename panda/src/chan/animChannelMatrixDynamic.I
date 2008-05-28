// Filename: animChannelMatrixDynamic.I
// Created by:  drose (20Oct03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::get_value_transform
//       Access: Published
//  Description: Returns the explicit TransformState value that was
//               set via set_value(), if any.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *AnimChannelMatrixDynamic::
get_value_transform() const {
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixDynamic::get_value_node
//       Access: Published
//  Description: Returns the node that was set via set_value_node(),
//               if any.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *AnimChannelMatrixDynamic::
get_value_node() const {
  return _value_node;
}
