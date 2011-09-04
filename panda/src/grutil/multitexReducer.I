// Filename: multitexReducer.I
// Created by:  drose (30Nov04)
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
//     Function: MultitexReducer::scan
//       Access: Published
//  Description: Starts scanning the hierarchy beginning at the
//               indicated node.  Any GeomNodes discovered in the
//               hierarchy with multitexture will be added to internal
//               structures in the MultitexReducer so that a future
//               call to flatten() will operate on all of these at
//               once.
//
//               This version of this method does not accumulate state
//               from the parents of the indicated node; thus, only
//               multitexture effects that have been applied at node
//               and below will be considered.
////////////////////////////////////////////////////////////////////
INLINE void MultitexReducer::
scan(const NodePath &node) {
  scan(node.node(), RenderState::make_empty(), TransformState::make_identity());
}

////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::scan
//       Access: Published
//  Description: Starts scanning the hierarchy beginning at the
//               indicated node.  Any GeomNodes discovered in the
//               hierarchy with multitexture will be added to internal
//               structures in the MultitexReducer so that a future
//               call to flatten() will operate on all of these at
//               once.
//
//               The second parameter represents the NodePath from
//               which to accumulate the state that is considered for
//               the multitexture.  Pass an empty NodePath to
//               accumulate all the state from the root of the graph,
//               or you may specify some other node here in order to
//               not consider nodes above that as contributing to the
//               state to be flattened.  This is particularly useful
//               if you have some texture stage which is applied
//               globally to a scene (for instance, a caustics
//               effect), which you don't want to be considered for
//               flattening by the MultitexReducer.
////////////////////////////////////////////////////////////////////
INLINE void MultitexReducer::
scan(const NodePath &node, const NodePath &state_from) {
  scan(node.node(), node.get_parent().get_state(state_from), 
       node.get_parent().get_transform(state_from));
}

////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::StageInfo::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool MultitexReducer::StageInfo::
operator < (const MultitexReducer::StageInfo &other) const {
  if (_stage != other._stage) {
    return _stage < other._stage;
  }
  if (_tex != other._tex) {
    return _tex < other._tex;
  }
  if (_tex_mat != other._tex_mat) {
    return _tex_mat->compare_to(*other._tex_mat, true) < 0;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::GeomInfo::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MultitexReducer::GeomInfo::
GeomInfo(const RenderState *state, const RenderState *geom_net_state,
         GeomNode *geom_node, int index) :
  _state(state),
  _geom_net_state(geom_net_state),
  _geom_node(geom_node),
  _index(index)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MultitexReducer::GeomNodeInfo::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MultitexReducer::GeomNodeInfo::
GeomNodeInfo(const RenderState *state, GeomNode *geom_node) :
  _state(state),
  _geom_node(geom_node)
{
}
