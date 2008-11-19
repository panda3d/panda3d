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
            const ClipPlaneAttrib *off_attrib) const {
  if (net_attrib == (ClipPlaneAttrib *)NULL) {
    return this;
  }

  PT(CullPlanes) new_planes;
  if (get_ref_count() == 1) {
    new_planes = (CullPlanes *)this;
  } else {
    new_planes = new CullPlanes(*this);
  }

  CPT(TransformState) net_transform = NULL;

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
  
  // If there are no clip planes in the state, the node is completely
  // in front of all zero of the clip planes.  (This can happen if
  // someone directly changes the state during the traversal.)
  if (orig_cpa == (ClipPlaneAttrib *)NULL) {
    return new CullPlanes;
  }

  CPT(CullPlanes) new_planes = this;
  CPT(ClipPlaneAttrib) new_cpa = orig_cpa;

  Planes::const_iterator pi;
  for (pi = _planes.begin(); pi != _planes.end(); ++pi) {
    int plane_result = (*pi).second->contains(node_gbv);
    if (plane_result == BoundingVolume::IF_no_intersection) {
      // The node is completely behind this clip plane.  Short-circuit
      // the rest of the logic; none of the other planes matter.
      result = plane_result;
      return new_planes;
    }

    if ((plane_result & BoundingVolume::IF_all) != 0) {
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
//     Function: CullPlanes::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void CullPlanes::
write(ostream &out) const {
  out << "CullPlanes (" << _planes.size() << " planes):\n";
  Planes::const_iterator pi;
  for (pi = _planes.begin(); pi != _planes.end(); ++pi) {
    out << "  " << (*pi).first << " : " << *(*pi).second << "\n";
  }
}
