// Filename: multitexReducer.I
// Created by:  drose (30Nov04)
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
    return _tex_mat < other._tex_mat;
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
