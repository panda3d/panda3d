// Filename: dftraverser.h
// Created by:  drose (26Oct98)
//
////////////////////////////////////////////////////////////////////

#ifndef DFTRAVERSER_H
#define DFTRAVERSER_H

#include <pandabase.h>

#include <node.h>
#include <nodeRelation.h>
#include <typeHandle.h>

///////////////////////////////////////////////////////////////////
// 	 Class : DFTraverser
// Description : Implements a depth-first traversal of the graph
//               beginning at the indicated node or arc.  DFTraverser
//               will also call visitor.forward_arc() and
//               visitor.backward_arc() to allow the visitor to manage
//               state.  See traverserVisitor.h.
////////////////////////////////////////////////////////////////////
template<class Visitor, class LevelState>
class DFTraverser {
public:
  typedef TYPENAME Visitor::TransitionWrapper TransitionWrapper;
  typedef TYPENAME Visitor::AttributeWrapper AttributeWrapper;

  INLINE_GRAPH DFTraverser(Visitor &visitor,
		     const AttributeWrapper &initial_render_state,
		     TypeHandle graph_type);

  INLINE_GRAPH void start(NodeRelation *arc, const LevelState &initial_level_state);
  INLINE_GRAPH void start(Node *root, const LevelState &initial_level_state);

protected:
  void traverse(NodeRelation *arc, 
		AttributeWrapper render_state,
		LevelState level_state);
  void traverse(Node *node,
		AttributeWrapper &render_state,
		LevelState &level_state);

  Visitor &_visitor;
  AttributeWrapper _initial_render_state;
  TypeHandle _graph_type;
};

#include "dftraverser.T"

#endif
