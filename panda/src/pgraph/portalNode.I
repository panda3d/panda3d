// Filename: portalNode.I
// Created by:  masad (13May04)
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
//     Function: PortalNode::set_portal_mask
//       Access: Published
//  Description: Simultaneously sets both the "from" and "into"
//               PortalMask values to the same thing.
////////////////////////////////////////////////////////////////////
INLINE void PortalNode::
set_portal_mask(PortalMask mask) {
  set_from_portal_mask(mask);
  set_into_portal_mask(mask);
}

////////////////////////////////////////////////////////////////////
//     Function: PortalNode::set_from_portal_mask
//       Access: Published
//  Description: Sets the "from" PortalMask.  In order for a
//               portal to be detected from this object into
//               another object, the intersection of this object's
//               "from" mask and the other object's "into" mask must
//               be nonzero.
////////////////////////////////////////////////////////////////////
INLINE void PortalNode::
set_from_portal_mask(PortalMask mask) {
  _from_portal_mask = mask;
}

////////////////////////////////////////////////////////////////////
//     Function: PortalNode::set_into_portal_mask
//       Access: Published
//  Description: Sets the "into" PortalMask.  In order for a
//               portal to be detected from another object into
//               this object, the intersection of the other object's
//               "from" mask and this object's "into" mask must be
//               nonzero.
////////////////////////////////////////////////////////////////////
INLINE void PortalNode::
set_into_portal_mask(PortalMask mask) {
  _into_portal_mask = mask;

  // We mark the bound stale when this changes, not because the actual
  // bounding volume changes, but rather because we piggyback the
  // computing of the _net_portal_mask on the bounding volume.
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: PortalNode::get_from_portal_mask
//       Access: Published
//  Description: Returns the current "from" PortalMask.  In order for
//               a portal to be detected from this object into
//               another object, the intersection of this object's
//               "from" mask and the other object's "into" mask must
//               be nonzero.
////////////////////////////////////////////////////////////////////
INLINE PortalMask PortalNode::
get_from_portal_mask() const {
  return _from_portal_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: PortalNode::get_into_portal_mask
//       Access: Published
//  Description: Returns the current "into" PortalMask.  In order for
//               a portal to be detected from another object into
//               this object, the intersection of the other object's
//               "from" mask and this object's "into" mask must be
//               nonzero.
////////////////////////////////////////////////////////////////////
INLINE PortalMask PortalNode::
get_into_portal_mask() const {
  return _into_portal_mask;
}

////////////////////////////////////////////////////////////////////
//     Function: PortalNode::set_portal_geom
//       Access: Published
//  Description: Sets the state of the "portal geom" flag for this
//               PortalNode.  Normally, this is false; when this is
//               set true, the PortalSolids in this node will test
//               for portals with actual renderable geometry, in
//               addition to whatever PortalSolids may be indicated
//               by the from_portal_mask.
//
//               Setting this to true causes this to test *all*
//               GeomNodes for portals.  It is an all-or-none
//               thing; there is no way to portal with only some
//               GeomNodes, as GeomNodes have no into_portal_mask.
////////////////////////////////////////////////////////////////////
INLINE void PortalNode::
set_portal_geom(bool flag) {
  if (flag) {
    _flags |= F_portal_geom;
  } else {
    _flags &= ~F_portal_geom;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PortalNode::get_portal_geom
//       Access: Published
//  Description: Returns the current state of the portal_geom flag.
//               See set_portal_geom().
////////////////////////////////////////////////////////////////////
INLINE bool PortalNode::
get_portal_geom() const {
  return (_flags & F_portal_geom) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PortalNode::clear_vertices
//       Access: Published
//  Description: Resets the vertices of the portal to the empty list.
////////////////////////////////////////////////////////////////////
INLINE void PortalNode::
clear_vertices() {
  _vertices.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PortalNode::add_vertex
//       Access: Published
//  Description: Adds a new vertex to the portal polygon.  The
//               vertices should be defined in a counterclockwise
//               orientation when viewing through the portal.
////////////////////////////////////////////////////////////////////
INLINE void PortalNode::
add_vertex(const LPoint3f &vertex) {
  _vertices.push_back(vertex);
}

////////////////////////////////////////////////////////////////////
//     Function: PortalNode::get_num_vertices
//       Access: Published
//  Description: Returns the number of vertices in the portal polygon.
////////////////////////////////////////////////////////////////////
INLINE int PortalNode::
get_num_vertices() const {
  return _vertices.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PortalNode::get_vertex
//       Access: Published
//  Description: Returns the nth vertex of the portal polygon.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &PortalNode::
get_vertex(int n) const {
  nassertr(n >= 0 && n < (int)_vertices.size(), LPoint3f::zero());
  return _vertices[n];
}

////////////////////////////////////////////////////////////////////
//     Function: PortalNode::set_zone_in
//       Access: Published
//  Description: Sets the zone that this portal belongs to
////////////////////////////////////////////////////////////////////
INLINE void PortalNode::set_zone_in(PandaNode *zone) {
  _zone_in = zone;
}

////////////////////////////////////////////////////////////////////
//     Function: PortalNode::get_zone_in
//       Access: Published
//  Description: Sets the zone that this portal belongs to
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PortalNode::get_zone_in() const {
  return _zone_in;
}
////////////////////////////////////////////////////////////////////
//     Function: PortalNode::set_zone_out
//       Access: Published
//  Description: Sets the zone that this portal leads out to
////////////////////////////////////////////////////////////////////
INLINE void PortalNode::set_zone_out(PandaNode *zone) {
  _zone_out = zone;
}

////////////////////////////////////////////////////////////////////
//     Function: PortalNode::get_zone_out
//       Access: Published
//  Description: Sets the zone that this portal leads out to
////////////////////////////////////////////////////////////////////
INLINE PandaNode *PortalNode::get_zone_out() const {
  return _zone_out;
}

////////////////////////////////////////////////////////////////////
//     Function: PortalNode::set_visible
//       Access: Published
//  Description: Python sets this based on curent camera zone
////////////////////////////////////////////////////////////////////
INLINE void PortalNode::set_visible(bool value) {
  _visible = value;
}

////////////////////////////////////////////////////////////////////
//     Function: PortalNode::is_visible
//       Access: Published
//  Description: Is this portal visible from current camera zone
////////////////////////////////////////////////////////////////////
INLINE bool PortalNode::is_visible() {
  return _visible;
}
