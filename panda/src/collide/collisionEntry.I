/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionEntry.I
 * @author drose
 * @date 2002-03-16
 */

/**
 *
 */
INLINE CollisionEntry::
CollisionEntry() {
  _flags = 0;
  // > 1. means collision didn't happen
  _t = 2.f;
}

/**
 * Returns the CollisionSolid pointer for the particular solid that triggered
 * this collision.
 */
INLINE const CollisionSolid *CollisionEntry::
get_from() const {
  return _from;
}

/**
 * Returns true if the "into" solid is, in fact, a CollisionSolid, and its
 * pointer is known (in which case get_into() may be called to retrieve it).
 * If this returns false, the collision was detected into a GeomNode, and
 * there is no CollisionSolid pointer to be retrieved.
 */
INLINE bool CollisionEntry::
has_into() const {
  return (_into != nullptr);
}

/**
 * Returns the CollisionSolid pointer for the particular solid was collided
 * into.  This pointer might be NULL if the collision was into a piece of
 * visible geometry, instead of a normal CollisionSolid collision; see
 * has_into().
 */
INLINE const CollisionSolid *CollisionEntry::
get_into() const {
  return _into;
}

/**
 * Returns the node that contains the CollisionSolid that triggered this
 * collision.  This will be a node that has been added to a CollisionTraverser
 * via add_collider().
 */
INLINE CollisionNode *CollisionEntry::
get_from_node() const {
  return _from_node;
}

/**
 * Returns the node that contains the CollisionSolid that was collided into.
 * This returns a PandaNode pointer instead of something more specific,
 * because it might be either a CollisionNode or a GeomNode.
 *
 * Also see get_into_node_path().
 */
INLINE PandaNode *CollisionEntry::
get_into_node() const {
  return _into_node;
}

/**
 * Returns the NodePath that represents the CollisionNode that contains the
 * CollisionSolid that triggered this collision.  This will be a NodePath that
 * has been added to a CollisionTraverser via add_collider().
 */
INLINE NodePath CollisionEntry::
get_from_node_path() const {
  return _from_node_path;
}

/**
 * Returns the NodePath that represents the specific CollisionNode or GeomNode
 * instance that was collided into.  This is the same node returned by
 * get_into_node(), represented as a NodePath; however, it may be more useful
 * because the NodePath can resolve the particular instance of the node, if
 * there is more than one.
 */
INLINE NodePath CollisionEntry::
get_into_node_path() const {
  return _into_node_path;
}

/**
 * Sets a time value for this collision relative to other CollisionEntries
 */
INLINE void CollisionEntry::
set_t(PN_stdfloat t) {
  nassertv(!cnan(t));
  _t = t;
}

/**
 * returns time value for this collision relative to other CollisionEntries
 */
INLINE PN_stdfloat CollisionEntry::
get_t() const {
  return _t;
}

/**
 * returns true if this represents an actual collision as opposed to a
 * potential collision, needed for iterative collision resolution where path
 * of collider changes mid-frame
 */
INLINE bool CollisionEntry::
collided() const {
  return ((0.f <= _t) && (_t <= 1.f));
}

/**
 * prepare for another collision test
 */
INLINE void CollisionEntry::
reset_collided() {
  _t = 2.f;
}

/**
 * Returns true if the collision was detected by a CollisionTraverser whose
 * respect_prev_transform flag was set true, meaning we should consider motion
 * significant in evaluating collisions.
 */
INLINE bool CollisionEntry::
get_respect_prev_transform() const {
  return (_flags & F_respect_prev_transform) != 0;
}


/**
 * Stores the point, on the surface of the "into" object, at which a collision
 * is detected.
 *
 * This point is specified in the coordinate space of the "into" object.
 */
INLINE void CollisionEntry::
set_surface_point(const LPoint3 &point) {
  nassertv(!point.is_nan());
  _surface_point = point;
  _flags |= F_has_surface_point;
}

/**
 * Stores the surface normal of the "into" object at the point of the
 * intersection.
 *
 * This normal is specified in the coordinate space of the "into" object.
 */
INLINE void CollisionEntry::
set_surface_normal(const LVector3 &normal) {
  nassertv(!normal.is_nan());
  _surface_normal = normal;
  _flags |= F_has_surface_normal;
}

/**
 * Stores the point, within the interior of the "into" object, which
 * represents the depth to which the "from" object has penetrated.  This can
 * also be described as the intersection point on the surface of the "from"
 * object (which is inside the "into" object).
 *
 * This point is specified in the coordinate space of the "into" object.
 */
INLINE void CollisionEntry::
set_interior_point(const LPoint3 &point) {
  nassertv(!point.is_nan());
  _interior_point = point;
  _flags |= F_has_interior_point;
}

/**
 * Returns true if the surface point has been specified, false otherwise.  See
 * get_surface_point().  Some types of collisions may not compute the surface
 * point.
 */
INLINE bool CollisionEntry::
has_surface_point() const {
  return (_flags & F_has_surface_point) != 0;
}

/**
 * Returns true if the surface normal has been specified, false otherwise.
 * See get_surface_normal().  Some types of collisions may not compute the
 * surface normal.
 */
INLINE bool CollisionEntry::
has_surface_normal() const {
  return (_flags & F_has_surface_normal) != 0;
}

/**
 * Returns true if the interior point has been specified, false otherwise.
 * See get_interior_point().  Some types of collisions may not compute the
 * interior point.
 */
INLINE bool CollisionEntry::
has_interior_point() const {
  return (_flags & F_has_interior_point) != 0;
}

/**
 * Stores the position of the "from" object at the instant at which the
 * collision is first detected.
 *
 * This position is specified in the coordinate space of the "into" object.
 */
INLINE void CollisionEntry::
set_contact_pos(const LPoint3 &pos) {
  nassertv(!pos.is_nan());
  _contact_pos = pos;
  _flags |= F_has_contact_pos;
}

/**
 * Stores the surface normal of the "into" object at the contact pos.
 *
 * This normal is specified in the coordinate space of the "into" object.
 */
INLINE void CollisionEntry::
set_contact_normal(const LVector3 &normal) {
  nassertv(!normal.is_nan());
  _contact_normal = normal;
  _flags |= F_has_contact_normal;
}

/**
 * Returns true if the contact position has been specified, false otherwise.
 * See get_contact_pos().  Some types of collisions may not compute the
 * contact pos.
 */
INLINE bool CollisionEntry::
has_contact_pos() const {
  return (_flags & F_has_contact_pos) != 0;
}

/**
 * Returns true if the contact normal has been specified, false otherwise.
 * See get_contact_normal().  Some types of collisions may not compute the
 * contact normal.
 */
INLINE bool CollisionEntry::
has_contact_normal() const {
  return (_flags & F_has_contact_normal) != 0;
}

/**
 * Returns the relative transform of the from node as seen from the into node.
 */
INLINE CPT(TransformState) CollisionEntry::
get_wrt_space() const {
  return _from_node_path.get_transform(_into_node_path);
}

/**
 * Returns the relative transform of the into node as seen from the from node.
 */
INLINE CPT(TransformState) CollisionEntry::
get_inv_wrt_space() const {
  return _into_node_path.get_transform(_from_node_path);
}

/**
 * Returns the relative transform of the from node as seen from the into node,
 * as of the previous frame (according to set_prev_transform(),
 * set_fluid_pos(), etc.)
 */
INLINE CPT(TransformState) CollisionEntry::
get_wrt_prev_space() const {
  if (get_respect_prev_transform()) {
    return _from_node_path.get_prev_transform(_into_node_path);
  } else {
    return get_wrt_space();
  }
}

/**
 * Returns the relative transform of the from node as seen from the into node.
 */
INLINE const LMatrix4 &CollisionEntry::
get_wrt_mat() const {
  return get_wrt_space()->get_mat();
}

/**
 * Returns the relative transform of the into node as seen from the from node.
 */
INLINE const LMatrix4 &CollisionEntry::
get_inv_wrt_mat() const {
  return get_inv_wrt_space()->get_mat();
}

/**
 * Returns the relative transform of the from node as seen from the into node,
 * as of the previous frame (according to set_prev_transform(),
 * set_fluid_pos(), etc.)
 */
INLINE const LMatrix4 &CollisionEntry::
get_wrt_prev_mat() const {
  return get_wrt_prev_space()->get_mat();
}

/**
 * Returns the ClipPlaneAttrib, if any, that is applied to the into_node_path,
 * or NULL if there is no clip plane in effect.
 */
INLINE const ClipPlaneAttrib *CollisionEntry::
get_into_clip_planes() const {
  if ((_flags & F_checked_clip_planes) == 0) {
    ((CollisionEntry *)this)->check_clip_planes();
  }
  return _into_clip_planes;
}

/**
 * This is intended to be called only by the CollisionTraverser.  It requests
 * the CollisionEntry to start the intersection test between the from and into
 * solids stored within it, passing the result (if positive) to the indicated
 * CollisionHandler.
 */
INLINE void CollisionEntry::
test_intersection(CollisionHandler *record,
                  const CollisionTraverser *trav) const {
  PT(CollisionEntry) result = get_from()->test_intersection(*this);
#ifdef DO_COLLISION_RECORDING
  if (trav->has_recorder()) {
    if (result != nullptr) {
      trav->get_recorder()->collision_tested(*result, true);
    } else {
      trav->get_recorder()->collision_tested(*this, false);
    }
  }
#endif  // DO_COLLISION_RECORDING
#ifdef DO_PSTATS
  ((CollisionSolid *)get_into())->get_test_pcollector().add_level(1);
#endif  // DO_PSTATS
  // if there was no collision detected but the handler wants to know about
  // all potential collisions, create a "didn't collide" collision entry for
  // it
  if (record->wants_all_potential_collidees() && result == nullptr) {
    result = new CollisionEntry(*this);
    result->reset_collided();
  }
  if (result != nullptr) {
    record->add_entry(result);
  }
}

INLINE std::ostream &
operator << (std::ostream &out, const CollisionEntry &entry) {
  entry.output(out);
  return out;
}
