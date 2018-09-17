/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionLevelStateBase.I
 * @author drose
 * @date 2002-03-16
 */

/**
 *
 */
INLINE CollisionLevelStateBase::
CollisionLevelStateBase(const NodePath &node_path) :
  _node_path(node_path),
  _colliders(get_class_type()),
  _include_mask(CollideMask::all_on())
{
}

/**
 * This constructor goes to the next child node in the traversal.
 */
INLINE CollisionLevelStateBase::
CollisionLevelStateBase(const CollisionLevelStateBase &parent, PandaNode *child) :
  _node_path(parent._node_path, child),
  _colliders(parent._colliders),
  _include_mask(parent._include_mask),
  _local_bounds(parent._local_bounds)
{
}

/**
 *
 */
INLINE CollisionLevelStateBase::
CollisionLevelStateBase(const CollisionLevelStateBase &copy) :
  _node_path(copy._node_path),
  _colliders(copy._colliders),
  _include_mask(copy._include_mask),
  _local_bounds(copy._local_bounds),
  _parent_bounds(copy._parent_bounds)
{
}

/**
 *
 */
INLINE void CollisionLevelStateBase::
operator = (const CollisionLevelStateBase &copy) {
  _node_path = copy._node_path;
  _colliders = copy._colliders;
  _include_mask = copy._include_mask;
  _local_bounds = copy._local_bounds;
  _parent_bounds = copy._parent_bounds;
}

/**
 * Returns the NodePath representing the node instance we have traversed to.
 */
INLINE NodePath CollisionLevelStateBase::
get_node_path() const {
  return _node_path.get_node_path();
}

/**
 * Returns the PandaNode pointer of the node we have traversed to.
 */
INLINE PandaNode *CollisionLevelStateBase::
node() const {
  return _node_path.node();
}

/**
 *
 */
INLINE int CollisionLevelStateBase::
get_num_colliders() const {
  return _colliders.size();
}

/**
 *
 */
INLINE const CollisionSolid *CollisionLevelStateBase::
get_collider(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), nullptr);

  return _colliders[n]._collider;
}

/**
 *
 */
INLINE CollisionNode *CollisionLevelStateBase::
get_collider_node(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), nullptr);

  return _colliders[n]._node;
}

/**
 *
 */
INLINE NodePath CollisionLevelStateBase::
get_collider_node_path(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), NodePath::fail());

  return _colliders[n]._node_path;
}

/**
 * Returns the bounding volume of the indicated collider, transformed into the
 * current node's transform space.
 */
INLINE const GeometricBoundingVolume *CollisionLevelStateBase::
get_local_bound(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), nullptr);
  nassertr(n >= 0 && n < (int)_local_bounds.size(), nullptr);

  // For whatever reason, the Intel compiler can't figure this line out.
  // return _local_bounds[n];

  // But it can figure out this equivalent line.
  return *(_local_bounds + n);
}

/**
 * Returns the bounding volume of the indicated collider, transformed into the
 * previous node's transform space, but not transformed by the current node's
 * transform.  This is appropriate for testing against the bounding volume of
 * the current node (which does not have its own transform applied to it).
 */
INLINE const GeometricBoundingVolume *CollisionLevelStateBase::
get_parent_bound(int n) const {
  nassertr(n >= 0 && n < (int)_colliders.size(), nullptr);
  nassertr(n >= 0 && n < (int)_parent_bounds.size(), nullptr);

  // But it can figure out this equivalent line.
  return *(_parent_bounds + n);
}

/**
 * Specifies the mask that is applied to the into CollideMask of nodes in the
 * scene graph before testing for bits in common with the from CollideMask of
 * colliders.  This is normally all bits on, but you may set it to some other
 * mask to restrict certain bits from consideration.
 *
 * This is used by the CollisionTraverser to restrict collision with geometry
 * except under the lowest level of LOD.
 */
INLINE void CollisionLevelStateBase::
set_include_mask(CollideMask include_mask) {
  _include_mask = include_mask;
}


/**
 * Returns the mask that is applied to the into CollideMask of nodes in the
 * scene graph before testing for bits in common with the from CollideMask of
 * colliders.  See set_include_mask().
 */
INLINE CollideMask CollisionLevelStateBase::
get_include_mask() const {
  return _include_mask;
}
