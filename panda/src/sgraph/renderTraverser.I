// Filename: renderTraverser.I
// Created by:  drose (12Apr00)
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
//     Function: RenderTraverser::Constructor
//       Access: Public
//  Description: Creates a new RenderTraverser ready to traverse a
//               scene graph beginning at the end of the arc chain
//               indicated in arc_chain (either an empty ArcChain, or
//               the result of get_arc_chain() from some other
//               traverser), with the indicated graph_type (usually
//               RenderRelation) and for the indicated gsg.
////////////////////////////////////////////////////////////////////
INLINE RenderTraverser::
RenderTraverser(GraphicsStateGuardian *gsg, TypeHandle graph_type,
                const ArcChain &arc_chain) :
  _gsg(gsg),
  _graph_type(graph_type),
  _arc_chain(arc_chain)
{
}

////////////////////////////////////////////////////////////////////
//     Function: RenderTraverser::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE RenderTraverser::
~RenderTraverser() {
}

////////////////////////////////////////////////////////////////////
//     Function: RenderTraverser::get_arc_chain
//       Access: Public
//  Description: Returns the complete chain of arcs from the root to
//               the current node in the traversal.
////////////////////////////////////////////////////////////////////
INLINE const ArcChain &RenderTraverser::
get_arc_chain() const {
  return _arc_chain;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderTraverser::begin
//       Access: Public
//  Description: Returns an iterator that can be used to traverse the
//               list of arcs from the root to the current node in the
//               traversal.
////////////////////////////////////////////////////////////////////
INLINE RenderTraverser::const_iterator RenderTraverser::
begin() const {
  return _arc_chain.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderTraverser::end
//       Access: Public
//  Description: Returns an iterator that can be used to traverse the
//               list of arcs from the root to the current node in the
//               traversal.
////////////////////////////////////////////////////////////////////
INLINE RenderTraverser::const_iterator RenderTraverser::
end() const {
  return _arc_chain.end();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderTraverser::empty
//       Access: Public
//  Description: Returns true if the list of arcs returned by begin()
//               .. end() is empty (i.e. begin() == end()), false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool RenderTraverser::
empty() const {
  return _arc_chain.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderTraverser::get_gsg
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GraphicsStateGuardian *RenderTraverser::
get_gsg() const {
  return _gsg;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderTraverser::get_graph_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TypeHandle RenderTraverser::
get_graph_type() const {
  return _graph_type;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderTraverser::mark_forward_arc
//       Access: Public
//  Description: To be called by the derived class's forward_arc()
//               method, this updates the list of arcs traversed to
//               include the current arc.
////////////////////////////////////////////////////////////////////
INLINE void RenderTraverser::
mark_forward_arc(NodeRelation *arc) {
  // This is now done by the FrustumCullTraverser.
}

////////////////////////////////////////////////////////////////////
//     Function: RenderTraverser::mark_backward_arc
//       Access: Public
//  Description: To be called by the derived class's forward_arc()
//               method, this updates the list of arcs traversed to
//               remove the current arc.
////////////////////////////////////////////////////////////////////
INLINE void RenderTraverser::
mark_backward_arc(NodeRelation *arc) {
  // This is now done by the FrustumCullTraverser.
}
