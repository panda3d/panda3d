// Filename: dftraverser.cxx
// Created by:  drose (26Oct98)
// 
////////////////////////////////////////////////////////////////////

#include "dftraverser.h"

#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: DFTraverser::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Visitor, class LevelState>
INLINE DFTraverser<Visitor, LevelState>::
DFTraverser(NodeRelation *arc,
	    Visitor &visitor,
	    const AttributeWrapper &initial_render_state,
	    const LevelState &initial_level_state,
	    TypeHandle graph_type) :
  _visitor(visitor),
  _initial_render_state(initial_render_state),
  _graph_type(graph_type)
{
  traverse(arc, _initial_render_state, initial_level_state);
}

////////////////////////////////////////////////////////////////////
//     Function: DFTraverser::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Visitor, class LevelState>
INLINE DFTraverser<Visitor, LevelState>::
DFTraverser(Node *root,
	    Visitor &visitor,
	    const AttributeWrapper &initial_render_state,
	    const LevelState &initial_level_state,
	    TypeHandle graph_type) :
  _visitor(visitor),
  _initial_render_state(initial_render_state),
  _graph_type(graph_type)
{
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
traverse(NodeRelation *arc, AttributeWrapper state, LevelState level_state) {
  nassertv(arc->get_child() != (Node *)NULL);
  
  // Give the visitor a chance to veto this arc.
  TransitionWrapper trans = 
    TransitionWrapper::init_from(_initial_render_state);
  trans.extract_from(arc);

  AttributeWrapper post_state(state);
  post_state.apply_in_place(trans);
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
traverse(Node *node, AttributeWrapper &state, LevelState &level_state) {
  // Tell the visitor we reached the node, and give it a chance to
  // veto our further progress.
  if (_visitor.reached_node(node, state, level_state)) {

    // Look for further children of the given NodeRelation.
    DownRelations::const_iterator dri;
    dri = node->_children.find(_graph_type);
    if (dri != node->_children.end()) {
      // Here are some!
      const DownRelationPointers &drp = (*dri).second;

      // Now visit each of the children in turn.
      DownRelationPointers::const_iterator drpi;
      for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
	traverse(*drpi, state, level_state);
      }
    }
  }
}

