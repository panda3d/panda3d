// Filename: plane_src.cxx
// Created by:  drose (03Apr01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: Plane::get_reflection_mat
//       Access: Published
//  Description: This computes a transform matrix that performs the
//               perspective transform defined by the frustum,
//               accordinate to the indicated coordinate system.
////////////////////////////////////////////////////////////////////
FLOATNAME(LMatrix4) FLOATNAME(Plane)::
get_reflection_mat(void) const {
  FLOATTYPE aa = _a * _a; FLOATTYPE ab = _a * _b; FLOATTYPE ac = _a * _c;
  FLOATTYPE ad = _a * _d;
  FLOATTYPE bb = _b * _b; FLOATTYPE bc = _b * _c; FLOATTYPE bd = _b * _d;
  FLOATTYPE cc = _c * _c; FLOATTYPE cd = _c * _d;

  return FLOATNAME(LMatrix4)(  1-2*aa,  -2*ab,  -2*ac,     0,
                              -2*ab, 1-2*bb,  -2*bc,     0,
                              -2*ac,  -2*bc, 1-2*cc,     0,
                              -2*ad,  -2*bd,  -2*cd,     1  );
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::get_point
//       Access: Published
//  Description: Returns an arbitrary point in the plane.  This can be
//               used along with the normal returned by get_normal()
//               to reconstruct the plane.
////////////////////////////////////////////////////////////////////
FLOATNAME(LPoint3) FLOATNAME(Plane)::
get_point() const {
  // Choose the denominator based on the largest axis in the normal.
  if (cabs(_a) >= cabs(_b) && cabs(_a) >= cabs(_c)) {
    nassertr(_a != 0.0f, FLOATNAME(LPoint3)(0.0f, 0.0f, 0.0f));
    return FLOATNAME(LPoint3)(-_d / _a, 0.0f, 0.0f);
  } else if (cabs(_b) >= cabs(_c)) {
    nassertr(_b != 0.0f, FLOATNAME(LPoint3)(0.0f, 0.0f, 0.0f));
    return FLOATNAME(LPoint3)(0.0f, -_d / _b, 0.0f);
  } else {
    nassertr(_c != 0.0f, FLOATNAME(LPoint3)(0.0f, 0.0f, 0.0f));
    return FLOATNAME(LPoint3)(0.0f, 0.0f, -_d / _c);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: Plane::intersects_plane
//       Access: Published
//  Description: Returns true if the two planes intersect, false if
//               they do not.  If they do intersect, then from and
//               delta are filled in with the parametric
//               representation of the line of intersection: that is,
//               from is a point on that line, and delta is a vector
//               showing the direction of the line.
////////////////////////////////////////////////////////////////////
bool FLOATNAME(Plane)::
intersects_plane(FLOATNAME(LPoint3) &from,
                 FLOATNAME(LVector3) &delta,
                 const FLOATNAME(Plane) &other) const {
  FLOATNAME(LVector3) n1 = get_normal();
  FLOATNAME(LVector3) n2 = other.get_normal();

  // The delta will be the cross product of the planes' normals.
  delta = cross(n1, n2);

  // If the delta came out to zero, the planes were parallel and do
  // not intersect.
  if (delta.almost_equal(FLOATNAME(LVector3)::zero())) {
    return false;
  }

  FLOATTYPE n1n1 = dot(n1, n1);
  FLOATTYPE n2n2 = dot(n2, n2);
  FLOATTYPE n1n2 = dot(n1, n2);
 
  FLOATTYPE determinant_inv = 1.0f / (n1n1 * n2n2 - n1n2 * n1n2);
  FLOATTYPE c1 = (other._d * n1n2 - _d * n2n2) * determinant_inv;
  FLOATTYPE c2 = (_d * n1n2 - other._d * n1n1) * determinant_inv;
  from = n1 * c1 + n2 * c2;

  return true;
}
