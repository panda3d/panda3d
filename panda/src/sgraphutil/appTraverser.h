// Filename: appTraverser.h
// Created by:  drose (25Apr00)
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

#ifndef APPTRAVERSER_H
#define APPTRAVERSER_H

#include <pandabase.h>

#include <renderTraverser.h>
#include <traverserVisitor.h>
#include <nodeRelation.h>
#include <nullTransitionWrapper.h>
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
PUBLISHED:
  INLINE AppTraverser(TypeHandle graph_type);
  INLINE ~AppTraverser();

  void traverse(Node *root);

public:
  // These methods, from parent class TraverserVisitor, define the
  // behavior of the AppTraverser as it traverses the graph.
  // Normally you would never call these directly.
  bool reached_node(Node *node, NullTransitionWrapper &render_state,
                    NullLevelState &level_state);

private:
  TypeHandle _graph_type;
};

#include "appTraverser.I"

#endif

