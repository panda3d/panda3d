// Filename: traverserVisitor.h
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

#ifndef TRAVERSERVISITOR_H
#define TRAVERSERVISITOR_H

#include <pandabase.h>

class Node;
class NodeRelation;

template<class TW, class LevelState>
class EXPCL_PANDA TraverserVisitor {
public:
  typedef TW TransitionWrapper;
  typedef TYPENAME TransitionWrapper::AttributeWrapper AttributeWrapper;

  INLINE_GRAPH bool reached_node(Node *node,
                           AttributeWrapper &render_state,
                           LevelState &level_state);

  // Some traversers (notably DFTraverser) will also call the
  // following two functions to mark the crossing of arcs.  This will
  // allow the Visitor to maintain its own internal state as needed.

  INLINE_GRAPH bool forward_arc(NodeRelation *arc, TransitionWrapper &trans,
                          AttributeWrapper &pre, AttributeWrapper &post,
                          LevelState &level_state);
  INLINE_GRAPH void backward_arc(NodeRelation *arc, TransitionWrapper &trans,
                           AttributeWrapper &pre, AttributeWrapper &post,
                           const LevelState &level_state);
};

#include "traverserVisitor.T"

#endif



