// Filename: actorNode.I
// Created by:  charles (07Aug00)
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
//  Function : set_parent_arc
//    Access : public
////////////////////////////////////////////////////////////////////
INLINE void ActorNode::
set_parent_arc(RenderRelation *arc) {
  _parent_arc = arc;
}

////////////////////////////////////////////////////////////////////
//  Function : get_parent_arc
//    Access : public
////////////////////////////////////////////////////////////////////
INLINE RenderRelation *ActorNode::
get_parent_arc(void) const {
  return _parent_arc;
}
