// Filename: directRenderTraverser.h
// Created by:  drose (18Feb99)
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

#ifndef DIRECTRENDERTRAVERSER_H
#define DIRECTRENDERTRAVERSER_H

#include <pandabase.h>

#include "directRenderLevelState.h"

#include <renderTraverser.h>
#include <traverserVisitor.h>
#include <nodeRelation.h>
#include <allTransitionsWrapper.h>
#include <geometricBoundingVolume.h>
#include <lmatrix.h>
#include <pointerTo.h>
#include <pStatCollector.h>

class Node;
class GraphicsStateGuardian;
class GeometricBoundingVolume;

////////////////////////////////////////////////////////////////////
//       Class : DirectRenderTraverser
// Description : A kind of RenderTraverser that renders each GeomNode
//               it encounters immediately as it is encountered.  No
//               attempt is made to perform state-sorting or binning;
//               however, view-frustum culling is performed.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DirectRenderTraverser :
  public RenderTraverser,
  public TraverserVisitor<AllTransitionsWrapper, DirectRenderLevelState> {
public:
  DirectRenderTraverser(GraphicsStateGuardian *gsg, TypeHandle graph_type,
                        const ArcChain &arc_chain = ArcChain());
  virtual ~DirectRenderTraverser();

  virtual void traverse(Node *root,
                        const AllTransitionsWrapper &initial_state);

  INLINE void set_view_frustum_cull(bool flag);

public:
  // These methods, from parent class TraverserVisitor, define the
  // behavior of the DirectRenderTraverser as it traverses the graph.
  // Normally you would never call these directly.
  bool reached_node(Node *node, AllTransitionsWrapper &render_state,
                    DirectRenderLevelState &level_state);

  bool forward_arc(NodeRelation *arc, AllTransitionsWrapper &trans,
                   AllTransitionsWrapper &pre, AllTransitionsWrapper &post,
                   DirectRenderLevelState &level_state);

  void backward_arc(NodeRelation *arc, AllTransitionsWrapper &trans,
                    AllTransitionsWrapper &pre, AllTransitionsWrapper &post,
                    const DirectRenderLevelState &level_state);

private:
  bool _view_frustum_cull;

  // Statistics
  static PStatCollector _draw_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderTraverser::init_type();
    register_type(_type_handle, "DirectRenderTraverser",
                  RenderTraverser::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "directRenderTraverser.I"

#endif

