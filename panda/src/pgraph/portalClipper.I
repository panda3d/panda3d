// Filename: portalClipper.I
// Created by:  masad (4May04)
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
  draw_hexahedron(_hex_frustum);
}

////////////////////////////////////////////////////////////////////
//     Function: PortalClipper::facing_camera
//       Access: Public
//  Description: checks if the _coords that forms  the plane is
//               facing the camera
////////////////////////////////////////////////////////////////////
INLINE bool PortalClipper::
is_facing_camera()
{
  Planef portal_plane(_coords[0], _coords[1], _coords[2]);
  Planef camera_plane(_hex_frustum->get_point(4), _hex_frustum->get_point(5), _hex_frustum->get_point(6));
#if 0
  // use the camera's near plane to calculate direction
  pgraph_cat.debug() << portal_plane.get_normal() << "; " << -camera_plane.get_normal() << endl;
  float direction = portal_plane.get_normal().dot(-camera_plane.get_normal());
  pgraph_cat.debug() << "Found direction of " << direction << endl;
  return (direction < _FACING_THRESHOLD);
#else
  // use the center of camera's near plane to the center of the
  // portal_plane to calulate the direction
  LPoint3f portal_center = (_coords[0] + _coords[1] + _coords[2] + _coords[3]) / 4;
  LPoint3f camera_center = (_hex_frustum->get_point(4) + _hex_frustum->get_point(5) + _hex_frustum->get_point(6) + _hex_frustum->get_point(7)) / 4;
  LVector3f camera_to_portal = portal_center - camera_center;
  camera_to_portal.normalize();
  pgraph_cat.debug() << "portal_center " << portal_center << "; camera_center " << camera_center << "; camera_to_portal " << camera_to_portal << endl;
  float direction = camera_plane.get_normal().dot(camera_to_portal);
  _color = Colorf(0,1,0,1);
  move_to(camera_center);
  draw_to(portal_center);
#endif
  pgraph_cat.debug() << "Found direction of " << direction << endl;
  return (direction < _FACING_THRESHOLD);
}
