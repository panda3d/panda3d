/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullTraverserData.cxx
 * @author drose
 * @date 2002-03-06
 */

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


/**
 * Applies the transform and state from the current node onto the current
 * data.  This also evaluates billboards, etc.
 */
void CullTraverserData::
apply_transform_and_state(CullTraverser *trav) {
  CPT(RenderState) node_state = _node_reader.get_state();

  if (trav->has_tag_state_key() &&
      _node_reader.has_tag(trav->get_tag_state_key())) {
    // Here's a node that has been tagged with the special key for our current
    // camera.  This indicates some special state transition for this node,
    // which is unique to this camera.
    const Camera *camera = trav->get_scene()->get_camera_node();
    std::string tag_state = _node_reader.get_tag(trav->get_tag_state_key());
    node_state = node_state->compose(camera->get_tag_state(tag_state));
  }
  _node_reader.compose_draw_mask(_draw_mask);

  const RenderEffects *node_effects = _node_reader.get_effects();
  if (!node_effects->has_cull_callback()) {
    apply_transform(_node_reader.get_transform());
  } else {
    // The cull callback may decide to modify the node_transform.
    CPT(TransformState) node_transform = _node_reader.get_transform();
    node_effects->cull_callback(trav, *this, node_transform, node_state);
    apply_transform(node_transform);

    // The cull callback may have changed the node properties.
    _node_reader.check_cached(false);
  }

  if (!node_state->is_empty()) {
    _state = _state->compose(node_state);
  }

  if (clip_plane_cull) {
    _cull_planes = _cull_planes->apply_state(trav, this,
      (const ClipPlaneAttrib *)node_state->get_attrib(ClipPlaneAttrib::get_class_slot()),
      (const ClipPlaneAttrib *)_node_reader.get_off_clip_planes(),
      (const OccluderEffect *)node_effects->get_effect(OccluderEffect::get_class_type()));
  }
}

/**
 * Applies the indicated transform changes onto the current data.
 */
void CullTraverserData::
apply_transform(const TransformState *node_transform) {
  if (!node_transform->is_identity()) {
    _net_transform = _net_transform->compose(node_transform);

    if ((_view_frustum != nullptr) ||
        (!_cull_planes->is_empty())) {
      // We need to move the viewing frustums into the node's coordinate space
      // by applying the node's inverse transform.
      if (node_transform->is_singular()) {
        // But we can't invert a singular transform!  Instead of trying, we'll
        // just give up on frustum culling from this point down.
        _view_frustum = nullptr;
        _cull_planes = CullPlanes::make_empty();

      } else {
        CPT(TransformState) inv_transform =
          node_transform->invert_compose(TransformState::make_identity());

        // Copy the bounding volumes for the frustums so we can transform
        // them.
        if (_view_frustum != nullptr) {
          _view_frustum = _view_frustum->make_copy()->as_geometric_bounding_volume();
          nassertv(_view_frustum != nullptr);

          _view_frustum->xform(inv_transform->get_mat());
        }

        _cull_planes = _cull_planes->xform(inv_transform->get_mat());
      }
    }
  }
}

/**
 * The private, recursive implementation of get_node_path(), this returns the
 * NodePathComponent representing the NodePath.
 */
PT(NodePathComponent) CullTraverserData::
r_get_node_path() const {
  if (_next == nullptr) {
    nassertr(_start != nullptr, nullptr);
    return _start;
  }

#ifdef _DEBUG
  nassertr(_start == nullptr, nullptr);
#endif
  nassertr(node() != nullptr, nullptr);

  PT(NodePathComponent) comp = _next->r_get_node_path();
  nassertr(comp != nullptr, nullptr);

  Thread *current_thread = Thread::get_current_thread();
  int pipeline_stage = current_thread->get_pipeline_stage();
  PT(NodePathComponent) result =
    PandaNode::get_component(comp, node(), pipeline_stage, current_thread);
  if (result == nullptr) {
    // This means we found a disconnected chain in the CullTraverserData's
    // ancestry: the node above this node isn't connected.  In this case,
    // don't attempt to go higher; just truncate the NodePath at the bottom of
    // the disconnect.
    return PandaNode::get_top_component(node(), true, pipeline_stage, current_thread);
  }

  return result;
}

/**
 * The private implementation of is_in_view().
 */
bool CullTraverserData::
is_in_view_impl() {
  const GeometricBoundingVolume *node_gbv = nullptr;

  if (_view_frustum != nullptr) {
    node_gbv = _node_reader.get_bounds()->as_geometric_bounding_volume();
    nassertr(node_gbv != nullptr, false);

    int result = _view_frustum->contains(node_gbv);

    if (pgraph_cat.is_spam()) {
      pgraph_cat.spam()
        << get_node_path() << " cull result = " << std::hex << result << std::dec << "\n";
    }

    if (result == BoundingVolume::IF_no_intersection) {
      // No intersection at all.  Cull.
#ifdef NDEBUG
      return false;
#else
      if (!fake_view_frustum_cull) {
        return false;
      }

      // If we have fake view-frustum culling enabled, instead of actually
      // culling an object we simply force it to be drawn in red wireframe.
      _view_frustum = nullptr;
      _state = _state->compose(get_fake_view_frustum_cull_state());
#endif

    } else if ((result & BoundingVolume::IF_all) != 0) {
      // The node and its descendents are completely enclosed within the
      // frustum.  No need to cull further.
      _view_frustum = nullptr;

    } else {
      // The node is partially, but not completely, within the viewing
      // frustum.
      if (_node_reader.is_final()) {
        // Normally we'd keep testing child bounding volumes as we continue
        // down.  But this node has the "final" flag, so the user is claiming
        // that there is some important reason we should consider everything
        // visible at this point.  So be it.
        _view_frustum = nullptr;
      }
    }
  }

  if (!_cull_planes->is_empty()) {
    if (node_gbv == nullptr) {
      node_gbv = _node_reader.get_bounds()->as_geometric_bounding_volume();
      nassertr(node_gbv != nullptr, false);
    }

    // Also cull against the current clip planes.
    int result;
    _cull_planes = _cull_planes->do_cull(result, _state, node_gbv);

    if (pgraph_cat.is_spam()) {
      pgraph_cat.spam()
        << get_node_path() << " cull planes cull result = " << std::hex
        << result << std::dec << "\n";
      _cull_planes->write(pgraph_cat.spam(false));
    }

    if (_node_reader.is_final()) {
      // Even though the node may be partially within the clip planes, do no
      // more culling against them below this node.
      _cull_planes = CullPlanes::make_empty();

      if (pgraph_cat.is_spam()) {
        pgraph_cat.spam()
          << get_node_path() << " is_final, cull planes disabled, state:\n";
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
      _state = _state->compose(get_fake_view_frustum_cull_state());
#endif

    } else if ((result & BoundingVolume::IF_all) != 0) {
      // The node and its descendents are completely in front of all of the
      // clip planes and occluders.  The do_cull() call should therefore have
      // removed all of the clip planes and occluders.
      nassertr(_cull_planes->is_empty(), true);
    }
  }

  return true;
}

/**
 * Returns a RenderState for rendering stuff in red wireframe, strictly for
 * the fake_view_frustum_cull effect.
 */
const RenderState *CullTraverserData::
get_fake_view_frustum_cull_state() {
#ifdef NDEBUG
  return nullptr;
#else
  // Once someone asks for this pointer, we hold its reference count and never
  // free it.
  static CPT(RenderState) state;
  if (state == nullptr) {
    state = RenderState::make
      (ColorAttrib::make_flat(LColor(1.0f, 0.0f, 0.0f, 1.0f)),
       TextureAttrib::make_all_off(),
       RenderModeAttrib::make(RenderModeAttrib::M_wireframe),
       RenderState::get_max_priority());
  }
  return state;
#endif
}
