// Filename: attribTraverser.h
// Created by:  mike (16Feb99)
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

#ifndef ATTRIBTRAVERSER_H
#define ATTRIBTRAVERSER_H

#include <pandabase.h>

#include <traverserVisitor.h>
#include <nodeTransitionWrapper.h>
#include <nullLevelState.h>

class AllTransitionsWrapper;

////////////////////////////////////////////////////////////////////
//       Class : AttribTraverser
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AttribTraverser :
  public TraverserVisitor<NodeTransitionWrapper, NullLevelState> {
public:
  AttribTraverser();
  bool reached_node(Node *node, NodeTransitionWrapper &state, NullLevelState &);
  bool forward_arc(NodeRelation *arc, TransitionWrapper &trans,
                   NodeTransitionWrapper &, NodeTransitionWrapper &,
                   NullLevelState &);


  void set_attrib_type(TypeHandle type);
  void set_transition_type(TypeHandle type);
public:
  bool _has_attrib;
  bool _has_transition;
private:
  TypeHandle _attrib_type;
  TypeHandle _transition_type;
};

EXPCL_PANDA bool is_textured(Node* root);
EXPCL_PANDA bool is_textured(Node* root, const AllTransitionsWrapper &init_state);

EXPCL_PANDA bool is_shaded(Node* root);

#endif

