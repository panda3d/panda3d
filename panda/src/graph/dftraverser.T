// Filename: dftraverser.T
// Created by:  drose (26Oct98)
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
//     Function: DFTraverser::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Visitor, class LevelState>
INLINE_GRAPH DFTraverser<Visitor, LevelState>::
DFTraverser(Visitor &visitor,
            const TransitionWrapper &initial_render_state,
            TypeHandle graph_type) :
  _visitor(visitor),
  _initial_render_state(initial_render_state),
  _graph_type(graph_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DFTraverser::start
//       Access: Public
//  Description: Starts the traversal from the indicated arc.
////////////////////////////////////////////////////////////////////
template<class Visitor, class LevelState>
INLINE_GRAPH void DFTraverser<Visitor, LevelState>::
start(NodeRelation *arc, const LevelState &initial_level_state) {
  traverse(arc, _initial_render_state, initial_level_state);
}

////////////////////////////////////////////////////////////////////
//     Function: DFTraverser::start
//       Access: Public
//  Description: Starts the traversal from the indicated node.
////////////////////////////////////////////////////////////////////
template<class Visitor, class LevelState>
INLINE_GRAPH void DFTraverser<Visitor, LevelState>::
start(Node *root, const LevelState &initial_level_state) {
  LevelState level_state(initial_level_state);
  traverse(root, _initial_render_state, level_state);
}

////////////////////////////////////////////////////////////////////
//     Function: DFTraverser::Traverse
//       Access: Public
//  Description: Walks to the next arc of the graph.
////////////////////////////////////////////////////////////////////
template<class Visitor, class LevelState>
void DFTraverser<Visitor, LevelState>::
traverse(NodeRelation *arc, TransitionWrapper state, LevelState level_state) {
  nassertv(arc->get_child() != (Node *)NULL);

  // Give the visitor a chance to veto this arc.
  TransitionWrapper trans =
    TransitionWrapper::init_from(_initial_render_state);
  trans.extract_from(arc);

  TransitionWrapper post_state(state);
  post_state.compose_in_place(trans);
  if (_visitor.forward_arc(arc, trans, state, post_state, level_state)) {
    traverse(arc->get_child(), post_state, level_state);
    _visitor.backward_arc(arc, trans, state, post_state, level_state);
  }
}



////////////////////////////////////////////////////////////////////
//     Function: DFTraverser::Traverse
//       Access: Public
//  Description: Walks to the next node of the graph.
////////////////////////////////////////////////////////////////////
template<class Visitor, class LevelState>
void DFTraverser<Visitor, LevelState>::
traverse(Node *node, TransitionWrapper &state, LevelState &level_state) {
  // Tell the visitor we reached the node, and give it a chance to
  // veto our further progress.
  if (_visitor.reached_node(node, state, level_state)) {

    // Look for further children of the given NodeRelation.
    const DownRelationPointers &drp =
      node->find_connection(_graph_type).get_down();

    // Now visit each of the children in turn.
    DownRelationPointers::const_iterator drpi;
    for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
      traverse(*drpi, state, level_state);
    }
  }
}

// Convenience functions.
template<class Visitor, class TransitionWrapper, class LevelState>
INLINE void
df_traverse(NodeRelation *arc, Visitor &visitor,
            const TransitionWrapper &initial_render_state,
            const LevelState &initial_level_state,
            TypeHandle graph_type) {
  DFTraverser<Visitor, LevelState>
    dft(visitor, initial_render_state, graph_type);
  dft.start(arc, initial_level_state);
}

template<class Visitor, class TransitionWrapper, class LevelState>
INLINE void
df_traverse(Node *root, Visitor &visitor,
            const TransitionWrapper &initial_render_state,
            const LevelState &initial_level_state,
            TypeHandle graph_type) {
  DFTraverser<Visitor, LevelState>
    dft(visitor, initial_render_state, graph_type);
  dft.start(root, initial_level_state);
}

