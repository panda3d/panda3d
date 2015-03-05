// Filename: paramNodePath.I
// Created by:  rdb (25Feb15)
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
//     Function: ParamNodePath::Constructor
//       Access: Published
//  Description: Creates a new ParamNodePath storing the given
//               node path object.
////////////////////////////////////////////////////////////////////
INLINE ParamNodePath::
ParamNodePath(const NodePath &node_path) :
  _node_path(node_path)
{
}

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: ParamNodePath::Move Constructor
//       Access: Published
//  Description: Creates a new ParamNodePath storing the given
//               node path object.
////////////////////////////////////////////////////////////////////
INLINE ParamNodePath::
ParamNodePath(NodePath &&node_path) NOEXCEPT :
  _node_path(move(node_path))
{
}
#endif  // USE_MOVE_SEMANTICS

////////////////////////////////////////////////////////////////////
//     Function: ParamNodePath::get_value_type
//       Access: Published, Virtual
//  Description: Returns NodePath::get_class_type().
////////////////////////////////////////////////////////////////////
INLINE TypeHandle ParamNodePath::
get_value_type() const {
  return NodePath::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: ParamNodePath::get_value
//       Access: Published
//  Description: Retrieves the NodePath stored in the parameter.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &ParamNodePath::
get_value() const {
  return _node_path;
}
