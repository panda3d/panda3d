// Filename: quickRenderTraverser.h
// Created by:  drose (24Jul01)
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

#ifndef QUICKRENDERTRAVERSER_H
#define QUICKRENDERTRAVERSER_H

#include "pandabase.h"

#include "quickRenderLevelState.h"

#include "renderTraverser.h"
#include "traverserVisitor.h"
#include "nullTransitionWrapper.h"
#include "allTransitionsWrapper.h"
#include "pStatCollector.h"

class GraphicsStateGuardian;
class AllTransitionsWrapper;

////////////////////////////////////////////////////////////////////
//       Class : QuickRenderTraverser
// Description : A small RenderTraverser that performs a left-to-right
//               depth-first traversal of the scene graph, rendering
//               nodes as it encounters them, like a
//               DirectRenderTraverser.
//
//               However, it does not support instancing, nor
//               view-frustum culling.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA QuickRenderTraverser :
  public RenderTraverser,
  public TraverserVisitor<NullTransitionWrapper, QuickRenderLevelState> {
public:
  QuickRenderTraverser(GraphicsStateGuardian *gsg, TypeHandle graph_type,
                       const ArcChain &arc_chain = ArcChain());
  virtual ~QuickRenderTraverser();

  virtual void traverse(Node *root,
                        const AllTransitionsWrapper &initial_state);

public:
  // These methods, from parent class TraverserVisitor, define the
  // behavior of the RenderTraverser as it traverses the graph.
  // Normally you would never call these directly.
  bool forward_arc(NodeRelation *arc, NullTransitionWrapper &trans,
                   NullTransitionWrapper &pre, NullTransitionWrapper &post,
                   QuickRenderLevelState &level_state);

  INLINE void
  backward_arc(NodeRelation *arc, NullTransitionWrapper &trans,
               NullTransitionWrapper &pre, NullTransitionWrapper &post,
               const QuickRenderLevelState &level_state);

private:
  Node *_root;
  AllTransitionsWrapper _initial_state;

  // Statistics
  static PStatCollector _draw_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderTraverser::init_type();
    register_type(_type_handle, "QuickRenderTraverser",
                  RenderTraverser::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "quickRenderTraverser.I"

#endif

