// Filename: appTraverser.h
// Created by:  drose (25Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef APPTRAVERSER_H
#define APPTRAVERSER_H

#include <pandabase.h>

#include <renderTraverser.h>
#include <traverserVisitor.h>
#include <nodeRelation.h>
#include <nullTransitionWrapper.h>
#include <nullAttributeWrapper.h>
#include <nullLevelState.h>

class Node;

////////////////////////////////////////////////////////////////////
//       Class : AppTraverser
// Description : This traverser is designed to make a per-frame pass
//               over the scene graph before rendering, to update any
//               internal nodes as appropriate.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AppTraverser :
  public TraverserVisitor<NullTransitionWrapper, NullLevelState> {
public:
  INLINE AppTraverser(TypeHandle graph_type);

  void traverse(Node *root);

public:  
  // These methods, from parent class TraverserVisitor, define the
  // behavior of the AppTraverser as it traverses the graph.
  // Normally you would never call these directly.
  bool reached_node(Node *node, NullAttributeWrapper &render_state,
		    NullLevelState &level_state);

private:
  TypeHandle _graph_type;
};

#include "appTraverser.I"

#endif

