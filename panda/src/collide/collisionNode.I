// Filename: collisionNode.I
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
//     Function: CollisionNode::set_collide_mask
//       Access: Published
//  Description: Simultaneously sets both the "from" and "into"
//               CollideMask values to the same thing.
////////////////////////////////////////////////////////////////////
INLINE void CollisionNode::
set_collide_mask(CollideMask mask) {
  set_from_collide_mask(mask);
  set_into_collide_mask(mask);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::set_from_collide_mask
//       Access: Published
//  Description: Sets the "from" CollideMask.  In order for a
//               collision to be detected from this object into
//               another object, the intersection of this object's
//               "from" mask and the other object's "into" mask must
//               be nonzero.
////////////////////////////////////////////////////////////////////
INLINE void CollisionNode::
set_from_collide_mask(CollideMask mask) {
  _from_collide_mask = mask;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::set_into_collide_mask
//       Access: Published
//  Description: Sets the "into" CollideMask.  In order for a
//               collision to be detected from another object into
//               this object, the intersection of the other object's
//               "from" mask and this object's "into" mask must be
//               nonzero.
////////////////////////////////////////////////////////////////////
INLINE void CollisionNode::
set_into_collide_mask(CollideMask mask) {
  _into_collide_mask = mask;

  // We mark the bound stale when this changes, not because the actual
  // bounding volume changes, but rather because we piggyback the
  // computing of the _net_collide_mask on the bounding volume.
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::get_from_collide_mask
//       Access: Published
//  Description: Returns the current "from" CollideMask.  In order for
//               a collision to be detected from this object into
//               another object, the intersection of this object's
//               "from" mask and the other object's "into" mask must
//               be nonzero.
////////////////////////////////////////////////////////////////////
INLINE CollideMask CollisionNode::
get_from_collide_mask() const {
  return _from_collide_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::get_into_collide_mask
//       Access: Published
//  Description: Returns the current "into" CollideMask.  In order for
//               a collision to be detected from another object into
//               this object, the intersection of the other object's
//               "from" mask and this object's "into" mask must be
//               nonzero.
////////////////////////////////////////////////////////////////////
INLINE CollideMask CollisionNode::
get_into_collide_mask() const {
  return _into_collide_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::set_collide_geom
//       Access: Published
//  Description: Sets the state of the "collide geom" flag for this
//               CollisionNode.  Normally, this is false; when this is
//               set true, the CollisionSolids in this node will test
//               for collisions with actual renderable geometry, in
//               addition to whatever CollisionSolids may be indicated
//               by the from_collide_mask.
//
//               Setting this to true causes this to test *all*
//               GeomNodes for collisions.  It is an all-or-none
//               thing; there is no way to collide with only some
//               GeomNodes, as GeomNodes have no into_collide_mask.
////////////////////////////////////////////////////////////////////
INLINE void CollisionNode::
set_collide_geom(bool flag) {
  if (flag) {
    _flags |= F_collide_geom;
  } else {
    _flags &= ~F_collide_geom;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::get_collide_geom
//       Access: Published
//  Description: Returns the current state of the collide_geom flag.
//               See set_collide_geom().
////////////////////////////////////////////////////////////////////
INLINE bool CollisionNode::
get_collide_geom() const {
  return (_flags & F_collide_geom) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::clear_solids
//       Access: Published
//  Description: Removes all solids from the node.
////////////////////////////////////////////////////////////////////
INLINE void CollisionNode::
clear_solids() {
  _solids.clear();
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::get_num_solids
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int CollisionNode::
get_num_solids() const {
  return _solids.size();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::get_solid
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionSolid *CollisionNode::
get_solid(int n) const {
  nassertr(n >= 0 && n < get_num_solids(), NULL);
  return _solids[n];
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::set_solid
//       Access: Published
//  Description: Replaces the solid with the indicated index.
////////////////////////////////////////////////////////////////////
INLINE void CollisionNode::
set_solid(int n, CollisionSolid *solid) {
  nassertv(n >= 0 && n < get_num_solids());
  _solids[n] = solid;
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::remove_solid
//       Access: Published
//  Description: Removes the solid with the indicated index.  This
//               will shift all subsequent indices down by one.
////////////////////////////////////////////////////////////////////
INLINE void CollisionNode::
remove_solid(int n) {
  nassertv(n >= 0 && n < get_num_solids());
  _solids.erase(_solids.begin() + n);
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionNode::add_solid
//       Access: Published
//  Description: Adds the indicated solid to the node.  Returns the
//               index of the new solid within the node's list of
//               solids.
////////////////////////////////////////////////////////////////////
INLINE int CollisionNode::
add_solid(CollisionSolid *solid) {
  _solids.push_back(solid);
  mark_bound_stale();
  return _solids.size() - 1;
}
