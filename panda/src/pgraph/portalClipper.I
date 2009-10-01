// Filename: portalClipper.I
// Created by:  masad (4May04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::Point::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PortalClipper::Point::
Point() {
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::Point::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PortalClipper::Point::
Point(const LVecBase3f &point, const Colorf &color) :
  _point(point[0], point[1], point[2]),
  _color(color)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::Point::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PortalClipper::Point::
Point(const PortalClipper::Point &copy) :
  _point(copy._point),
  _color(copy._color)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::Point::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PortalClipper::Point::
operator = (const PortalClipper::Point &copy) {
  _point = copy._point;
  _color = copy._color;
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::move_to
//       Access: Public
//  Description: Moves the pen to the given point without drawing a
//               line.  When followed by draw_to(), this marks the
//               first point of a line segment; when followed by
//               move_to() or create(), this creates a single point.
////////////////////////////////////////////////////////////////////
INLINE void PortalClipper::
move_to(float x, float y, float z) {
  move_to(Vertexf(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::draw_to
//       Access: Public
//  Description: Draws a line segment from the pen's last position
//               (the last call to move_to or draw_to) to the
//               indicated point.  move_to() and draw_to() only update
//               tables; the actual drawing is performed when create()
//               is called.
////////////////////////////////////////////////////////////////////
INLINE void PortalClipper::
draw_to(float x, float y, float z) {
  draw_to(Vertexf(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::draw_camera_frustum
//       Access: Public
//  Description: Draw the current camera frustum in white color
//           
////////////////////////////////////////////////////////////////////
INLINE void PortalClipper::
draw_camera_frustum() {
  _color = Colorf(1,1,1,1);
  draw_hexahedron(_view_frustum);
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::set_reduced_frustum
//       Access: Public
//  Description: Set the current view frustum that is being calculated
//               by the portal clipper
//           
////////////////////////////////////////////////////////////////////
INLINE void PortalClipper::
set_reduced_frustum(BoundingHexahedron *frustum) {
  _reduced_frustum = frustum;
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::get_reduced_frustum
//       Access: Published
//  Description: Return the reduced frustum
////////////////////////////////////////////////////////////////////
INLINE BoundingHexahedron *PortalClipper::
get_reduced_frustum() const {
  return _reduced_frustum;
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::set_clip_state
//       Access: Public
//  Description: Set the clip state of the current portal node
//               This is done to remember the state for the child portal nodes
//           
////////////////////////////////////////////////////////////////////
INLINE void PortalClipper::
set_clip_state(const RenderState* clip_state) {
  _clip_state = clip_state;
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::get_clip_state
//       Access: Published
//  Description: Returns the stored clip state
////////////////////////////////////////////////////////////////////
INLINE const RenderState *PortalClipper::
get_clip_state() const {
  return _clip_state;
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::set_reduced_viewport
//       Access: Public
//  Description: Set the current viewport that is being used
//               by the portal clipper
//           
////////////////////////////////////////////////////////////////////
INLINE void PortalClipper::
set_reduced_viewport(const LPoint2f& min, const LPoint2f& max) {
  _reduced_viewport_min = min;
  _reduced_viewport_max = max;
}


////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::get_reduced_viewport
//       Access: Published
//  Description: Return the reduced viewport
////////////////////////////////////////////////////////////////////
INLINE void PortalClipper::
get_reduced_viewport(LPoint2f& min, LPoint2f& max) const  {
  min = _reduced_viewport_min;
  max = _reduced_viewport_max;
}



////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::is_facing_view
//       Access: Public
//  Description: checks if the portal plane (in camera space)
//               is facing the camera's near plane
////////////////////////////////////////////////////////////////////
INLINE bool PortalClipper::
is_facing_view(Planef portal_plane) {
  //  Planef view_plane(_reduced_frustum->get_point(4), _reduced_frustum->get_point(5), _reduced_frustum->get_point(6));

  // use the view_frustum's near plane to calculate direction
  //portal_cat.spam() << portal_plane.get_normal() << "; " << -view_plane.get_normal() << endl;
  //float direction = portal_plane.get_normal().dot(-view_plane.get_normal());
  portal_cat.spam() << portal_plane.get_normal() << "; " << Normalf::forward() << endl;
  float direction = portal_plane.get_normal().dot(Normalf::forward());
  portal_cat.spam() << "Found direction of " << direction << endl;
  return (direction < _FACING_THRESHOLD);
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::is_whole_portal_in_view
//       Access: Public
//  Description: checks if portal_node is within the view frustum.
//               If so, then the portal is worth considering. This
//               is a 2nd level test to weed out most of the portals
////////////////////////////////////////////////////////////////////
INLINE bool PortalClipper::
is_whole_portal_in_view(LMatrix4f cmat) {
  // I am about to xform this gbv, so lets make a copy
  const BoundingVolume *bv = _portal_node->get_bounds();
  BoundingVolume *cbv = bv->make_copy();
  GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, cbv);

  // trasform it to camera space
  gbv->xform(cmat);

  int result = _reduced_frustum->contains(gbv);

  portal_cat.spam() << "1st level test if portal: " << *_reduced_frustum << " is in view " << result << endl;
  return (result != 0);
}

/*
////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::is_partial_portal_in_view
//       Access: Public
//  Description: checks if any of the _coords is within the view frustum.
//               If so, then the portal is facing the camera. 2nd level
//               test to make sure this portal is worth visiting
////////////////////////////////////////////////////////////////////
INLINE bool PortalClipper::
is_partial_portal_in_view() {
  int result = 0;
 
  // check if any of the _coords in tested frustum
  for (int j=0; j<_num_vert; ++j) {
    result |= _reduced_frustum->contains(_coords[j]);
  }
  portal_cat.spam() << "frustum: " << *_reduced_frustum << " contains(coord) result = " << result << endl;

  return (result != 0);
}
*/

/*
////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::get_plane_depth
//       Access: Public
//  Description: Given the x and z, solve for y: from the plane
////////////////////////////////////////////////////////////////////
INLINE float PortalClipper::
get_plane_depth(float x, float z, Planef *portal_plane) {
  float y = 0.0;
  // Plane equation: Ax + By + Cz + D = 0
  // y = (Ax + Cz + D) / -B
  portal_cat.spam() << *portal_plane << endl;
  portal_cat.spam() << portal_plane->_v.v._0 << " " << portal_plane->_v.v._1 << " "
                     << portal_plane->_v.v._2 << " " << portal_plane->_v.v._3 << endl;

  if (portal_plane->_v.v._1 != 0.0) {
    y = (((portal_plane->_v.v._0*x)+(portal_plane->_v.v._2*z)+portal_plane->_v.v._3)
         / -(portal_plane->_v.v._1));
  }
  return y;    
}
*/

