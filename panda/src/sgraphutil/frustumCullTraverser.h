// Filename: frustumCullTraverser.h
// Created by:  drose (14Apr00)
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

#ifndef FRUSTUMCULLTRAVERSER_H
#define FRUSTUMCULLTRAVERSER_H

#include <pandabase.h>

#include <node.h>
#include <switchNode.h>
#include <nodeRelation.h>
#include <typedObject.h>
#include <geometricBoundingVolume.h>
#include <graphicsStateGuardian.h>
#include <displayRegion.h>
#include <arcChain.h>
#include <nodeTransitionWrapper.h>
#include <transformTransition.h>
#include <billboardTransition.h>
#include <wrt.h>
#include <pStatCollector.h>

#include "config_sgraphutil.h"


///////////////////////////////////////////////////////////////////
//       Class : FrustumCullTraverser
// Description : A special kind of depth-first traverser that can
//               prune the graph based on a lack of intersection with
//               a given bounding volume; i.e. it performs
//               view-frustum culling.
////////////////////////////////////////////////////////////////////
template<class Visitor, class LevelState>
class FrustumCullTraverser {
public:
  typedef TYPENAME Visitor::TransitionWrapper TransitionWrapper;
  typedef TYPENAME Visitor::AttributeWrapper AttributeWrapper;

  FrustumCullTraverser(ArcChain &arc_chain, Node *root,
                       const LMatrix4f &rel_from_camera, Visitor &visitor,
                       const AttributeWrapper &initial_render_state,
                       const LevelState &initial_level_state,
                       GraphicsStateGuardian *gsg,
                       TypeHandle graph_type);

protected:
  void traverse(NodeRelation *arc,
                AttributeWrapper render_state,
                LevelState level_state,
                PT(GeometricBoundingVolume) local_frustum,
                bool all_in);
  void traverse(Node *node,
                AttributeWrapper &render_state,
                LevelState &level_state,
                GeometricBoundingVolume *local_frustum,
                bool all_in);

  ArcChain &_arc_chain;
  Visitor &_visitor;
  AttributeWrapper _initial_render_state;
  GraphicsStateGuardian *_gsg;
  TypeHandle _graph_type;

  // If we are performing view-frustum culling, this is a pointer to
  // the bounding volume that encloses the view frustum, in its own
  // coordinate space.  If we are not performing view-frustum culling,
  // this will be a NULL pointer.
  PT(GeometricBoundingVolume) _view_frustum;
};

// Convenience function.
template<class Visitor, class AttributeWrapper, class LevelState>
INLINE void
fc_traverse(ArcChain &arc_chain, Node *root,
            const LMatrix4f &rel_from_camera, Visitor &visitor,
            const AttributeWrapper &initial_render_state,
            const LevelState &initial_level_state,
            GraphicsStateGuardian *gsg, TypeHandle graph_type) {
  FrustumCullTraverser<Visitor, LevelState>
    fct(arc_chain, root, rel_from_camera, visitor, initial_render_state,
        initial_level_state, gsg, graph_type);
}

#include "frustumCullTraverser.I"

#endif
