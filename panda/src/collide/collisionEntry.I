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
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionEntry::
CollisionEntry() {
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_from
//       Access: Public
//  Description: Returns the CollisionSolid pointer for the particular
//               solid that triggered this collision.
////////////////////////////////////////////////////////////////////
INLINE const CollisionSolid *CollisionEntry::
get_from() const {
  return _from;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_into
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//     Function: CollisionEntry::get_from_space
//       Access: Public
//  Description: Returns the global coordinate space of the
//               CollisionNode returned by get_from_node(), as of the
//               time of the collision.  This will be equivalent to a
//               wrt() from the node to render.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &CollisionEntry::
get_from_space() const {
  return _from_space;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_into_node
//       Access: Public
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
//     Function: CollisionEntry::get_into_node_path
//       Access: Public
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
//     Function: CollisionEntry::get_into_space
//       Access: Public
//  Description: Returns the global coordinate space of the
//               CollisionNode or GeomNode returned by
//               get_into_node(), as of the time of the collision.
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &CollisionEntry::
get_into_space() const {
  return _into_space;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_wrt_space
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &CollisionEntry::
get_wrt_space() const {
  return _wrt_space;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_inv_wrt_space
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &CollisionEntry::
get_inv_wrt_space() const {
  return _inv_wrt_space;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::set_from_velocity
//       Access: Public
//  Description: Indicates the velocity associated with the "from"
//               object, in the object's coordinate space.
////////////////////////////////////////////////////////////////////
INLINE void CollisionEntry::
set_from_velocity(const LVector3f &vel) {
  _from_velocity = vel;
  _flags |= F_has_from_velocity;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_from_velocity
//       Access: Public
//  Description: Returns true if the "from" object had an indicated
//               velocity, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionEntry::
has_from_velocity() const {
  return (_flags & F_has_from_velocity) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_from_velocity
//       Access: Public
//  Description: Returns the instantaneous velocity of the "from"
//               object, in its own coordinate space.  This represents
//               the delta between its current position and its
//               position last frame.
////////////////////////////////////////////////////////////////////
INLINE const LVector3f &CollisionEntry::
get_from_velocity() const {
  nassertr(has_from_velocity(), _from_velocity);
  return _from_velocity;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::set_into_intersection_point
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionEntry::
set_into_intersection_point(const LPoint3f &point) {
  _into_intersection_point = point;
  _flags |= F_has_into_intersection_point;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_into_intersection_point
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
//  Description: Returns the intersection point in the coordinate
//               space of the colliding object.  It is an error to
//               call this if has_from_intersection_point() returns
//               false.
////////////////////////////////////////////////////////////////////
INLINE LPoint3f CollisionEntry::
get_from_intersection_point() const {
  return get_into_intersection_point() * get_inv_wrt_space();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::set_into_surface_normal
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionEntry::
set_into_surface_normal(const LVector3f &normal) {
  _into_surface_normal = normal;
  _flags |= F_has_into_surface_normal;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_into_surface_normal
//       Access: Public
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
//       Access: Public
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
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionEntry::
set_from_surface_normal(const LVector3f &normal) {
  _from_surface_normal = normal;
  _flags |= F_has_from_surface_normal;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_from_surface_normal
//       Access: Public
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
//       Access: Public
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
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionEntry::
set_into_depth(float depth) {
  _into_depth = depth;
  _flags |= F_has_into_depth;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_into_depth
//       Access: Public
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
//       Access: Public
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
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionEntry::
set_from_depth(float depth) {
  _from_depth = depth;
  _flags |= F_has_from_depth;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::has_from_depth
//       Access: Public
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
//       Access: Public
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
