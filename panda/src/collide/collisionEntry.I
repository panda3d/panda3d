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
  _from_space = TransformState::make_identity();
  _into_space = TransformState::make_identity();
  _wrt_space = TransformState::make_identity();
  _inv_wrt_space = TransformState::make_identity();
  _wrt_prev_space = TransformState::make_identity();
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
//     Function: CollisionEntry::get_from_space
//       Access: Published
//  Description: Returns the global coordinate space of the
//               CollisionNode returned by get_from_node(), as of the
//               time of the collision.  This will be equivalent to a
//               wrt() from the node to render.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *CollisionEntry::
get_from_space() const {
  return _from_space;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_into_space
//       Access: Published
//  Description: Returns the global coordinate space of the
//               CollisionNode or GeomNode returned by
//               get_into_node(), as of the time of the collision.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *CollisionEntry::
get_into_space() const {
  return _into_space;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_wrt_space
//       Access: Published
//  Description: Returns the relative transform of the from node as
//               seen from the into node.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *CollisionEntry::
get_wrt_space() const {
  return _wrt_space;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_inv_wrt_space
//       Access: Published
//  Description: Returns the relative transform of the into node as
//               seen from the from node.
////////////////////////////////////////////////////////////////////
INLINE const TransformState *CollisionEntry::
get_inv_wrt_space() const {
  return _inv_wrt_space;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_wrt_prev_space
//       Access: Published
//  Description: Returns the relative transform of the from node as
//               seen from the into node, as of the previous frame
//               (according to set_prev_transform(), set_fluid_pos(),
//               etc.)
////////////////////////////////////////////////////////////////////
INLINE const TransformState *CollisionEntry::
get_wrt_prev_space() const {
  return _wrt_prev_space;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_from_mat
//       Access: Published
//  Description: Returns the global coordinate space of the
//               CollisionNode returned by get_from_node(), as of the
//               time of the collision.  This will be equivalent to a
//               wrt() from the node to render.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &CollisionEntry::
get_from_mat() const {
  return _from_space->get_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_into_mat
//       Access: Published
//  Description: Returns the global coordinate space of the
//               CollisionNode or GeomNode returned by
//               get_into_node(), as of the time of the collision.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &CollisionEntry::
get_into_mat() const {
  return _into_space->get_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_wrt_mat
//       Access: Published
//  Description: Returns the relative transform of the from node as
//               seen from the into node.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &CollisionEntry::
get_wrt_mat() const {
  return _wrt_space->get_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_inv_wrt_mat
//       Access: Published
//  Description: Returns the relative transform of the into node as
//               seen from the from node.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &CollisionEntry::
get_inv_wrt_mat() const {
  return _inv_wrt_space->get_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_wrt_prev_mat
//       Access: Published
//  Description: Returns the relative transform of the from node as
//               seen from the into node, as of the previous frame
//               (according to set_prev_transform(), set_fluid_pos(),
//               etc.)
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &CollisionEntry::
get_wrt_prev_mat() const {
  return _wrt_prev_space->get_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::set_into_intersection_point
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionEntry::
set_into_intersection_point(const LPoint3f &point) {
  _into_intersection_point = point;
  _flags |= F_has_into_intersection_point;
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
  return (_flags & F_has_into_intersection_point) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_into_intersection_point
//       Access: Published
//  Description: Returns the intersection point in the coordinate
//               space of the collided-into object.  It is an error to
//               call this if has_into_intersection_point() returns
//               false.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &CollisionEntry::
get_into_intersection_point() const {
  nassertr(has_into_intersection_point(), _into_intersection_point);
  return _into_intersection_point;
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
  // Since we derive the from_intersection_point from the
  // into_intersection_point, this is really the same question.
  return has_into_intersection_point();
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
  return get_into_intersection_point() * get_inv_wrt_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::set_into_surface_normal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionEntry::
set_into_surface_normal(const LVector3f &normal) {
  _into_surface_normal = normal;
  _flags |= F_has_into_surface_normal;
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
  return (_flags & F_has_into_surface_normal) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_into_surface_normal
//       Access: Published
//  Description: Returns the surface normal of the collided-into
//               object at the point of the collision.  It is an error
//               to call this if has_into_surface_normal() returns
//               false.
////////////////////////////////////////////////////////////////////
INLINE const LVector3f &CollisionEntry::
get_into_surface_normal() const {
  nassertr(has_into_surface_normal(), _into_surface_normal);
  return _into_surface_normal;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::set_from_surface_normal
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionEntry::
set_from_surface_normal(const LVector3f &normal) {
  _from_surface_normal = normal;
  _flags |= F_has_from_surface_normal;
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
  return (_flags & (F_has_into_surface_normal | F_has_from_surface_normal)) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_from_surface_normal
//       Access: Published
//  Description: Returns the surface normal of the collided-into
//               object at the point of the collision, in the space of
//               the collided-from object.  It is an error to call
//               this if has_from_surface_normal() returns false.
////////////////////////////////////////////////////////////////////
INLINE const LVector3f &CollisionEntry::
get_from_surface_normal() const {
  nassertr(has_from_surface_normal(), _from_surface_normal);
  if ((_flags & F_has_from_surface_normal) == 0) {
    ((CollisionEntry *)this)->compute_from_surface_normal();
  }
  return _from_surface_normal;
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::set_into_depth
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionEntry::
set_into_depth(float depth) {
  _into_depth = depth;
  _flags |= F_has_into_depth;
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
  return (_flags & F_has_into_depth) != 0;
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
  return _into_depth;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::set_from_depth
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionEntry::
set_from_depth(float depth) {
  _from_depth = depth;
  _flags |= F_has_from_depth;
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
  return (_flags & F_has_from_depth) != 0;
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
  nassertr(has_from_depth(), 0.0);
  return _from_depth;
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
