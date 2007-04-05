// Filename: collisionLevelState.I
// Created by:  drose (16Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionLevelState::
CollisionLevelState(const NodePath &node_path) :
  _node_path(node_path),
  _current(CollisionLevelState::CurrentMask::all_off()),
  _include_mask(CollideMask::all_on())
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
  _include_mask(parent._include_mask),
  _local_bounds(parent._local_bounds)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CollisionLevelState::
CollisionLevelState(const CollisionLevelState &copy) :
  _node_path(copy._node_path),
  _colliders(copy._colliders),
  _current(copy._current),
  _include_mask(copy._include_mask),
  _local_bounds(copy._local_bounds),
  _parent_bounds(copy._parent_bounds)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CollisionLevelState::
operator = (const CollisionLevelState &copy) {
  _node_path = copy._node_path;
  _colliders = copy._colliders;
  _current = copy._current;
  _include_mask = copy._include_mask;
  _local_bounds = copy._local_bounds;
  _parent_bounds = copy._parent_bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::get_max_colliders
//       Access: Public, Static
//  Description: Returns the maximum number of colliders that may be
//               added to the CollisionLevelState at any one time.
////////////////////////////////////////////////////////////////////
INLINE int CollisionLevelState::
get_max_colliders() {
  return CurrentMask::get_max_num_bits();
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
  return (_current.get_bit(n));
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::has_any_collider
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool CollisionLevelState::
has_any_collider() const {
  return !_current.is_zero();
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
//     Function: CollisionLevelState::get_collider_node
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionNode *CollisionLevelState::
get_collider_node(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), NULL);
  nassertr(has_collider(n), NULL);

  return _colliders[n]._node;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::get_collider_node_path
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE NodePath CollisionLevelState::
get_collider_node_path(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), NodePath::fail());
  nassertr(has_collider(n), NodePath::fail());

  return _colliders[n]._node_path;
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

  _current.clear_bit(n);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::set_include_mask
//       Access: Public
//  Description: Specifies the mask that is applied to the into
//               CollideMask of nodes in the scene graph before
//               testing for bits in common with the from CollideMask
//               of colliders.  This is normally all bits on, but you
//               may set it to some other mask to restrict certain
//               bits from consideration.
//
//               This is used by the CollisionTraverser to restrict
//               collision with geometry except under the lowest level
//               of LOD.
////////////////////////////////////////////////////////////////////
INLINE void CollisionLevelState::
set_include_mask(CollideMask include_mask) {
  _include_mask = include_mask;
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionLevelState::get_include_mask
//       Access: Public
//  Description: Returns the mask that is applied to the into
//               CollideMask of nodes in the scene graph before
//               testing for bits in common with the from CollideMask
//               of colliders.  See set_include_mask().
////////////////////////////////////////////////////////////////////
INLINE CollideMask CollisionLevelState::
get_include_mask() const {
  return _include_mask;
}
