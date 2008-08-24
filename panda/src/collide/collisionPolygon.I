// Filename: collisionPolygon.I
// Created by:  drose (25Apr00)
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
//     Function: CollisionPolygon::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionPolygon::
CollisionPolygon(const LVecBase3f &a, const LVecBase3f &b,
                 const LVecBase3f &c) {
  LPoint3f array[3];
  array[0] = a;
  array[1] = b;
  array[2] = c;
  setup_points(array, array + 3);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionPolygon::
CollisionPolygon(const LVecBase3f &a, const LVecBase3f &b,
                 const LVecBase3f &c, const LVecBase3f &d) {
  LPoint3f array[4];
  array[0] = a;
  array[1] = b;
  array[2] = c;
  array[3] = d;
  setup_points(array, array + 4);
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CollisionPolygon::
CollisionPolygon(const LPoint3f *begin, const LPoint3f *end) {
  setup_points(begin, end);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::Constructor
//       Access: Private
//  Description: Creates an invalid polygon.  Only used when reading
//               from a bam file.
////////////////////////////////////////////////////////////////////
INLINE CollisionPolygon::
CollisionPolygon() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::get_num_points
//       Access: Published
//  Description: Returns the number of vertices of the
//               CollisionPolygon.
////////////////////////////////////////////////////////////////////
INLINE int CollisionPolygon::
get_num_points() const {
  return _points.size();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::get_point
//       Access: Published
//  Description: Returns the nth vertex of the CollisionPolygon,
//               expressed in 3-D space.
////////////////////////////////////////////////////////////////////
INLINE LPoint3f CollisionPolygon::
get_point(int n) const {
  nassertr(n >= 0 && n < (int)_points.size(), LPoint3f::zero());
  LMatrix4f to_3d_mat;
  rederive_to_3d_mat(to_3d_mat);
  return to_3d(_points[n]._p, to_3d_mat);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::verify_points
//       Access: Published, Static
//  Description: Verifies that the indicated set of points will define
//               a valid CollisionPolygon: that is, at least three
//               non-collinear points, with no points repeated.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionPolygon::
verify_points(const LPoint3f &a, const LPoint3f &b,
              const LPoint3f &c) {
  LPoint3f array[3];
  array[0] = a;
  array[1] = b;
  array[2] = c;
  return verify_points(array, array + 3);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::verify_points
//       Access: Published, Static
//  Description: Verifies that the indicated set of points will define
//               a valid CollisionPolygon: that is, at least three
//               non-collinear points, with no points repeated.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionPolygon::
verify_points(const LPoint3f &a, const LPoint3f &b,
              const LPoint3f &c, const LPoint3f &d) {
  LPoint3f array[4];
  array[0] = a;
  array[1] = b;
  array[2] = c;
  array[3] = d;
  return verify_points(array, array + 4);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::flush_level
//       Access: Public, Static
//  Description: Flushes the PStatCollectors used during traversal.
////////////////////////////////////////////////////////////////////
INLINE void CollisionPolygon::
flush_level() {
  _volume_pcollector.flush_level();
  _test_pcollector.flush_level();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::is_right
//       Access: Private, Static
//  Description: Returns true if the 2-d v1 is to the right of v2.
////////////////////////////////////////////////////////////////////
INLINE bool CollisionPolygon::
is_right(const LVector2f &v1, const LVector2f &v2) {
  return (v1[0] * v2[1] - v1[1] * v2[0]) > 1.0e-6f;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::dist_to_line
//       Access: Private, Static
//  Description: Returns the linear distance of p to the line defined
//               by f and f+v, where v is a normalized vector.  The
//               result is negative if p is left of the line, positive
//               if it is right of the line.
////////////////////////////////////////////////////////////////////
INLINE float CollisionPolygon::
dist_to_line(const LPoint2f &p,
             const LPoint2f &f, const LVector2f &v) {
  LVector2f v1 = (p - f);
  return (v1[0] * v[1] - v1[1] * v[0]);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::to_2d
//       Access: Private
//  Description: Assuming the indicated point in 3-d space lies within
//               the polygon's plane, returns the corresponding point
//               in the polygon's 2-d definition space.
////////////////////////////////////////////////////////////////////
INLINE LPoint2f CollisionPolygon::
to_2d(const LVecBase3f &point3d) const {
  LPoint3f point = LPoint3f(point3d) * _to_2d_mat;
  return LPoint2f(point[0], point[2]);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::calc_to_3d_mat
//       Access: Private
//  Description: Fills the indicated matrix with the appropriate
//               rotation transform to move points from the 2-d plane
//               into the 3-d (X, 0, Z) plane.
////////////////////////////////////////////////////////////////////
INLINE void CollisionPolygon::
calc_to_3d_mat(LMatrix4f &to_3d_mat) const {
  // We have to be explicit about the coordinate system--we
  // specifically mean CS_zup_right, because that points the forward
  // vector down the Y axis and moves the coords in (X, 0, Z).  We
  // want this effect regardless of the user's coordinate system of
  // choice.

  // The up vector, on the other hand, is completely arbitrary.

  look_at(to_3d_mat, -get_plane().get_normal(), 
          LVector3f(0.0f, 0.0f, 1.0f), CS_zup_right);
  to_3d_mat.set_row(3, get_plane().get_point());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::rederive_to_3d_mat
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
INLINE void CollisionPolygon::
rederive_to_3d_mat(LMatrix4f &to_3d_mat) const {
  to_3d_mat.invert_from(_to_2d_mat);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::to_3d
//       Access: Private, Static
//  Description: Extrude the indicated point in the polygon's 2-d
//               definition space back into 3-d coordinates.
////////////////////////////////////////////////////////////////////
INLINE LPoint3f CollisionPolygon::
to_3d(const LVecBase2f &point2d, const LMatrix4f &to_3d_mat) {
  return LPoint3f(point2d[0], 0.0f, point2d[1]) * to_3d_mat;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::PointDef::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CollisionPolygon::PointDef::
PointDef(const LPoint2f &p, const LVector2f &v) : _p(p), _v(v) {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::PointDef::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CollisionPolygon::PointDef::
PointDef(float x, float y) : _p(x, y), _v(0.0f, 0.0f) {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::PointDef::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CollisionPolygon::PointDef::
PointDef(const CollisionPolygon::PointDef &copy) : _p(copy._p), _v(copy._v) {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionPolygon::PointDef::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void CollisionPolygon::PointDef::
operator = (const CollisionPolygon::PointDef &copy) {
  _p = copy._p;
  _v = copy._v;
}
