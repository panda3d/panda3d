/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionNode.I
 * @author drose
 * @date 2002-03-16
 */

/**
 * Simultaneously sets both the "from" and "into" CollideMask values to the
 * same thing.
 */
INLINE void CollisionNode::
set_collide_mask(CollideMask mask) {
  set_from_collide_mask(mask);
  set_into_collide_mask(mask);
}

/**
 * Sets the "into" CollideMask.  In order for a collision to be detected from
 * another object into this object, the intersection of the other object's
 * "from" mask and this object's "into" mask must be nonzero.
 */
INLINE void CollisionNode::
set_into_collide_mask(CollideMask mask) {
  // This is now inherited from the PandaNode base class.
  PandaNode::set_into_collide_mask(mask);
}

/**
 * Returns the current "from" CollideMask.  In order for a collision to be
 * detected from this object into another object, the intersection of this
 * object's "from" mask and the other object's "into" mask must be nonzero.
 */
INLINE CollideMask CollisionNode::
get_from_collide_mask() const {
  return _from_collide_mask;
}

/**
 * Returns the current "into" CollideMask.  In order for a collision to be
 * detected from another object into this object, the intersection of the
 * other object's "from" mask and this object's "into" mask must be nonzero.
 */
INLINE CollideMask CollisionNode::
get_into_collide_mask() const {
  // This is now inherited from the PandaNode base class.
  return PandaNode::get_into_collide_mask();
}

/**
 * Removes all solids from the node.
 */
INLINE void CollisionNode::
clear_solids() {
  _solids.clear();
  mark_internal_bounds_stale();
}

/**
 *
 */
INLINE size_t CollisionNode::
get_num_solids() const {
  return _solids.size();
}

/**
 *
 */
INLINE CPT(CollisionSolid) CollisionNode::
get_solid(size_t n) const {
  nassertr(n < get_num_solids(), nullptr);
  return _solids[n].get_read_pointer();
}

/**
 *
 */
INLINE PT(CollisionSolid) CollisionNode::
modify_solid(size_t n) {
  nassertr(n < get_num_solids(), nullptr);
  mark_internal_bounds_stale();
  return _solids[n].get_write_pointer();
}

/**
 * Replaces the solid with the indicated index.
 */
INLINE void CollisionNode::
set_solid(size_t n, CollisionSolid *solid) {
  nassertv(n < get_num_solids());
  _solids[n] = solid;
  mark_internal_bounds_stale();
}

/**
 * Inserts the indicated solid to the node at the indicated position.
 */
INLINE void CollisionNode::
insert_solid(size_t n, const CollisionSolid *solid) {
  if (n > _solids.size()) {
    n = _solids.size();
  }
  _solids.insert(_solids.begin() + n, (CollisionSolid *)solid);
  mark_internal_bounds_stale();
}

/**
 * Removes the solid with the indicated index.  This will shift all subsequent
 * indices down by one.
 */
INLINE void CollisionNode::
remove_solid(size_t n) {
  nassertv(n < get_num_solids());
  _solids.erase(_solids.begin() + n);
  mark_internal_bounds_stale();
}

/**
 * Adds the indicated solid to the node.  Returns the index of the new solid
 * within the node's list of solids.
 */
INLINE size_t CollisionNode::
add_solid(const CollisionSolid *solid) {
  _solids.push_back((CollisionSolid *)solid);
  mark_internal_bounds_stale();
  return _solids.size() - 1;
}

/**
 * Returns the collider_sort value that has been set for this particular node.
 * See set_collider_sort().
 */
INLINE int CollisionNode::
get_collider_sort() const {
  return _collider_sort;
}

/**
 * Sets a particular collider_sort value on this node.  This controls the
 * order in which colliders (that is, "from nodes") are grouped together for
 * the collision traversal.
 *
 * If there are 32 or fewer colliders added to any particular
 * CollisionTraverser, then this value has no meaning.  It is only useful if
 * there are many colliders, which may force the CollisionTraverser to make
 * multiple passes through the data; in that case, it may be a useful
 * optimization to group colliders that have similar bounding volumes together
 * (by giving them similar sort values).
 */
INLINE void CollisionNode::
set_collider_sort(int sort) {
  _collider_sort = sort;
}

/**
 * Returns the default into_collide_mask assigned to new CollisionNodes.
 */
INLINE CollideMask CollisionNode::
get_default_collide_mask() {
  return default_collision_node_collide_mask;
}
