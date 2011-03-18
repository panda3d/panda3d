// Filename: cullPlanes.cxx
// Created by:  drose (23Aug05)
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

#include "cullPlanes.h"
#include "cullTraverserData.h"
#include "clipPlaneAttrib.h"
#include "occluderEffect.h"


////////////////////////////////////////////////////////////////////
//     Function: CullPlanes::make_empty
//       Access: Public, Static
//  Description: Returns a pointer to an empty CullPlanes object.
////////////////////////////////////////////////////////////////////
CPT(CullPlanes) CullPlanes::
make_empty() {
  static CPT(CullPlanes) empty;
  if (empty == NULL) {
    empty = new CullPlanes;
    // Artificially tick the reference count, just to ensure we won't
    // accidentally modify this object in any of the copy-on-write
    // operations below.
    empty->ref();
  }
  return empty;
}

////////////////////////////////////////////////////////////////////
//     Function: CullPlanes::xform
//       Access: Public
//  Description: Returns a pointer to a new CullPlanes object that is
//               the same as this one, but with the clip planes
//               modified by the indicated transform.
////////////////////////////////////////////////////////////////////
CPT(CullPlanes) CullPlanes::
xform(const LMatrix4f &mat) const {
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

////////////////////////////////////////////////////////////////////
//     Function: CullPlanes::apply_state
//       Access: Public
//  Description: Returns a pointer to a new CullPlanes object that is
//               the same as this one, but with the indicated
//               attributes applied to the state.
//
//               In particular, any new ClipPlanes given in
//               net_attrib, if it is not NULL, will be added to the
//               state, unless those ClipPlanes are also listed in
//               off_attrib.
////////////////////////////////////////////////////////////////////
CPT(CullPlanes) CullPlanes::
apply_state(const CullTraverser *trav, const CullTraverserData *data,
            const ClipPlaneAttrib *net_attrib,
            const ClipPlaneAttrib *off_attrib,
            const OccluderEffect *node_effect) const {
  if (net_attrib == (ClipPlaneAttrib *)NULL && node_effect == (OccluderEffect *)NULL) {
    return this;
  }

  PT(CullPlanes) new_planes;
  if (get_ref_count() == 1) {
    new_planes = (CullPlanes *)this;
  } else {
    new_planes = new CullPlanes(*this);
  }

  CPT(TransformState) net_transform = NULL;

  if (net_attrib != (ClipPlaneAttrib *)NULL) {
    int num_on_planes = net_attrib->get_num_on_planes();
    for (int i = 0; i < num_on_planes; ++i) {
      NodePath clip_plane = net_attrib->get_on_plane(i);
      Planes::const_iterator pi = new_planes->_planes.find(clip_plane);
      if (pi == new_planes->_planes.end()) {
        if (!off_attrib->has_off_plane(clip_plane)) {
          // Here's a new clip plane; add it to the list.  For this we
          // need the net transform to this node.
          if (net_transform == (TransformState *)NULL) {
            net_transform = data->get_net_transform(trav);
          }
          
          PlaneNode *plane_node = DCAST(PlaneNode, clip_plane.node());
          CPT(TransformState) new_transform = 
            net_transform->invert_compose(clip_plane.get_net_transform());
          
          Planef plane = plane_node->get_plane() * new_transform->get_mat();
          new_planes->_planes[clip_plane] = new BoundingPlane(-plane);
        }
      }
    }
  }

  if (node_effect != (OccluderEffect *)NULL) {
    CPT(TransformState) center_transform = NULL;

    int num_on_occluders = node_effect->get_num_on_occluders();
    for (int i = 0; i < num_on_occluders; ++i) {
      NodePath occluder = node_effect->get_on_occluder(i);
      Occluders::const_iterator oi = new_planes->_occluders.find(occluder);
      if (oi == new_planes->_occluders.end()) {
        // Here's a new occluder; consider adding it to the list.
        OccluderNode *occluder_node = DCAST(OccluderNode, occluder.node());
        nassertr(occluder_node->get_num_vertices() == 4, new_planes);
        
        // We'll need to know the occluder's frustum in cull-center
        // space.
        SceneSetup *scene = trav->get_scene();

        CPT(TransformState) occluder_transform = occluder.get_transform(scene->get_cull_center());

        // And the transform from cull-center space into the current
        // node's coordinate space.
        if (center_transform == (TransformState *)NULL) {
          if (net_transform == (TransformState *)NULL) {
            net_transform = data->get_net_transform(trav);
          }

          center_transform = net_transform->invert_compose(scene->get_cull_center().get_net_transform());
        }

        // Compare the occluder node's bounding volume to the view
        // frustum.
        PT(GeometricBoundingVolume) occluder_gbv = DCAST(GeometricBoundingVolume, occluder_node->get_internal_bounds()->make_copy());
        {
          CPT(TransformState) composed_transform = occluder_transform->compose(center_transform);
          occluder_gbv->xform(composed_transform->get_mat());
        }

        int occluder_result = data->_view_frustum->contains(occluder_gbv);
        if (occluder_result == BoundingVolume::IF_no_intersection) {
          // This occluder is outside the view frustum; ignore it.
          continue;
        }

        // Also check if the new occluder is completely within any of
        // our existing occluder volumes.
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
          // No reason to add this occluder; it's behind an existing
          // occluder.
          continue;
        }
        // TODO: perhaps we should also check whether any existing
        // occluders are fully contained within this new one.
        
        // Get the occluder geometry in cull-center space.
        const LMatrix4f &occluder_mat = occluder_transform->get_mat();
        Vertexf points_near[4];
        points_near[0] = occluder_node->get_vertex(0) * occluder_mat;
        points_near[1] = occluder_node->get_vertex(1) * occluder_mat;
        points_near[2] = occluder_node->get_vertex(2) * occluder_mat;
        points_near[3] = occluder_node->get_vertex(3) * occluder_mat;
        
        Planef plane(points_near[0], points_near[1], points_near[2]);
        if (plane.get_normal().dot(LVector3f::forward()) >= 0.0) {
          // This occluder is facing the wrong direction.  Ignore it.
          continue;
        }

        float near_clip = scene->get_lens()->get_near();
        if (plane.dist_to_plane(LPoint3f::zero()) <= near_clip) {
          // This occluder is behind the camera's near plane.  Ignore it.
          continue;
        }

        float d0 = points_near[0].dot(LVector3f::forward());
        float d1 = points_near[1].dot(LVector3f::forward());
        float d2 = points_near[2].dot(LVector3f::forward());
        float d3 = points_near[3].dot(LVector3f::forward());
        if (d0 <= near_clip && d1 <= near_clip && d2 <= near_clip && d3 <= near_clip) {
          // All four corners of the occluder are behind the camera's
          // near plane.  Ignore it.
          continue;
        }

        // TODO: it's possible for part of the occlusion polygon to
        // intersect the camera's y = 0 plane.  If this happens, the
        // frustum will go insane and the occluder won't work.  The
        // proper fix for this is to clip the polygon against the near
        // plane, producing a smaller polygon, and use that to
        // generate the frustum.  But maybe it doesn't matter.  In
        // lieu of this, we just toss out any polygon with *any*
        // corner behind the y = 0 plane.
        if (d0 <= 0.0 || d1 <= 0.0 || d2 <= 0.0 || d3 <= 0.0) {
          // One of the corners is behind the y = 0 plane.  We can't
          // handle this case.  Ignore it.
          continue;
        }

        // Project those four lines to the camera's far plane.
        float far_clip = scene->get_lens()->get_far();
        Planef far_plane(-LVector3f::forward(), LVector3f::forward() * far_clip);

        LPoint3f points_far[4];
        far_plane.intersects_line(points_far[0], LPoint3f::zero(), points_near[0]);
        far_plane.intersects_line(points_far[1], LPoint3f::zero(), points_near[1]);
        far_plane.intersects_line(points_far[2], LPoint3f::zero(), points_near[2]);
        far_plane.intersects_line(points_far[3], LPoint3f::zero(), points_near[3]);

        // With these points, construct the bounding frustum of the
        // occluded region.
        PT(BoundingHexahedron) frustum = 
          new BoundingHexahedron(points_far[1], points_far[2], points_far[3], points_far[0],  
                                 points_near[1], points_near[2], points_near[3], points_near[0]);
        frustum->xform(center_transform->get_mat());

        new_planes->_occluders[occluder] = frustum;
        
        if (show_occluder_volumes) {
          // Draw the frustum for visualization.
          nassertr(net_transform != NULL, new_planes);
          trav->draw_bounding_volume(frustum, net_transform, 
                                     data->get_modelview_transform(trav));
        }
      }
    }
  }
    
  return new_planes;
}

////////////////////////////////////////////////////////////////////
//     Function: CullPlanes::do_cull
//       Access: Public
//  Description: Tests the indicated bounding volume against all of
//               the clip planes in this object.  Sets result to an
//               appropriate union of
//               BoundingVolume::IntersectionFlags, similar to the
//               result of BoundingVolume::contains().
//
//               Also, if the bounding volume is completely in front
//               of any of the clip planes, removes those planes both
//               from this object and from the indicated state,
//               returning a new CullPlanes object in that case.
////////////////////////////////////////////////////////////////////
CPT(CullPlanes) CullPlanes::
do_cull(int &result, CPT(RenderState) &state,
        const GeometricBoundingVolume *node_gbv) const {
  result = 
    BoundingVolume::IF_all | BoundingVolume::IF_possible | BoundingVolume::IF_some;

  CPT(ClipPlaneAttrib) orig_cpa = DCAST(ClipPlaneAttrib, state->get_attrib(ClipPlaneAttrib::get_class_slot()));

  CPT(CullPlanes) new_planes = this;
  
  if (orig_cpa == (ClipPlaneAttrib *)NULL) {
    // If there are no clip planes in the state, the node is completely
    // in front of all zero of the clip planes.  (This can happen if
    // someone directly changes the state during the traversal.)
    CullPlanes *planes = new CullPlanes;
    planes->_occluders = _occluders;
    new_planes = planes;

  } else {
    CPT(ClipPlaneAttrib) new_cpa = orig_cpa;

    Planes::const_iterator pi;
    for (pi = _planes.begin(); pi != _planes.end(); ++pi) {
      int plane_result = (*pi).second->contains(node_gbv);
      if (plane_result == BoundingVolume::IF_no_intersection) {
        // The node is completely behind this clip plane and gets
        // culled.  Short-circuit the rest of the logic; none of the
        // other planes matter.
        result = plane_result;
        return new_planes;
      } else if ((plane_result & BoundingVolume::IF_all) != 0) {
        // The node is completely in front of this clip plane.  We don't
        // need to consider this plane ever again for any descendents of
        // this node.
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
      // The node is completely in front of this occluder.  We don't
      // need to consider this occluder ever again for any descendents of
      // this node.
      
      // Reverse the sense of the test, because an occluder volume is
      // the inverse of a cull plane volume: it describes the volume
      // that is to be culled, not the volume that is to be kept.
      occluder_result = BoundingVolume::IF_all | BoundingVolume::IF_possible | BoundingVolume::IF_some;
      new_planes = new_planes->remove_occluder((*oi).first);
      nassertr(new_planes != this, new_planes);

    } else if ((occluder_result & BoundingVolume::IF_all) != 0) {
      // The node is completely behind this occluder and gets culled.
      // Short-circuit the rest of the logic; none of the other
      // occluders matter.
      result = BoundingVolume::IF_no_intersection;
      return new_planes;
    }

    result &= occluder_result;
  }
    
  return new_planes;
}

////////////////////////////////////////////////////////////////////
//     Function: CullPlanes::remove_plane
//       Access: Public
//  Description: Returns a pointer to a new CullPlanes object that is
//               the same as this one, but with the indicated
//               clip plane removed.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CullPlanes::remove_occluder
//       Access: Public
//  Description: Returns a pointer to a new CullPlanes object that is
//               the same as this one, but with the indicated
//               occluder removed.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CullPlanes::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void CullPlanes::
write(ostream &out) const {
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
