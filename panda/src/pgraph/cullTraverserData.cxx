// Filename: cullTraverserData.cxx
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

#include "cullTraverserData.h"
#include "qpcullTraverser.h"
#include "config_pgraph.h"
#include "pandaNode.h"
#include "colorAttrib.h"
#include "textureAttrib.h"
#include "billboardEffect.h"

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::apply_transform_and_state
//       Access: Public
//  Description: Applies the transform and state from the indicated
//               node onto the current data.  This also evaluates
//               billboards, etc.
////////////////////////////////////////////////////////////////////
void CullTraverserData::
apply_transform_and_state(qpCullTraverser *trav, PandaNode *node) {
  const TransformState *node_transform = node->get_transform();
  if (!node_transform->is_identity()) {
    _render_transform = _render_transform->compose(node_transform);
    _net_transform = _net_transform->compose(node_transform);
    
    if ((_view_frustum != (GeometricBoundingVolume *)NULL) ||
        (_guard_band != (GeometricBoundingVolume *)NULL)) {
      // We need to move the viewing frustums into the node's
      // coordinate space by applying the node's inverse transform.
      if (node_transform->is_singular()) {
        // But we can't invert a singular transform!  Instead of
        // trying, we'll just give up on frustum culling from this
        // point down.
        _view_frustum = (GeometricBoundingVolume *)NULL;
        _guard_band = (GeometricBoundingVolume *)NULL;

      } else {
        CPT(TransformState) inv_transform = 
          node_transform->invert_compose(TransformState::make_identity());

        // Copy the bounding volumes for the frustums so we can
        // transform them.
        if (_view_frustum != (GeometricBoundingVolume *)NULL) {
          _view_frustum = DCAST(GeometricBoundingVolume, _view_frustum->make_copy());
          _view_frustum->xform(inv_transform->get_mat());
        }
        
        if (_guard_band != (GeometricBoundingVolume *)NULL) {
          _guard_band = DCAST(GeometricBoundingVolume, _guard_band->make_copy());
          _guard_band->xform(inv_transform->get_mat());
        }
      }
    }
  }

  _state = _state->compose(node->get_state());

  const RenderEffects *node_effects = node->get_effects();
  const BillboardEffect *billboard = node_effects->get_billboard();
  if (billboard != (const BillboardEffect *)NULL) {
    // Got to apply a billboard transform here.
    CPT(TransformState) billboard_transform = 
      billboard->do_billboard(_net_transform, trav->get_camera_transform());
    _render_transform = _render_transform->compose(billboard_transform);
    _net_transform = _net_transform->compose(billboard_transform);

    // We can't reliably cull within a billboard, because the geometry
    // might get rotated out of its bounding volume.  So once we get
    // within a billboard, we consider it all visible.
    _view_frustum = (GeometricBoundingVolume *)NULL;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::is_in_view_impl
//       Access: Private
//  Description: The private implementation of is_in_view().
////////////////////////////////////////////////////////////////////
bool CullTraverserData::
is_in_view_impl(PandaNode *node) {
  // By the time we get here, we know we have a viewing frustum.
  nassertr(_view_frustum != (GeometricBoundingVolume *)NULL, true);

  const BoundingVolume &node_volume = node->get_bound();
  nassertr(node_volume.is_of_type(GeometricBoundingVolume::get_class_type()), false);
  const GeometricBoundingVolume *node_gbv =
    DCAST(GeometricBoundingVolume, &node_volume);

  int result = _view_frustum->contains(node_gbv);
  if (result == BoundingVolume::IF_no_intersection) {
    // No intersection at all.  Cull.
    if (!qpfake_view_frustum_cull) {
      return false;
    }

    // If we have fake view-frustum culling enabled, instead of
    // actually culling an object we simply force it to be drawn in
    // red wireframe.
    _view_frustum = (GeometricBoundingVolume *)NULL;
    CPT(RenderState) fake_effect = get_fake_view_frustum_cull_effect();
    _state = _state->compose(fake_effect);
    
  } else if ((result & BoundingVolume::IF_all) != 0) {
    // The node and its descendants are completely enclosed within
    // the frustum.  No need to cull further.
    _view_frustum = (GeometricBoundingVolume *)NULL;

  } else {
    if (node->is_final()) {
      // The bounding volume is partially, but not completely,
      // within the viewing frustum.  Normally we'd keep testing
      // child bounding volumes as we continue down.  But this node
      // has the "final" flag, so the user is claiming that there is
      // some important reason we should consider everything visible
      // at this point.  So be it.
      _view_frustum = (GeometricBoundingVolume *)NULL;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::get_fake_view_frustum_cull_effect
//       Access: Private, Static
//  Description: Returns a RenderState for rendering stuff in red
//               wireframe, strictly for the fake_view_frustum_cull
//               effect.
////////////////////////////////////////////////////////////////////
CPT(RenderState) CullTraverserData::
get_fake_view_frustum_cull_effect() {
  // Once someone asks for this pointer, we hold its reference count
  // and never free it.
  static CPT(RenderState) effect = (const RenderState *)NULL;
  if (effect == (const RenderState *)NULL) {
    effect = RenderState::make
      (ColorAttrib::make_flat(Colorf(1.0f, 0.0f, 0.0f, 1.0f)),
       TextureAttrib::make_off(),
       1000);
  }
  return effect;
}

