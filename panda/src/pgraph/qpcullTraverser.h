// Filename: qpcullTraverser.h
// Created by:  drose (23Feb02)
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

#ifndef qpCULLTRAVERSER_H
#define qpCULLTRAVERSER_H

#include "pandabase.h"

#include "renderState.h"
#include "transformState.h"
#include "geometricBoundingVolume.h"
#include "pointerTo.h"
#include "drawMask.h"

class PandaNode;
class CullHandler;
class CullTraverserData;
class CullableObject;

////////////////////////////////////////////////////////////////////
//       Class : CullTraverser
// Description : This object performs a depth-first traversal of the
//               scene graph, with optional view-frustum culling,
//               collecting CullState and searching for GeomNodes.
//               Each renderable Geom encountered is passed along with
//               its associated RenderState to the CullHandler object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpCullTraverser {
public:
  qpCullTraverser();

  INLINE void set_initial_state(const RenderState *initial_state);
  INLINE void set_camera_mask(const DrawMask &draw_mask);
  INLINE void set_camera_transform(const TransformState *camera_transform);
  INLINE void set_render_transform(const TransformState *render_transform);
  INLINE void set_view_frustum(GeometricBoundingVolume *view_frustum);
  INLINE void set_guard_band(GeometricBoundingVolume *guard_band);
  INLINE void set_cull_handler(CullHandler *cull_handler);

  void traverse(PandaNode *root);

private:
  void r_traverse(PandaNode *node, const CullTraverserData &data);
  void start_decal(PandaNode *node, const CullTraverserData &data);
  CullableObject *r_get_decals(PandaNode *node,
                               const CullTraverserData &data,
                               CullableObject *decals);

  CPT(RenderState) _initial_state;
  DrawMask _camera_mask;
  CPT(TransformState) _camera_transform;
  CPT(TransformState) _render_transform;
  PT(GeometricBoundingVolume) _view_frustum;
  PT(GeometricBoundingVolume) _guard_band;
  CullHandler *_cull_handler;
};

#include "qpcullTraverser.I"

#endif


  
