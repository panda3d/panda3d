// Filename: cullTraverserData.h
// Created by:  drose (06Mar02)
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

#ifndef CULLTRAVERSERDATA_H
#define CULLTRAVERSERDATA_H

#include "pandabase.h"

#include "renderState.h"
#include "transformState.h"
#include "geometricBoundingVolume.h"
#include "pointerTo.h"

class PandaNode;

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
  INLINE CullTraverserData(const TransformState *render_transform,
                           const TransformState *net_transform,
                           const RenderState *state,
                           GeometricBoundingVolume *view_frustum,
                           GeometricBoundingVolume *guard_band,
                           const TransformState *camera_transform);
  INLINE CullTraverserData(const CullTraverserData &copy);
  INLINE void operator = (const CullTraverserData &copy);
  INLINE ~CullTraverserData();

  INLINE bool is_in_view(PandaNode *node);
  void apply_transform_and_state(PandaNode *node);

  CPT(TransformState) _render_transform;
  CPT(TransformState) _net_transform;
  CPT(RenderState) _state;
  PT(GeometricBoundingVolume) _view_frustum;
  PT(GeometricBoundingVolume) _guard_band;

  // This one is not modified during traversal, so it doesn't need to
  // be reference counted (we trust the original owner of this pointer
  // to reference count it and hold it during the lifetime of the
  // traversal).
  const TransformState *_camera_transform;

private:
  bool is_in_view_impl(PandaNode *node);
  static CPT(RenderState) get_fake_view_frustum_cull_effect();
};

#include "cullTraverserData.I"

#endif
