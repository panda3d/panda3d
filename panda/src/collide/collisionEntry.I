// Filename: collisionEntry.I
// Created by:  drose (16Mar02)
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


////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionEntry::
CollisionEntry() {
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_from
//       Access: Published
//  Description: Returns the CollisionSolid pointer for the particular
//               solid that triggered this collision.
////////////////////////////////////////////////////////////////////
INLINE const CollisionSolid *CollisionEntry::
get_from() const {
  return _from;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_into
//       Access: Published
//  Description: Returns true if the "into" solid is, in fact, a
//               CollisionSolid, and its pointer is known (in which
//               case get_into() may be called to retrieve it).  If
//               this returns false, the collision was detected into a
//               GeomNode, and there is no CollisionSolid pointer to
//               be retrieved.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionEntry::
has_into() const {
  return (_into != (CollisionSolid *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_into
//       Access: Published
//  Description: Returns the CollisionSolid pointer for the particular
//               solid was collided into.  This pointer might be NULL
//               if the collision was into a piece of visible
//               geometry, instead of a normal CollisionSolid
//               collision; see has_into().
////////////////////////////////////////////////////////////////////
INLINE const CollisionSolid *CollisionEntry::
get_into() const {
  return _into;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_from_node
//       Access: Published
//  Description: Returns the node that contains the CollisionSolid
//               that triggered this collision.  This will be a node
//               that has been added to a CollisionTraverser via
//               add_collider().
////////////////////////////////////////////////////////////////////
INLINE CollisionNode *CollisionEntry::
get_from_node() const {
  return _from_node;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_into_node
//       Access: Published
//  Description: Returns the node that contains the CollisionSolid
//               that was collided into.  This returns a PandaNode
//               pointer instead of something more specific, because
//               it might be either a CollisionNode or a GeomNode.
//
//               Also see get_into_node_path().
////////////////////////////////////////////////////////////////////
INLINE PandaNode *CollisionEntry::
get_into_node() const {
  return _into_node;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_from_node_path
//       Access: Published
//  Description: Returns the NodePath that represents the
//               CollisionNode that contains the CollisionSolid that
//               triggered this collision.  This will be a NodePath
//               that has been added to a CollisionTraverser via
//               add_collider().
////////////////////////////////////////////////////////////////////
INLINE const NodePath &CollisionEntry::
get_from_node_path() const {
  return _from_node_path;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_into_node_path
//       Access: Published
//  Description: Returns the NodePath that represents the specific
//               CollisionNode or GeomNode instance that was collided
//               into.  This is the same node returned by
//               get_into_node(), represented as a NodePath; however,
//               it may be more useful because the NodePath can
//               resolve the particular instance of the node, if there
//               is more than one.
////////////////////////////////////////////////////////////////////
INLINE const NodePath &CollisionEntry::
get_into_node_path() const {
  return _into_node_path;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_respect_prev_transform
//       Access: Published
//  Description: Returns true if the collision was detected by a
//               CollisionTraverser whose respect_prev_transform
//               flag was set true, meaning we should consider motion
//               significant in evaluating collisions.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionEntry::
get_respect_prev_transform() const {
  return (_flags & F_respect_prev_transform) != 0;
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::set_surface_point
//       Access: Published
//  Description: Stores the point, on the surface of the "into"
//               object, at which a collision is detected.
//
//               This point is specified in the coordinate space of
//               the "into" object.
////////////////////////////////////////////////////////////////////
INLINE void CollisionEntry::
set_surface_point(const LPoint3f &point) {
  _surface_point = point;
  _flags |= F_has_surface_point;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::set_surface_normal
//       Access: Published
//  Description: Stores the surface normal of the "into" object at the
//               point of the intersection.
//
//               This normal is specified in the coordinate space of
//               the "into" object.
////////////////////////////////////////////////////////////////////
INLINE void CollisionEntry::
set_surface_normal(const LVector3f &normal) {
  _surface_normal = normal;
  _flags |= F_has_surface_normal;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::set_interior_point
//       Access: Published
//  Description: Stores the point, within the interior of the "into"
//               object, which represents the depth to which the
//               "from" object has penetrated.  This can also be
//               described as the intersection point on the surface of
//               the "from" object (which is inside the "into"
//               object).
//
//               This point is specified in the coordinate space of
//               the "into" object.
////////////////////////////////////////////////////////////////////
INLINE void CollisionEntry::
set_interior_point(const LPoint3f &point) {
  _interior_point = point;
  _flags |= F_has_interior_point;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_surface_point
//       Access: Published
//  Description: Returns true if the surface point has been specified,
//               false otherwise.  See get_surface_point().  Some
//               types of collisions may not compute the surface
//               point.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionEntry::
has_surface_point() const {
  return (_flags & F_has_surface_point) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_surface_normal
//       Access: Published
//  Description: Returns true if the surface normal has been specified,
//               false otherwise.  See get_surface_normal().  Some
//               types of collisions may not compute the surface
//               normal.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionEntry::
has_surface_normal() const {
  return (_flags & F_has_surface_normal) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_interior_point
//       Access: Published
//  Description: Returns true if the interior point has been specified,
//               false otherwise.  See get_interior_point().  Some
//               types of collisions may not compute the interior
//               point.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionEntry::
has_interior_point() const {
  return (_flags & F_has_interior_point) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_into_intersection_point
//       Access: Published
//  Description: Returns true if the detected collision knows its
//               intersection point in the coordinate space of the
//               collided-into object, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionEntry::
has_into_intersection_point() const {
  return has_surface_point();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_into_intersection_point
//       Access: Published
//  Description: Returns the intersection point in the coordinate
//               space of the collided-into object.  It is an error to
//               call this if has_into_intersection_point() returns
//               false.
////////////////////////////////////////////////////////////////////
INLINE LPoint3f CollisionEntry::
get_into_intersection_point() const {
  return get_surface_point(get_into_node_path());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_from_intersection_point
//       Access: Published
//  Description: Returns true if the detected collision knows its
//               intersection point in the coordinate space of the
//               colliding object, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionEntry::
has_from_intersection_point() const {
  return has_surface_point();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_from_intersection_point
//       Access: Published
//  Description: Returns the intersection point in the coordinate
//               space of the colliding object.  It is an error to
//               call this if has_from_intersection_point() returns
//               false.
////////////////////////////////////////////////////////////////////
INLINE LPoint3f CollisionEntry::
get_from_intersection_point() const {
  return get_surface_point(get_from_node_path());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_into_surface_normal
//       Access: Published
//  Description: Returns true if the detected collision knows the
//               surface normal of the collided-into object at the
//               point of the collision, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionEntry::
has_into_surface_normal() const {
  return has_surface_normal();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_into_surface_normal
//       Access: Published
//  Description: Returns the surface normal of the collided-into
//               object at the point of the collision.  It is an error
//               to call this if has_into_surface_normal() returns
//               false.
////////////////////////////////////////////////////////////////////
INLINE LVector3f CollisionEntry::
get_into_surface_normal() const {
  return get_surface_normal(get_into_node_path());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_from_surface_normal
//       Access: Published
//  Description: Returns true if the detected collision knows the
//               surface normal of the collided-into object at the
//               point of the collision, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionEntry::
has_from_surface_normal() const {
  return has_surface_normal();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_from_surface_normal
//       Access: Published
//  Description: Returns the surface normal of the collided-into
//               object at the point of the collision, in the space of
//               the collided-from object.  It is an error to call
//               this if has_from_surface_normal() returns false.
////////////////////////////////////////////////////////////////////
INLINE LVector3f CollisionEntry::
get_from_surface_normal() const {
  return get_surface_normal(get_from_node_path());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_into_depth
//       Access: Published
//  Description: Returns true if the collision entry knows how "deep"
//               the collision was into the collided-into object; that
//               is, how far into the surface of the collided-into
//               object the colliding object has penetrated.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionEntry::
has_into_depth() const {
  return has_surface_point() && has_interior_point();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_into_depth
//       Access: Published
//  Description: Returns how "deep" the collision was into the
//               collided-into object; that is, how far into the
//               surface of the collided-into object the colliding
//               object has penetrated.  It is an error to call this
//               if has_into_depth() returns false.
////////////////////////////////////////////////////////////////////
INLINE float CollisionEntry::
get_into_depth() const {
  nassertr(has_into_depth(), 0.0);
  return (get_surface_point(get_into_node_path()) - get_interior_point(get_into_node_path())).length();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_from_depth
//       Access: Published
//  Description: Returns true if the collision entry knows how "deep"
//               the collision was from the collided-from object; that
//               is, how far from the surface of the collided-from
//               object the colliding object has penetrated.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionEntry::
has_from_depth() const {
  return has_surface_point() && has_interior_point();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_from_depth
//       Access: Published
//  Description: Returns how "deep" the collision was from the
//               collided-from object; that is, how far from the
//               surface of the collided-from object the colliding
//               object has penetrated.  It is an error to call this
//               if has_from_depth() returns false.
////////////////////////////////////////////////////////////////////
INLINE float CollisionEntry::
get_from_depth() const {
  return (get_surface_point(get_from_node_path()) - get_interior_point(get_from_node_path())).length();
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_wrt_space
//       Access: Public
//  Description: Returns the relative transform of the from node as
//               seen from the into node.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) CollisionEntry::
get_wrt_space() const {
  return _from_node_path.get_transform(_into_node_path);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_inv_wrt_space
//       Access: Public
//  Description: Returns the relative transform of the into node as
//               seen from the from node.
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) CollisionEntry::
get_inv_wrt_space() const {
  return _into_node_path.get_transform(_from_node_path);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_wrt_prev_space
//       Access: Public
//  Description: Returns the relative transform of the from node as
//               seen from the into node, as of the previous frame
//               (according to set_prev_transform(), set_fluid_pos(),
//               etc.)
////////////////////////////////////////////////////////////////////
INLINE CPT(TransformState) CollisionEntry::
get_wrt_prev_space() const {
  if (get_respect_prev_transform()) {
    return _from_node_path.get_prev_transform(_into_node_path);
  } else {
    return get_wrt_space();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_wrt_mat
//       Access: Public
//  Description: Returns the relative transform of the from node as
//               seen from the into node.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &CollisionEntry::
get_wrt_mat() const {
  return get_wrt_space()->get_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_inv_wrt_mat
//       Access: Public
//  Description: Returns the relative transform of the into node as
//               seen from the from node.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &CollisionEntry::
get_inv_wrt_mat() const {
  return get_inv_wrt_space()->get_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_wrt_prev_mat
//       Access: Public
//  Description: Returns the relative transform of the from node as
//               seen from the into node, as of the previous frame
//               (according to set_prev_transform(), set_fluid_pos(),
//               etc.)
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &CollisionEntry::
get_wrt_prev_mat() const {
  return get_wrt_prev_space()->get_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_into_clip_planes
//       Access: Public
//  Description: Returns the ClipPlaneAttrib, if any, that is applied
//               to the into_node_path, or NULL if there is no clip
//               plane in effect.
////////////////////////////////////////////////////////////////////
INLINE const ClipPlaneAttrib *CollisionEntry::
get_into_clip_planes() const {
  if ((_flags & F_checked_clip_planes) == 0) {
    ((CollisionEntry *)this)->check_clip_planes();
  }
  return _into_clip_planes;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::test_intersection
//       Access: Private
//  Description: This is intended to be called only by the
//               CollisionTraverser.  It requests the CollisionEntry
//               to start the intersection test between the from and
//               into solids stored within it, passing the result (if
//               positive) to the indicated CollisionHandler.
////////////////////////////////////////////////////////////////////
INLINE void CollisionEntry::
test_intersection(CollisionHandler *record, 
                  const CollisionTraverser *trav) const {
  PT(CollisionEntry) result = get_from()->test_intersection(*this);
#ifdef DO_COLLISION_RECORDING
  if (trav->has_recorder()) {
    if (result != (CollisionEntry *)NULL) {
      trav->get_recorder()->collision_tested(*result, true);
    } else {
      trav->get_recorder()->collision_tested(*this, false);
    }
  }
#endif  // DO_COLLISION_RECORDING
  if (result != (CollisionEntry *)NULL) {
    record->add_entry(result);
  }
}

INLINE ostream &
operator << (ostream &out, const CollisionEntry &entry) {
  entry.output(out);
  return out;
}
