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
    const ClipPlaneAttrib *cpa = (const ClipPlaneAttrib *)
      node_state->get_attrib(ClipPlaneAttrib::get_class_slot());
    const OccluderEffect *occluders = (const OccluderEffect *)
      node_effects->get_effect(OccluderEffect::get_class_type());
    const ClipPlaneAttrib *off_cpa = nullptr;

    if (cpa != nullptr) {
      _node_reader.check_cached(false);
      off_cpa = (const ClipPlaneAttrib *)_node_reader.get_off_clip_planes();
    }

    if (cpa != nullptr || occluders != nullptr) {
      CullPlanes::apply_state(_cull_planes, trav, this, cpa, off_cpa, occluders);
    }
  }

  const FogAttrib *fog_attr;
  if (node_state->get_attrib(fog_attr)) {
    Fog *fog = fog_attr->get_fog();
    if (fog != nullptr) {
      // If we just introduced a FogAttrib here, call adjust_to_camera()
      // now.  This maybe isn't the perfect time to call it, but it's good
      // enough; and at this time we have all the information we need for
      // it.
      fog->adjust_to_camera(trav->get_camera_transform());
    }
  }
}

/**
 * Applies the indicated transform changes onto the current data.
 */
void CullTraverserData::
apply_transform(const TransformState *node_transform) {
  if (!node_transform->is_identity()) {
    if (_instances != nullptr) {
      InstanceList *instances = new InstanceList(*_instances);
      for (InstanceList::Instance &instance : *instances) {
        instance.set_transform(instance.get_transform()->compose(node_transform));
      }
      _instances = std::move(instances);
      return;
    }

    _net_transform = _net_transform->compose(node_transform);

    if (_view_frustum != nullptr || _cull_planes != nullptr) {
      // We need to move the viewing frustums into the node's coordinate space
      // by applying the node's inverse transform.
      const LMatrix4 *inverse_mat = node_transform->get_inverse_mat();
      if (inverse_mat != nullptr) {
        // Copy the bounding volumes for the frustums so we can transform
        // them.
        if (_view_frustum != nullptr) {
          _view_frustum = _view_frustum->make_copy()->as_geometric_bounding_volume();
          nassertv(_view_frustum != nullptr);

          _view_frustum->xform(*inverse_mat);
        }

        if (_cull_planes != nullptr) {
          _cull_planes = _cull_planes->xform(*inverse_mat);
        }
      }
      else {
        // But we can't invert a singular transform!  Instead of trying, we'll
        // just give up on frustum culling from this point down.
        pgraph_cat.warning()
          << "Singular transformation detected on node: " << get_node_path() << "\n";
        _view_frustum = nullptr;
        _cull_planes = nullptr;
      }
    }
  }
}

/**
 * Returns intersection flags if any of the children under the current node are
 * in view if first transformed by the given transform, false otherwise.
 */
bool CullTraverserData::
is_instance_in_view(const TransformState *instance_transform, const DrawMask &camera_mask) const {
  PT(GeometricBoundingVolume) view_frustum_p;
  const GeometricBoundingVolume *view_frustum = nullptr;

  if (_view_frustum != nullptr) {
    if (!instance_transform->is_identity()) {
      // We need to move the viewing frustums into the node's coordinate space
      // by applying the node's inverse transform.
      const LMatrix4 *inverse_mat = instance_transform->get_inverse_mat();
      if (inverse_mat != nullptr) {
        // Copy the bounding volumes for the frustums so we can transform them.
        view_frustum_p = _view_frustum->make_copy()->as_geometric_bounding_volume();
        nassertr(view_frustum_p != nullptr, false);

        view_frustum_p->xform(*inverse_mat);
        view_frustum = view_frustum_p;
      } else {
        // Don't render instances with a singular transformation.
        return false;
      }
    } else {
      view_frustum = _view_frustum;
    }
  }

  PandaNode::Children children = _node_reader.get_children();
  int num_children = children.get_num_children();
  for (int i = 0; i < num_children; ++i) {
    const PandaNode::DownConnection &child = children.get_child_connection(i);

    if (!child.compare_draw_mask(_draw_mask, camera_mask)) {
      // If there are no draw bits in common with the camera, the node is out.
      continue;
    }

    if (view_frustum == nullptr) {
      return true;
    }

    const GeometricBoundingVolume *node_gbv = child.get_bounds();
    nassertd(node_gbv != nullptr) continue;

    if (view_frustum->contains(node_gbv)) {
      return true;
    }
  }
  return false;
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
 * Applies the cull planes.  Returns true if the node should still be rendered,
 * false if it should be culled.
 */
bool CullTraverserData::
apply_cull_planes(const CullPlanes *planes, const GeometricBoundingVolume *node_gbv) {
  if (!planes->is_empty()) {
    // Also cull against the current clip planes.
    int result;
    CPT(CullPlanes) new_planes = planes->do_cull(result, _state, node_gbv);

    if (pgraph_cat.is_spam()) {
      pgraph_cat.spam()
        << get_node_path() << " cull planes cull result = " << std::hex
        << result << std::dec << "\n";
      new_planes->write(pgraph_cat.spam(false));
    }

    if (result == BoundingVolume::IF_no_intersection) {
      // No intersection at all.  Cull.
      return false;
    }
    else if ((result & BoundingVolume::IF_all) != 0) {
      // The node and its descendants are completely in front of all of the
      // clip planes and occluders.  The do_cull() call should therefore have
      // removed all of the clip planes and occluders.
      nassertr(new_planes->is_empty(), true);
    }
    else if (!_node_reader.is_final() && !new_planes->is_empty()) {
      _cull_planes = std::move(new_planes);
    }
  }

  return true;
}
