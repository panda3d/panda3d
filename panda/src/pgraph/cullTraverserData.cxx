// Filename: cullTraverserData.cxx
// Created by:  drose (06Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "cullTraverserData.h"
#include "cullTraverser.h"
#include "config_pgraph.h"
#include "pandaNode.h"
#include "colorAttrib.h"
#include "textureAttrib.h"
#include "renderModeAttrib.h"
#include "clipPlaneAttrib.h"
#include "boundingPlane.h"
#include "billboardEffect.h"
#include "compassEffect.h"
#include "occluderEffect.h"
#include "polylightEffect.h"
#include "renderState.h"


////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::apply_transform_and_state
//       Access: Published
//  Description: Applies the transform and state from the current
//               node onto the current data.  This also evaluates
//               billboards, etc.
////////////////////////////////////////////////////////////////////
void CullTraverserData::
apply_transform_and_state(CullTraverser *trav) {
  CPT(RenderState) node_state = _node_reader.get_state();

  if (trav->has_tag_state_key() &&
      _node_reader.has_tag(trav->get_tag_state_key())) {
    // Here's a node that has been tagged with the special key for our
    // current camera.  This indicates some special state transition
    // for this node, which is unique to this camera.
    const Camera *camera = trav->get_scene()->get_camera_node();
    string tag_state = _node_reader.get_tag(trav->get_tag_state_key());
    node_state = node_state->compose(camera->get_tag_state(tag_state));
  }
  _node_reader.compose_draw_mask(_draw_mask);

  apply_transform_and_state(trav, _node_reader.get_transform(),
                            node_state, _node_reader.get_effects(),
                            _node_reader.get_off_clip_planes());
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::apply_transform_and_state
//       Access: Published
//  Description: Applies the indicated transform and state changes
//               (e.g. as extracted from a node) onto the current
//               data.  This also evaluates billboards, etc.
////////////////////////////////////////////////////////////////////
void CullTraverserData::
apply_transform_and_state(CullTraverser *trav,
                          CPT(TransformState) node_transform,
                          CPT(RenderState) node_state,
                          CPT(RenderEffects) node_effects,
                          const RenderAttrib *off_clip_planes) {
  if (node_effects->has_cull_callback()) {
    node_effects->cull_callback(trav, *this, node_transform, node_state);
  }

  if (!node_transform->is_identity()) {
    _net_transform = _net_transform->compose(node_transform);

    if ((_view_frustum != (GeometricBoundingVolume *)NULL) ||
        (!_cull_planes->is_empty())) {
      // We need to move the viewing frustums into the node's
      // coordinate space by applying the node's inverse transform.
      if (node_transform->is_singular()) {
        // But we can't invert a singular transform!  Instead of
        // trying, we'll just give up on frustum culling from this
        // point down.
        _view_frustum = (GeometricBoundingVolume *)NULL;
        _cull_planes = CullPlanes::make_empty();

      } else {
        CPT(TransformState) inv_transform =
          node_transform->invert_compose(TransformState::make_identity());

        // Copy the bounding volumes for the frustums so we can
        // transform them.
        if (_view_frustum != (GeometricBoundingVolume *)NULL) {
          _view_frustum = DCAST(GeometricBoundingVolume, _view_frustum->make_copy());
          _view_frustum->xform(inv_transform->get_mat());
        }

        _cull_planes = _cull_planes->xform(inv_transform->get_mat());
      }
    }
  }

  _state = _state->compose(node_state);

  if (clip_plane_cull) {
    _cull_planes = _cull_planes->apply_state(trav, this,
                                             DCAST(ClipPlaneAttrib, node_state->get_attrib(ClipPlaneAttrib::get_class_slot())),
                                             DCAST(ClipPlaneAttrib, off_clip_planes),
                                             DCAST(OccluderEffect, node_effects->get_effect(OccluderEffect::get_class_type())));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::is_in_view_impl
//       Access: Private
//  Description: The private implementation of is_in_view().
////////////////////////////////////////////////////////////////////
bool CullTraverserData::
is_in_view_impl() {
  const GeometricBoundingVolume *node_gbv = NULL;

  if (_view_frustum != (GeometricBoundingVolume *)NULL) {
    DCAST_INTO_R(node_gbv, _node_reader.get_bounds(), false)

    int result = _view_frustum->contains(node_gbv);

    if (pgraph_cat.is_spam()) {
      pgraph_cat.spam()
        << _node_path << " cull result = " << hex << result << dec << "\n";
    }

    if (result == BoundingVolume::IF_no_intersection) {
      // No intersection at all.  Cull.
#ifdef NDEBUG
      return false;
#else
      if (!fake_view_frustum_cull) {
        return false;
      }

      // If we have fake view-frustum culling enabled, instead of
      // actually culling an object we simply force it to be drawn in
      // red wireframe.
      _view_frustum = (GeometricBoundingVolume *)NULL;
      CPT(RenderState) fake_state = get_fake_view_frustum_cull_state();
      _state = _state->compose(fake_state);
#endif

    } else if ((result & BoundingVolume::IF_all) != 0) {
      // The node and its descendents are completely enclosed within
      // the frustum.  No need to cull further.
      _view_frustum = (GeometricBoundingVolume *)NULL;

    } else {
      // The node is partially, but not completely, within the viewing
      // frustum.
      if (_node_reader.is_final()) {
        // Normally we'd keep testing child bounding volumes as we
        // continue down.  But this node has the "final" flag, so the
        // user is claiming that there is some important reason we
        // should consider everything visible at this point.  So be it.
        _view_frustum = (GeometricBoundingVolume *)NULL;
      }
    }
  }

  if (!_cull_planes->is_empty()) {
    if (node_gbv == (const GeometricBoundingVolume *)NULL) {
      DCAST_INTO_R(node_gbv, _node_reader.get_bounds(), false)
    }

    // Also cull against the current clip planes.
    int result;
    _cull_planes = _cull_planes->do_cull(result, _state, node_gbv);

    if (pgraph_cat.is_spam()) {
      pgraph_cat.spam()
        << _node_path << " cull planes cull result = " << hex
        << result << dec << "\n";
      _cull_planes->write(pgraph_cat.spam(false));
    }

    if (_node_reader.is_final()) {
      // Even though the node may be partially within the clip planes,
      // do no more culling against them below this node.
      _cull_planes = CullPlanes::make_empty();

      if (pgraph_cat.is_spam()) {
        pgraph_cat.spam()
          << _node_path << " is_final, cull planes disabled, state:\n";
        _state->write(pgraph_cat.spam(false), 2);
      }
    }

    if (result == BoundingVolume::IF_no_intersection) {
      // No intersection at all.  Cull.
#ifdef NDEBUG
      return false;
#else
      if (!fake_view_frustum_cull) {
        return false;
      }
      _cull_planes = CullPlanes::make_empty();
      CPT(RenderState) fake_state = get_fake_view_frustum_cull_state();
      _state = _state->compose(fake_state);
#endif

    } else if ((result & BoundingVolume::IF_all) != 0) {
      // The node and its descendents are completely in front of all
      // of the clip planes and occluders.  The do_cull() call should
      // therefore have removed all of the clip planes and occluders.
      nassertr(_cull_planes->is_empty(), true);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CullTraverserData::get_fake_view_frustum_cull_state
//       Access: Private, Static
//  Description: Returns a RenderState for rendering stuff in red
//               wireframe, strictly for the fake_view_frustum_cull
//               effect.
////////////////////////////////////////////////////////////////////
CPT(RenderState) CullTraverserData::
get_fake_view_frustum_cull_state() {
#ifdef NDEBUG
  return NULL;
#else
  // Once someone asks for this pointer, we hold its reference count
  // and never free it.
  static CPT(RenderState) state = (const RenderState *)NULL;
  if (state == (const RenderState *)NULL) {
    state = RenderState::make
      (ColorAttrib::make_flat(LColor(1.0f, 0.0f, 0.0f, 1.0f)),
       TextureAttrib::make_all_off(),
       RenderModeAttrib::make(RenderModeAttrib::M_wireframe),
       RenderState::get_max_priority());
  }
  return state;
#endif
}
