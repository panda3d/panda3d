// Filename: collisionLevelState.I
// Created by:  drose (24Apr00)
// 
////////////////////////////////////////////////////////////////////

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
INLINE const LMatrix4f &CollisionLevelState::
get_space(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), LMatrix4f::ident_mat());
  nassertr(has_collider(n), LMatrix4f::ident_mat());

  return _colliders[n]._space;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::get_inv_space
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE const LMatrix4f &CollisionLevelState::
get_inv_space(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), LMatrix4f::ident_mat());
  nassertr(has_collider(n), LMatrix4f::ident_mat());

  return _colliders[n]._inv_space;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::get_local_bound
//       Access: Public
//  Description: 
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
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CollisionLevelState::ColliderMask CollisionLevelState::
get_mask(int n) const {
  return ((ColliderMask)1) << n;
}
