// Filename: collisionBox.I
// Created by:  amith tudur (31Jul09)
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
//     Function: CollisionBox::Constructor
//       Access: Public
//  Description: Create the Box by giving a Center and distances of
//               of each of the sides of box from the Center.
////////////////////////////////////////////////////////////////////
INLINE CollisionBox::
CollisionBox(const LPoint3 &center, PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) :
  _center(center), _x(x), _y(y), _z(z)
{
  _min = LPoint3(_center.get_x() - _x, _center.get_y() - _y, _center.get_z() - _z);
  _max = LPoint3(_center.get_x() + _x, _center.get_y() + _y, _center.get_z() + _z); 
  _radius = sqrt(_x*_x + _y*_y + _z*_z);
  for(int v = 0; v < 8; v++)
    _vertex[v] = get_point_aabb(v);
  for(int p = 0; p < 6; p++)
    _planes[p] = set_plane(p);
  setup_box();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::Constructor
//       Access: Public
//  Description: Create the Box by Specifying the Diagonal Points
////////////////////////////////////////////////////////////////////
INLINE CollisionBox::
CollisionBox(const LPoint3 &min, const LPoint3 &max) :
  _min(min), _max(max)
{
  _center = (_min + _max) / 2;
  _x = _center.get_x() - _min.get_x();
  _y = _center.get_y() - _min.get_y();
  _z = _center.get_z() - _min.get_z();
  _radius = sqrt(_x*_x + _y*_y + _z*_z);
  for(int v = 0; v < 8; v++)
    _vertex[v] = get_point_aabb(v);
  for(int p = 0; p < 6; p++)
    _planes[p] = set_plane(p);
  setup_box();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::Default constructor
//       Access: Protected
//  Description: Creates an invalid Box.  Only used when reading
//               from a bam file.
////////////////////////////////////////////////////////////////////
INLINE CollisionBox::
CollisionBox() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionBox::
CollisionBox(const CollisionBox &copy) :
  CollisionSolid(copy),
  _center(copy._center),
  _min(copy._min),
  _max(copy._max),
  _x(copy._x ),
  _y(copy._y ),
  _z(copy._z ),
  _radius(copy._radius )
{
  for(int v = 0; v < 8; v++)
    _vertex[v] = copy._vertex[v];
  for(int p = 0; p < 6; p++)
    _planes[p] = copy._planes[p];
  setup_box();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::flush_level
//       Access: Public, Static
//  Description: Flushes the PStatCollectors used during traversal.
////////////////////////////////////////////////////////////////////
INLINE void CollisionBox::
flush_level() {
  _volume_pcollector.flush_level();
  _test_pcollector.flush_level();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::set_center
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionBox::
set_center(const LPoint3 &center) {
  _center = center;
  mark_internal_bounds_stale();
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::set_center
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void CollisionBox::
set_center(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z) {
  set_center(LPoint3(x, y, z));
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::get_center
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const LPoint3 &CollisionBox::
get_center() const {
  return _center;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::get_radius
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat CollisionBox::
get_radius() const {
  return _radius;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::get_num_points
//       Access: Published
//  Description: Returns 8: the number of vertices of a rectangular solid.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL int CollisionBox::
get_num_points() const {
  return 8;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::get_point
//       Access: Published
//  Description: Returns the nth vertex of the OBB.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL LPoint3 CollisionBox::
get_point(int n) const {
  nassertr(n >= 0 && n < 8, LPoint3::zero());
  return _vertex[n];
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::get_point_aabb
//       Access: Published
//  Description: Returns the nth vertex of the Axis Aligned Bounding Box.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL LPoint3 CollisionBox::
get_point_aabb(int n) const {
  nassertr(n >= 0 && n < 8, LPoint3::zero());
  
  // We do some trickery assuming that _min and _max are consecutive
  // in memory.
  const LPoint3 *a = &_min;
  return LPoint3(a[(n>>2)&1][0], a[(n>>1)&1][1], a[(n)&1][2]);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::get_num_planes
//       Access: Published
//  Description: Returns 6: the number of faces of a rectangular solid.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL int CollisionBox::
get_num_planes() const {
  return 6;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::get_plane
//       Access: Published
//  Description: Returns the nth face of the rectangular solid.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL LPlane CollisionBox::
get_plane(int n) const {
  nassertr(n >= 0 && n < 6, LPlane());
  return _planes[n];
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::set_plane
//       Access: Published
//  Description: Creates the nth face of the rectangular solid.
////////////////////////////////////////////////////////////////////
INLINE_MATHUTIL LPlane CollisionBox::
set_plane(int n) const {
  nassertr(n >= 0 && n < 6, LPlane());
  return LPlane(get_point(plane_def[n][0]),
                get_point(plane_def[n][1]),
                get_point(plane_def[n][2]));
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::is_right
//       Access: Private, Static
//  Description: Returns true if the 2-d v1 is to the right of v2.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionBox::
is_right(const LVector2 &v1, const LVector2 &v2) {
  return (v1[0] * v2[1] - v1[1] * v2[0]) > 1.0e-6f;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::dist_to_line
//       Access: Private, Static
//  Description: Returns the linear distance of p to the line defined
//               by f and f+v, where v is a normalized vector.  The
//               result is negative if p is left of the line, positive
//               if it is right of the line.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat CollisionBox::
dist_to_line(const LPoint2 &p,
             const LPoint2 &f, const LVector2 &v) {
  LVector2 v1 = (p - f);
  return (v1[0] * v[1] - v1[1] * v[0]);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::to_2d
//       Access: Private
//  Description: Assuming the indicated point in 3-d space lies within
//               the polygon's plane, returns the corresponding point
//               in the polygon's 2-d definition space.
////////////////////////////////////////////////////////////////////
INLINE LPoint2 CollisionBox::
to_2d(const LVecBase3 &point3d, int plane) const {
  LPoint3 point = LPoint3(point3d) * _to_2d_mat[plane];
  return LPoint2(point[0], point[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::calc_to_3d_mat
//       Access: Private
//  Description: Fills the indicated matrix with the appropriate
//               rotation transform to move points from the 2-d plane
//               into the 3-d (X, 0, Z) plane.
////////////////////////////////////////////////////////////////////
INLINE void CollisionBox::
calc_to_3d_mat(LMatrix4 &to_3d_mat,int plane) const {
  // We have to be explicit about the coordinate system--we
  // specifically mean CS_zup_right, because that points the forward
  // vector down the Y axis and moves the coords in (X, 0, Z).  We
  // want this effect regardless of the user's coordinate system of
  // choice.

  // The up vector, on the other hand, is completely arbitrary.

  look_at(to_3d_mat, -get_plane(plane).get_normal(), 
          LVector3(0.0f, 0.0f, 1.0f), CS_zup_right);
  to_3d_mat.set_row(3, get_plane(plane).get_point());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::rederive_to_3d_mat
//       Access: Private
//  Description: Fills the indicated matrix with the appropriate
//               rotation transform to move points from the 2-d plane
//               into the 3-d (X, 0, Z) plane.
//
//               This is essentially similar to calc_to_3d_mat, except
//               that the matrix is rederived from whatever is stored
//               in _to_2d_mat, guaranteeing that it will match
//               whatever algorithm produced that one, even if it was
//               produced on a different machine with different
//               numerical precision.
////////////////////////////////////////////////////////////////////
INLINE void CollisionBox::
rederive_to_3d_mat(LMatrix4 &to_3d_mat, int plane) const {
  to_3d_mat.invert_from(_to_2d_mat[plane]);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::to_3d
//       Access: Private, Static
//  Description: Extrude the indicated point in the polygon's 2-d
//               definition space back into 3-d coordinates.
////////////////////////////////////////////////////////////////////
INLINE LPoint3 CollisionBox::
to_3d(const LVecBase2 &point2d, const LMatrix4 &to_3d_mat) {
  return LPoint3(point2d[0], 0.0f, point2d[1]) * to_3d_mat;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::PointDef::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CollisionBox::PointDef::
PointDef(const LPoint2 &p, const LVector2 &v) : _p(p), _v(v) {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::PointDef::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CollisionBox::PointDef::
PointDef(PN_stdfloat x, PN_stdfloat y) : _p(x, y), _v(0.0f, 0.0f) {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::PointDef::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CollisionBox::PointDef::
PointDef(const CollisionBox::PointDef &copy) : _p(copy._p), _v(copy._v) {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::PointDef::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CollisionBox::PointDef::
operator = (const CollisionBox::PointDef &copy) {
  _p = copy._p;
  _v = copy._v;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionBox::get_plane_points
//       Access: Public
//  Description: returns the points that form the nth plane
////////////////////////////////////////////////////////////////////
INLINE CollisionBox::Points CollisionBox::
get_plane_points(int n) {
  return _points[n];
}
