/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullPlanes.cxx
 * @author drose
 * @date 2005-08-23
 */

#include "cullPlanes.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "clipPlaneAttrib.h"
#include "occluderEffect.h"
#include "boundingBox.h"

using std::max;
using std::min;

/**
 * Returns a pointer to an empty CullPlanes object.
 */
CPT(CullPlanes) CullPlanes::
make_empty() {
  static CPT(CullPlanes) empty;
  if (empty == nullptr) {
    empty = new CullPlanes;
    // Artificially tick the reference count, just to ensure we won't
    // accidentally modify this object in any of the copy-on-write operations
    // below.
    empty->ref();
  }
  return empty;
}

/**
 * Returns a pointer to a new CullPlanes object that is the same as this one,
 * but with the clip planes modified by the indicated transform.
 */
CPT(CullPlanes) CullPlanes::
xform(const LMatrix4 &mat) const {
  PT(CullPlanes) new_planes;
  if (get_ref_count() == 1) {
    new_planes = (CullPlanes *)this;
  } else {
    new_planes = new CullPlanes(*this);
  }

  for (Planes::iterator pi = new_planes->_planes.begin();
       pi != new_planes->_planes.end();
       ++pi) {
    if ((*pi).second->get_ref_count() != 1) {
      (*pi).second = DCAST(BoundingPlane, (*pi).second->make_copy());
    }
    (*pi).second->xform(mat);
  }

  for (Occluders::iterator oi = new_planes->_occluders.begin();
       oi != new_planes->_occluders.end();
       ++oi) {
    if ((*oi).second->get_ref_count() != 1) {
      (*oi).second = DCAST(BoundingHexahedron, (*oi).second->make_copy());
    }
    (*oi).second->xform(mat);
  }

  return new_planes;
}

/**
 * Returns a pointer to a new CullPlanes object that is the same as this one,
 * but with the indicated attributes applied to the state.
 *
 * In particular, any new ClipPlanes given in net_attrib, if it is not NULL,
 * will be added to the state, unless those ClipPlanes are also listed in
 * off_attrib.
 */
CPT(CullPlanes) CullPlanes::
apply_state(const CullTraverser *trav, const CullTraverserData *data,
            const ClipPlaneAttrib *net_attrib,
            const ClipPlaneAttrib *off_attrib,
            const OccluderEffect *node_effect) const {
  if (net_attrib == nullptr && node_effect == nullptr) {
    return this;
  }

  PT(CullPlanes) new_planes;
  if (get_ref_count() == 1) {
    new_planes = (CullPlanes *)this;
  } else {
    new_planes = new CullPlanes(*this);
  }

  CPT(TransformState) net_transform = nullptr;

  if (net_attrib != nullptr) {
    int num_on_planes = net_attrib->get_num_on_planes();
    for (int i = 0; i < num_on_planes; ++i) {
      NodePath clip_plane = net_attrib->get_on_plane(i);
      Planes::const_iterator pi = new_planes->_planes.find(clip_plane);
      if (pi == new_planes->_planes.end()) {
        if (!off_attrib->has_off_plane(clip_plane)) {
          // Here's a new clip plane; add it to the list.  For this we need
          // the net transform to this node.
          if (net_transform == nullptr) {
            net_transform = data->get_net_transform(trav);
          }

          PlaneNode *plane_node = DCAST(PlaneNode, clip_plane.node());
          CPT(TransformState) new_transform =
            net_transform->invert_compose(clip_plane.get_net_transform());

          LPlane plane = plane_node->get_plane() * new_transform->get_mat();
          new_planes->_planes[clip_plane] = new BoundingPlane(-plane);
        }
      }
    }
  }

  if (node_effect != nullptr) {
    CPT(TransformState) center_transform = nullptr;
    // We'll need to know the occluder's frustum in cull-center space.
    SceneSetup *scene = trav->get_scene();
    const Lens *lens = scene->get_lens();

    int num_on_occluders = node_effect->get_num_on_occluders();
    for (int i = 0; i < num_on_occluders; ++i) {
      NodePath occluder = node_effect->get_on_occluder(i);
      Occluders::const_iterator oi = new_planes->_occluders.find(occluder);
      if (oi == new_planes->_occluders.end()) {
        // Here's a new occluder; consider adding it to the list.
        OccluderNode *occluder_node = DCAST(OccluderNode, occluder.node());
        nassertr(occluder_node->get_num_vertices() == 4, new_planes);

        CPT(TransformState) occluder_transform = occluder.get_transform(scene->get_cull_center());

        // And the transform from cull-center space into the current node's
        // coordinate space.
        if (center_transform == nullptr) {
          if (net_transform == nullptr) {
            net_transform = data->get_net_transform(trav);
          }

          center_transform = net_transform->invert_compose(scene->get_cull_center().get_net_transform());
        }

        // Compare the occluder node's bounding volume to the view frustum.
        // We construct a new bounding volume because (a) the node's existing
        // bounding volume is in the coordinate space of its parent, which
        // isn't what we have here, and (b) we might as well make a
        // BoundingBox, which is as tight as possible, and creating one isn't
        // any less efficient than transforming the existing bounding volume.
        PT(BoundingBox) occluder_gbv;
        // Get a transform from the occluder directly to this node's space for
        // comparing with the current view frustum.
        CPT(TransformState) composed_transform = center_transform->compose(occluder_transform);
        const LMatrix4 &composed_mat = composed_transform->get_mat();
        LPoint3 ccp[4];
        ccp[0] = occluder_node->get_vertex(0) * composed_mat;
        ccp[1] = occluder_node->get_vertex(1) * composed_mat;
        ccp[2] = occluder_node->get_vertex(2) * composed_mat;
        ccp[3] = occluder_node->get_vertex(3) * composed_mat;

        LPoint3 ccp_min(min(min(ccp[0][0], ccp[1][0]),
                     min(ccp[2][0], ccp[3][0])),
                 min(min(ccp[0][1], ccp[1][1]),
                     min(ccp[2][1], ccp[3][1])),
                 min(min(ccp[0][2], ccp[1][2]),
                     min(ccp[2][2], ccp[3][2])));
        LPoint3 ccp_max(max(max(ccp[0][0], ccp[1][0]),
                     max(ccp[2][0], ccp[3][0])),
                 max(max(ccp[0][1], ccp[1][1]),
                     max(ccp[2][1], ccp[3][1])),
                 max(max(ccp[0][2], ccp[1][2]),
                     max(ccp[2][2], ccp[3][2])));

        occluder_gbv = new BoundingBox(ccp_min, ccp_max);

        if (data->_view_frustum != nullptr) {
          int occluder_result = data->_view_frustum->contains(occluder_gbv);
          if (occluder_result == BoundingVolume::IF_no_intersection) {
            // This occluder is outside the view frustum; ignore it.
            if (pgraph_cat.is_spam()) {
              pgraph_cat.spam()
            << "Ignoring occluder " << occluder << ": outside view frustum.\n";
            }
            continue;
          }
        }

        // Get the occluder geometry in cull-center space.
        const LMatrix4 &occluder_mat_cull = occluder_transform->get_mat();
        LPoint3 points_near[4];
        points_near[0] = occluder_node->get_vertex(0) * occluder_mat_cull;
        points_near[1] = occluder_node->get_vertex(1) * occluder_mat_cull;
        points_near[2] = occluder_node->get_vertex(2) * occluder_mat_cull;
        points_near[3] = occluder_node->get_vertex(3) * occluder_mat_cull;
        LPlane plane(points_near[0], points_near[1], points_near[2]);

        if (plane.get_normal().dot(LVector3::forward()) >= 0.0) {
          if (occluder_node->is_double_sided()) {
            std::swap(points_near[0], points_near[3]);
            std::swap(points_near[1], points_near[2]);
            plane = LPlane(points_near[0], points_near[1], points_near[2]);
          } else {
            // This occluder is facing the wrong direction.  Ignore it.
            if (pgraph_cat.is_spam()) {
              pgraph_cat.spam()
                << "Ignoring occluder " << occluder << ": wrong direction.\n";
            }
            continue;
          }
        }

        if (occluder_node->get_min_coverage()) {
          LPoint3 coords[4];
          lens->project(points_near[0], coords[0]);
          lens->project(points_near[1], coords[1]);
          lens->project(points_near[2], coords[2]);
          lens->project(points_near[3], coords[3]);
          coords[0][0] = max((PN_stdfloat)-1.0, min((PN_stdfloat)1.0, coords[0][0]));
          coords[0][1] = max((PN_stdfloat)-1.0, min((PN_stdfloat)1.0, coords[0][1]));
          coords[1][0] = max((PN_stdfloat)-1.0, min((PN_stdfloat)1.0, coords[1][0]));
          coords[1][1] = max((PN_stdfloat)-1.0, min((PN_stdfloat)1.0, coords[1][1]));
          coords[2][0] = max((PN_stdfloat)-1.0, min((PN_stdfloat)1.0, coords[2][0]));
          coords[2][1] = max((PN_stdfloat)-1.0, min((PN_stdfloat)1.0, coords[2][1]));
          coords[3][0] = max((PN_stdfloat)-1.0, min((PN_stdfloat)1.0, coords[3][0]));
          coords[3][1] = max((PN_stdfloat)-1.0, min((PN_stdfloat)1.0, coords[3][1]));
          PN_stdfloat coverage = ((coords[0] - coords[1]).cross(coords[0] - coords[2]).length()
                          + (coords[3] - coords[1]).cross(coords[3] - coords[2]).length())
                          * 0.125;
          if (coverage < occluder_node->get_min_coverage()) {
            // The occluder does not cover enough screen space.  Ignore it.
            if (pgraph_cat.is_spam()) {
              pgraph_cat.spam()
                << "Ignoring occluder " << occluder << ": coverage less than minimum.\n";
            }
            continue;
          }
        }

        // Also check if the new occluder is completely within any of our
        // existing occluder volumes.
        bool is_enclosed = false;
        Occluders::const_iterator oi;
        for (oi = _occluders.begin(); oi != _occluders.end(); ++oi) {
          int occluder_result = (*oi).second->contains(occluder_gbv);
          if ((occluder_result & BoundingVolume::IF_all) != 0) {
            is_enclosed = true;
            break;
          }
        }
        if (is_enclosed) {
          // No reason to add this occluder; it's behind an existing occluder.
          if (pgraph_cat.is_spam()) {
            pgraph_cat.spam()
              << "Ignoring occluder " << occluder << ": behind another.\n";
          }
          continue;
        }
        // TODO: perhaps we should also check whether any existing occluders
        // are fully contained within this new one.

        // Get the occluder coordinates in global space.
        const LMatrix4 &occluder_mat = occluder.get_net_transform()->get_mat();
        points_near[0] = occluder_node->get_vertex(0) * occluder_mat;
        points_near[1] = occluder_node->get_vertex(1) * occluder_mat;
        points_near[2] = occluder_node->get_vertex(2) * occluder_mat;
        points_near[3] = occluder_node->get_vertex(3) * occluder_mat;

        // For the far points, project PAST the far clip of the lens to
        // ensures we get stuff that might be intersecting the far clip.
        LPoint3 center = scene->get_cull_center().get_net_transform()->get_pos();
        PN_stdfloat far_clip = scene->get_lens()->get_far() * 2.0;
        LPoint3 points_far[4];
        points_far[0] = normalize(points_near[0] - center) * far_clip + points_near[0];
        points_far[1] = normalize(points_near[1] - center) * far_clip + points_near[1];
        points_far[2] = normalize(points_near[2] - center) * far_clip + points_near[2];
        points_far[3] = normalize(points_near[3] - center) * far_clip + points_near[3];

        // With these points, construct the bounding frustum of the occluded
        // region.
        PT(BoundingHexahedron) frustum =
          new BoundingHexahedron(points_far[1], points_far[2], points_far[3], points_far[0],
                                 points_near[1], points_near[2], points_near[3], points_near[0]);

        new_planes->_occluders[occluder] = frustum;

        if (show_occluder_volumes) {
          // Draw the frustum for visualization.
          nassertr(net_transform != nullptr, new_planes);
          trav->draw_bounding_volume(frustum, data->get_internal_transform(trav));
        }
      }
    }
  }

  return new_planes;
}

/**
 * Tests the indicated bounding volume against all of the clip planes in this
 * object.  Sets result to an appropriate union of
 * BoundingVolume::IntersectionFlags, similar to the result of
 * BoundingVolume::contains().
 *
 * Also, if the bounding volume is completely in front of any of the clip
 * planes, removes those planes both from this object and from the indicated
 * state, returning a new CullPlanes object in that case.
 */
CPT(CullPlanes) CullPlanes::
do_cull(int &result, CPT(RenderState) &state,
        const GeometricBoundingVolume *node_gbv) const {
  result =
    BoundingVolume::IF_all | BoundingVolume::IF_possible | BoundingVolume::IF_some;

  CPT(CullPlanes) new_planes = this;

  const ClipPlaneAttrib *orig_cpa;
  if (!state->get_attrib(orig_cpa)) {
    // If there are no clip planes in the state, the node is completely in
    // front of all zero of the clip planes.  (This can happen if someone
    // directly changes the state during the traversal.)
    CullPlanes *planes = new CullPlanes;
    planes->_occluders = _occluders;
    new_planes = planes;

  } else {
    CPT(ClipPlaneAttrib) new_cpa = orig_cpa;

    Planes::const_iterator pi;
    for (pi = _planes.begin(); pi != _planes.end(); ++pi) {
      int plane_result = (*pi).second->contains(node_gbv);
      if (plane_result == BoundingVolume::IF_no_intersection) {
        // The node is completely behind this clip plane and gets culled.
        // Short-circuit the rest of the logic; none of the other planes
        // matter.
        result = plane_result;
        return new_planes;
      } else if ((plane_result & BoundingVolume::IF_all) != 0) {
        // The node is completely in front of this clip plane.  We don't need
        // to consider this plane ever again for any descendents of this node.
        new_planes = new_planes->remove_plane((*pi).first);
        nassertr(new_planes != this, new_planes);
        new_cpa = DCAST(ClipPlaneAttrib, new_cpa->remove_on_plane((*pi).first));
      }

      result &= plane_result;
    }

    if (new_cpa != orig_cpa) {
      if (new_cpa->is_identity()) {
        state = state->remove_attrib(ClipPlaneAttrib::get_class_slot());
      } else {
        state = state->add_attrib(new_cpa);
      }
    }
  }

  Occluders::const_iterator oi;
  for (oi = _occluders.begin(); oi != _occluders.end(); ++oi) {
    int occluder_result = (*oi).second->contains(node_gbv);
    if (occluder_result == BoundingVolume::IF_no_intersection) {
      // The node is completely in front of this occluder.  We don't need to
      // consider this occluder ever again for any descendents of this node.

      // Reverse the sense of the test, because an occluder volume is the
      // inverse of a cull plane volume: it describes the volume that is to be
      // culled, not the volume that is to be kept.
      occluder_result = BoundingVolume::IF_all | BoundingVolume::IF_possible | BoundingVolume::IF_some;
      new_planes = new_planes->remove_occluder((*oi).first);
      nassertr(new_planes != this, new_planes);

    } else if ((occluder_result & BoundingVolume::IF_all) != 0) {
      // The node is completely behind this occluder and gets culled.  Short-
      // circuit the rest of the logic; none of the other occluders matter.
      result = BoundingVolume::IF_no_intersection;
      return new_planes;
    }

    result &= occluder_result;
  }

  return new_planes;
}

/**
 * Returns a pointer to a new CullPlanes object that is the same as this one,
 * but with the indicated clip plane removed.
 */
CPT(CullPlanes) CullPlanes::
remove_plane(const NodePath &clip_plane) const {
  PT(CullPlanes) new_planes;
  if (get_ref_count() == 1) {
    new_planes = (CullPlanes *)this;
  } else {
    new_planes = new CullPlanes(*this);
  }

  Planes::iterator pi = new_planes->_planes.find(clip_plane);
  nassertr(pi != new_planes->_planes.end(), new_planes);
  new_planes->_planes.erase(pi);

  return new_planes;
}

/**
 * Returns a pointer to a new CullPlanes object that is the same as this one,
 * but with the indicated occluder removed.
 */
CPT(CullPlanes) CullPlanes::
remove_occluder(const NodePath &occluder) const {
  PT(CullPlanes) new_planes;
  if (get_ref_count() == 1) {
    new_planes = (CullPlanes *)this;
  } else {
    new_planes = new CullPlanes(*this);
  }

  Occluders::iterator pi = new_planes->_occluders.find(occluder);
  nassertr(pi != new_planes->_occluders.end(), new_planes);
  new_planes->_occluders.erase(pi);

  return new_planes;
}

/**
 *
 */
void CullPlanes::
write(std::ostream &out) const {
  out << "CullPlanes (" << _planes.size() << " planes and "
      << _occluders.size() << " occluders):\n";
  Planes::const_iterator pi;
  for (pi = _planes.begin(); pi != _planes.end(); ++pi) {
    out << "  " << (*pi).first << " : " << *(*pi).second << "\n";
  }

  Occluders::const_iterator oi;
  for (oi = _occluders.begin(); oi != _occluders.end(); ++oi) {
    out << "  " << (*oi).first << " : " << *(*oi).second << "\n";
  }
}
