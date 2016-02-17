/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file portalNode.I
 * @author masad
 * @date 2004-05-13
 */

/**
 * Simultaneously sets both the "from" and "into" PortalMask values to the
 * same thing.
 */
INLINE void PortalNode::
set_portal_mask(PortalMask mask) {
  set_from_portal_mask(mask);
  set_into_portal_mask(mask);
}

/**
 * Sets the "from" PortalMask.  In order for a portal to be detected from this
 * object into another object, the intersection of this object's "from" mask
 * and the other object's "into" mask must be nonzero.
 */
INLINE void PortalNode::
set_from_portal_mask(PortalMask mask) {
  _from_portal_mask = mask;
}

/**
 * Sets the "into" PortalMask.  In order for a portal to be detected from
 * another object into this object, the intersection of the other object's
 * "from" mask and this object's "into" mask must be nonzero.
 */
INLINE void PortalNode::
set_into_portal_mask(PortalMask mask) {
  _into_portal_mask = mask;

  // We mark the bound stale when this changes, not because the actual
  // bounding volume changes, but rather because we piggyback the computing of
  // the _net_portal_mask on the bounding volume.
  mark_bounds_stale();
}

/**
 * Returns the current "from" PortalMask.  In order for a portal to be
 * detected from this object into another object, the intersection of this
 * object's "from" mask and the other object's "into" mask must be nonzero.
 */
INLINE PortalMask PortalNode::
get_from_portal_mask() const {
  return _from_portal_mask;
}

/**
 * Returns the current "into" PortalMask.  In order for a portal to be
 * detected from another object into this object, the intersection of the
 * other object's "from" mask and this object's "into" mask must be nonzero.
 */
INLINE PortalMask PortalNode::
get_into_portal_mask() const {
  return _into_portal_mask;
}

/**
 * Sets the state of the "portal geom" flag for this PortalNode.  Normally,
 * this is false; when this is set true, the PortalSolids in this node will
 * test for portals with actual renderable geometry, in addition to whatever
 * PortalSolids may be indicated by the from_portal_mask.
 *
 * Setting this to true causes this to test *all* GeomNodes for portals.  It
 * is an all-or-none thing; there is no way to portal with only some
 * GeomNodes, as GeomNodes have no into_portal_mask.
 */
INLINE void PortalNode::
set_portal_geom(bool flag) {
  if (flag) {
    _flags |= F_portal_geom;
  } else {
    _flags &= ~F_portal_geom;
  }
}

/**
 * Returns the current state of the portal_geom flag.  See set_portal_geom().
 */
INLINE bool PortalNode::
get_portal_geom() const {
  return (_flags & F_portal_geom) != 0;
}

/**
 * Resets the vertices of the portal to the empty list.
 */
INLINE void PortalNode::
clear_vertices() {
  _vertices.clear();
}

/**
 * Adds a new vertex to the portal polygon.  The vertices should be defined in
 * a counterclockwise orientation when viewing through the portal.
 */
INLINE void PortalNode::
add_vertex(const LPoint3 &vertex) {
  _vertices.push_back(vertex);
}

/**
 * Returns the number of vertices in the portal polygon.
 */
INLINE int PortalNode::
get_num_vertices() const {
  return _vertices.size();
}

/**
 * Returns the nth vertex of the portal polygon.
 */
INLINE const LPoint3 &PortalNode::
get_vertex(int n) const {
  nassertr(n >= 0 && n < (int)_vertices.size(), LPoint3::zero());
  return _vertices[n];
}

/**
 * Sets the cell that this portal belongs to
 */
INLINE void PortalNode::set_cell_in(const NodePath &cell) {
  _cell_in = cell;
}

/**
 * Sets the cell that this portal belongs to
 */
INLINE NodePath PortalNode::get_cell_in() const {
  return _cell_in;
}
/**
 * Sets the cell that this portal leads out to
 */
INLINE void PortalNode::set_cell_out(const NodePath &cell) {
  _cell_out = cell;
}

/**
 * Sets the cell that this portal leads out to
 */
INLINE NodePath PortalNode::get_cell_out() const {
  return _cell_out;
}

/**
 * this is set if the portal will clip against its left and right planes
 */
INLINE void PortalNode::set_clip_plane(bool value) {
  _clip_plane = value;
  if (_clip_plane) {
    enable_clipping_planes();
  }
}

/**
 * Is this portal clipping against its left-right planes
 */
INLINE bool PortalNode::is_clip_plane() {
  return _clip_plane;
}


/**
 * this is set if the portal is facing camera
 */
INLINE void PortalNode::set_visible(bool value) {
  _visible = value;
}

/**
 * Is this portal facing the camera
 */
INLINE bool PortalNode::is_visible() {
  return _visible;
}

/**
 * Python sets this based on curent camera zone
 */
INLINE void PortalNode::set_open(bool value) {
  _open = value;
}

/**
 * Is this portal open from current camera zone
 */
INLINE bool PortalNode::is_open() {
  return _open;
}

/**
 * Set the maximum depth this portal will be visible at
 */
INLINE void PortalNode::set_max_depth(int value) {
  _max_depth = value;
}

/**
 * Returns the maximum depth this portal will be visible at
 */
INLINE int PortalNode::get_max_depth() {
  return _max_depth;
}
