// Filename: cullTraverserData.h
// Created by:  drose (06Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

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

class PandaNode;
class CullTraverser;

////////////////////////////////////////////////////////////////////
//       Class : CullTraverserData
// Description : This collects together the pieces of data that are
//               accumulated for each node while walking the scene
//               graph during the cull traversal.
//
//               Having this as a separate object simplifies the
//               parameter list to CullTraverser::r_traverse(), as
//               well as to other functions like
//               PandaNode::cull_callback().  It also makes it easier
//               to add cull parameters, and provides a place to
//               abstract out some of the cull behavior (like
//               view-frustum culling).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullTraverserData {
public:
  INLINE CullTraverserData(const NodePath &start,
                           const TransformState *modelview_transform,
                           const RenderState *state,
                           GeometricBoundingVolume *view_frustum,
                           GeometricBoundingVolume *guard_band);
  INLINE CullTraverserData(const CullTraverserData &copy);
  INLINE void operator = (const CullTraverserData &copy); 
  INLINE CullTraverserData(const CullTraverserData &parent, 
                           PandaNode *child);
  INLINE ~CullTraverserData();

  INLINE PandaNode *node() const;

  INLINE const TransformState *get_modelview_transform() const;
  CPT(TransformState) get_net_transform(const CullTraverser *trav) const;

  INLINE bool is_in_view(const DrawMask &camera_mask);
  INLINE int test_within_clip_planes(const CullTraverser *trav) const;

  void apply_transform_and_state(CullTraverser *trav);
  void apply_transform_and_state(CullTraverser *trav, 
                                 CPT(TransformState) node_transform, 
                                 CPT(RenderState) node_state,
                                 CPT(RenderEffects) node_effects,
                                 const RenderAttrib *off_clip_planes);

  WorkingNodePath _node_path;
  CPT(TransformState) _modelview_transform;
  CPT(RenderState) _state;
  PT(GeometricBoundingVolume) _view_frustum;
  PT(GeometricBoundingVolume) _guard_band;
  CPT(CullPlanes) _cull_planes;

private:
  bool is_in_view_impl();
  int test_within_clip_planes_impl(const CullTraverser *trav,
                                   const ClipPlaneAttrib *cpa) const;

  static CPT(RenderState) get_fake_view_frustum_cull_state();
};

#include "cullTraverserData.I"

#endif
