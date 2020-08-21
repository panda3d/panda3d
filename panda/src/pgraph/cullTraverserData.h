/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullTraverserData.h
 * @author drose
 * @date 2002-03-06
 */

#ifndef CULLTRAVERSERDATA_H
#define CULLTRAVERSERDATA_H

#include "pandabase.h"
#include "cullPlanes.h"
#include "workingNodePath.h"
#include "renderState.h"
#include "transformState.h"
#include "geometricBoundingVolume.h"
#include "pointerTo.h"
#include "drawMask.h"
#include "pvector.h"

class PandaNode;
class CullTraverser;

/**
 * This collects together the pieces of data that are accumulated for each
 * node while walking the scene graph during the cull traversal.
 *
 * Having this as a separate object simplifies the parameter list to
 * CullTraverser::r_traverse(), as well as to other functions like
 * PandaNode::cull_callback().  It also makes it easier to add cull
 * parameters, and provides a place to abstract out some of the cull behavior
 * (like view-frustum culling).
 */
class EXPCL_PANDA_PGRAPH CullTraverserData {
public:
  INLINE CullTraverserData(const NodePath &start,
                           const TransformState *net_transform,
                           const RenderState *state,
                           GeometricBoundingVolume *view_frustum,
                           Thread *current_thread);
  INLINE CullTraverserData(const CullTraverserData &parent,
                           PandaNode *child);

PUBLISHED:
  INLINE PandaNode *node() const;

public:
  INLINE PandaNodePipelineReader *node_reader();
  INLINE const PandaNodePipelineReader *node_reader() const;

  INLINE NodePath get_node_path() const;

PUBLISHED:
  INLINE CPT(TransformState) get_modelview_transform(const CullTraverser *trav) const;
  INLINE CPT(TransformState) get_internal_transform(const CullTraverser *trav) const;
  INLINE const TransformState *get_net_transform(const CullTraverser *trav) const;

  INLINE bool is_in_view(const DrawMask &camera_mask);
  INLINE bool is_this_node_hidden(const DrawMask &camera_mask) const;

  void apply_transform_and_state(CullTraverser *trav);
  void apply_transform(const TransformState *node_transform);

  MAKE_PROPERTY(node_path, get_node_path);

private:
  // We store a chain leading all the way to the root, so that we can compose
  // a NodePath.  We may be able to eliminate this requirement in the future.
  const CullTraverserData *_next;
  NodePathComponent *_start;

public:
  PandaNodePipelineReader _node_reader;
  CPT(TransformState) _net_transform;
  CPT(RenderState) _state;
  PT(GeometricBoundingVolume) _view_frustum;
  CPT(CullPlanes) _cull_planes;
  DrawMask _draw_mask;
  int _portal_depth;

private:
  PT(NodePathComponent) r_get_node_path() const;

  bool is_in_view_impl();
  static const RenderState *get_fake_view_frustum_cull_state();
};

/* okcircular */
#include "cullTraverser.h"

#include "cullTraverserData.I"

#endif
