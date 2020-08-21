/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file portalClipper.I
 * @author masad
 * @date 2004-05-04
 */

/**
 *
 */
INLINE PortalClipper::Point::
Point() {
}

/**
 *
 */
INLINE PortalClipper::Point::
Point(const LVecBase3 &point, const LColor &color) :
  _point(point[0], point[1], point[2]),
  _color(color)
{
}

/**
 *
 */
INLINE PortalClipper::Point::
Point(const PortalClipper::Point &copy) :
  _point(copy._point),
  _color(copy._color)
{
}

/**
 *
 */
INLINE void PortalClipper::Point::
operator = (const PortalClipper::Point &copy) {
  _point = copy._point;
  _color = copy._color;
}

/**
 * Moves the pen to the given point without drawing a line.  When followed by
 * draw_to(), this marks the first point of a line segment; when followed by
 * move_to() or create(), this creates a single point.
 */
INLINE void PortalClipper::
move_to(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  move_to(LVertex(x, y, z));
}

/**
 * Draws a line segment from the pen's last position (the last call to move_to
 * or draw_to) to the indicated point.  move_to() and draw_to() only update
 * tables; the actual drawing is performed when create() is called.
 */
INLINE void PortalClipper::
draw_to(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  draw_to(LVertex(x, y, z));
}

/**
 * Draw the current camera frustum in white color
 *
 */
INLINE void PortalClipper::
draw_camera_frustum() {
  _color = LColor(1,1,1,1);
  draw_hexahedron(_view_frustum);
}

/**
 * Set the current view frustum that is being calculated by the portal clipper
 *
 */
INLINE void PortalClipper::
set_reduced_frustum(BoundingHexahedron *frustum) {
  _reduced_frustum = frustum;
}

/**
 * Return the reduced frustum
 */
INLINE BoundingHexahedron *PortalClipper::
get_reduced_frustum() const {
  return _reduced_frustum;
}

/**
 * Set the clip state of the current portal node This is done to remember the
 * state for the child portal nodes
 *
 */
INLINE void PortalClipper::
set_clip_state(const RenderState* clip_state) {
  _clip_state = clip_state;
}

/**
 * Returns the stored clip state
 */
INLINE const RenderState *PortalClipper::
get_clip_state() const {
  return _clip_state;
}

/**
 * Set the current viewport that is being used by the portal clipper
 *
 */
INLINE void PortalClipper::
set_reduced_viewport(const LPoint2& min, const LPoint2& max) {
  _reduced_viewport_min = min;
  _reduced_viewport_max = max;
}


/**
 * Return the reduced viewport
 */
INLINE void PortalClipper::
get_reduced_viewport(LPoint2& min, LPoint2& max) const  {
  min = _reduced_viewport_min;
  max = _reduced_viewport_max;
}



/**
 * checks if the portal plane (in camera space) is facing the camera's near
 * plane
 */
INLINE bool PortalClipper::
is_facing_view(const LPlane &portal_plane) {
  if (portal_cat.is_debug()) {
    portal_cat.debug() << "portal plane check value: " << portal_plane[3] << "\n";
  }
  return (portal_plane[3] > 0);
}

/**
 * checks if portal_node is within the view frustum.  If so, then the portal
 * is worth considering.  This is a 2nd level test to weed out most of the
 * portals
 */
INLINE bool PortalClipper::
is_whole_portal_in_view(const LMatrix4 &cmat) {
  // I am about to xform this gbv, so lets make a copy
  const BoundingVolume *bv = _portal_node->get_bounds();
  BoundingVolume *cbv = bv->make_copy();
  GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, cbv);

  // trasform it to camera space
  gbv->xform(cmat);

  int result = _reduced_frustum->contains(gbv);

  if (portal_cat.is_spam()) {
    portal_cat.spam() << "1st level test if portal: " << *_reduced_frustum << " is in view " << result << std::endl;
  }
  return (result != 0);
}

/**
 * checks if any of the _coords is within the view frustum.  If so, then the
 * portal is facing the camera.  2nd level test to make sure this portal is
 * worth visiting
 */
/*
INLINE bool PortalClipper::
is_partial_portal_in_view() {
  int result = 0;

  // check if any of the _coords in tested frustum
  for (int j=0; j<_num_vert; ++j) {
    result |= _reduced_frustum->contains(_coords[j]);
  }
  if (portal_cat.is_spam()) {
    portal_cat.spam() << "frustum: " << *_reduced_frustum << " contains(coord) result = " << result << endl;
  }

  return (result != 0);
}
*/

/**
 * Given the x and z, solve for y: from the plane
 */
/*
INLINE PN_stdfloat PortalClipper::
get_plane_depth(PN_stdfloat x, PN_stdfloat z, LPlane *portal_plane) {
  PN_stdfloat y = 0.0;
  // Plane equation: Ax + By + Cz + D = 0 y = (Ax + Cz + D)  -B
  if (portal_cat.is_spam()) {
    portal_cat.spam() << *portal_plane << endl;
    portal_cat.spam() << portal_plane->_v.v._0 << " " << portal_plane->_v.v._1 << " "
                      << portal_plane->_v.v._2 << " " << portal_plane->_v.v._3 << endl;
  }

  if (portal_plane->_v.v._1 != 0.0) {
    y = (((portal_plane->_v.v._0*x)+(portal_plane->_v.v._2*z)+portal_plane->_v.v._3)
         / -(portal_plane->_v.v._1));
  }
  return y;
}
*/
