// Filename: collisionLevelState.I
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
//     Function: CollisionLevelState::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionLevelState::
CollisionLevelState(const NodePath &node_path) :
  _node_path(node_path)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::Constructor
//       Access: Public
//  Description: This constructor goes to the next child node in the
//               traversal.
////////////////////////////////////////////////////////////////////
INLINE CollisionLevelState::
CollisionLevelState(const CollisionLevelState &parent, PandaNode *child) :
  _node_path(parent._node_path, child),
  _colliders(parent._colliders),
  _current(parent._current),
  _colliders_with_geom(parent._colliders_with_geom),
  _local_bounds(parent._local_bounds)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::get_node_path
//       Access: Public
//  Description: Returns the NodePath representing the node instance
//               we have traversed to.
////////////////////////////////////////////////////////////////////
INLINE NodePath CollisionLevelState::
get_node_path() const {
  return _node_path.get_node_path();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::node
//       Access: Public
//  Description: Returns the PandaNode pointer of the node we have
//               traversed to.
////////////////////////////////////////////////////////////////////
INLINE PandaNode *CollisionLevelState::
node() const {
  return _node_path.node();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::get_num_colliders
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int CollisionLevelState::
get_num_colliders() const {
  return _colliders.size();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::has_collider
//       Access: Public
//  Description: Returns true if the nth collider in the LevelState is
//               still part of the level.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionLevelState::
has_collider(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), false);
  return (_current & get_mask(n)) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::has_collider_with_geom
//       Access: Public
//  Description: Returns true if the nth collider in the LevelState is
//               still part of the level, and it has the
//               "collide_geom" flag set.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionLevelState::
has_collider_with_geom(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), false);
  return (_current & _colliders_with_geom & get_mask(n)) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::has_any_collider
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool CollisionLevelState::
has_any_collider() const {
  return _current != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::has_any_collide_geom
//       Access: Public
//  Description: Returns true if any Collider in the level state has
//               the "collide_geom" flag set, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionLevelState::
has_any_collide_geom() const {
  return (_current & _colliders_with_geom) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::reached_collision_node
//       Access: Public
//  Description: Called by the traverser when we reach a CollisionNode
//               in the traversal.  At this point, we zero out our set
//               of colliders with the "collide_geom" flag set,
//               because no colliders will test against geometry
//               parented beneath a CollisionNode.
////////////////////////////////////////////////////////////////////
INLINE void CollisionLevelState::
reached_collision_node() {
  _colliders_with_geom = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::get_collider
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionSolid *CollisionLevelState::
get_collider(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), NULL);
  nassertr(has_collider(n), NULL);

  return _colliders[n]._collider;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::get_node
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionNode *CollisionLevelState::
get_node(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), NULL);
  nassertr(has_collider(n), NULL);

  return _colliders[n]._node;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::get_space
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const TransformState *CollisionLevelState::
get_space(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), TransformState::make_identity());
  nassertr(has_collider(n), TransformState::make_identity());

  return _colliders[n]._space;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::get_inv_space
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const TransformState *CollisionLevelState::
get_inv_space(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), TransformState::make_identity());
  nassertr(has_collider(n), TransformState::make_identity());

  return _colliders[n]._inv_space;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::get_prev_space
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const TransformState *CollisionLevelState::
get_prev_space(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), TransformState::make_identity());
  nassertr(has_collider(n), TransformState::make_identity());

  return _colliders[n]._prev_space;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::get_delta
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LVector3f &CollisionLevelState::
get_delta(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), LVector3f::zero());
  nassertr(has_collider(n), LVector3f::zero());

  return _colliders[n]._delta;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::get_local_bound
//       Access: Public
//  Description: Returns the bounding volume of the indicated
//               collider, transformed into the current node's
//               transform space.
////////////////////////////////////////////////////////////////////
INLINE const GeometricBoundingVolume *CollisionLevelState::
get_local_bound(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), NULL);
  nassertr(has_collider(n), NULL);
  nassertr(n >= 0 && n < (int)_local_bounds.size(), NULL);

  // For whatever reason, the Intel compiler can't figure this line
  // out.
  //return _local_bounds[n];

  // But it can figure out this equivalent line.
  return *(_local_bounds + n);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::get_parent_bound
//       Access: Public
//  Description: Returns the bounding volume of the indicated
//               collider, transformed into the previous node's
//               transform space, but not transformed by the current
//               node's transform.  This is appropriate for testing
//               against the bounding volume of the current node
//               (which does not have its own transform applied to
//               it).
////////////////////////////////////////////////////////////////////
INLINE const GeometricBoundingVolume *CollisionLevelState::
get_parent_bound(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), NULL);
  nassertr(has_collider(n), NULL);
  nassertr(n >= 0 && n < (int)_parent_bounds.size(), NULL);

  // But it can figure out this equivalent line.
  return *(_parent_bounds + n);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::omit_collider
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionLevelState::
omit_collider(int n) {
  nassertv(n >= 0 && n < (int)_colliders.size());
  nassertv(has_collider(n));

  _current &= ~get_mask(n);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::get_mask
//       Access: Private
//  Description: Returns a single bit associated with the nth
//               collider.
////////////////////////////////////////////////////////////////////
INLINE CollisionLevelState::ColliderMask CollisionLevelState::
get_mask(int n) const {
  return ((ColliderMask)1) << n;
}
