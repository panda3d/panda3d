// Filename: collisionEntry.I
// Created by:  drose (24Apr00)
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
//               that was collided into.  This returns a NamedNode
//               pointer instead of something more specific, because
//               it might be either a CollisionNode or a GeomNode.
////////////////////////////////////////////////////////////////////
INLINE NamedNode *CollisionEntry::
get_into_node() const {
  return _into_node;
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
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool CollisionEntry::
has_into_intersection_point() const {
  return (_flags & F_has_into_intersection_point) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_into_intersection_point
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &CollisionEntry::
get_into_intersection_point() const {
  nassertr(has_into_intersection_point(), _into_intersection_point);
  return _into_intersection_point;
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
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool CollisionEntry::
has_into_surface_normal() const {
  return (_flags & F_has_into_surface_normal) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_into_surface_normal
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE const LVector3f &CollisionEntry::
get_into_surface_normal() const {
  nassertr(has_into_surface_normal(), _into_surface_normal);
  return _into_surface_normal;
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
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool CollisionEntry::
has_into_depth() const {
  return (_flags & F_has_into_depth) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionEntry::get_into_depth
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE float CollisionEntry::
get_into_depth() const {
  nassertr(has_into_depth(), 0.0);
  return _into_depth;
}
