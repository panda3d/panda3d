// Filename: cylindricalLens.cxx
// Created by:  drose (12Dec01)
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

#include "cylindricalLens.h"
#include "deg_2_rad.h"

TypeHandle CylindricalLens::_type_handle;

// This is the focal-length constant for fisheye lenses.  See
// fisheyeLens.cxx.
static const float cylindrical_k = 60.0f;
// focal_length = film_size * cylindrical_k / fov;


////////////////////////////////////////////////////////////////////
//     Function: CylindricalLens::make_copy
//       Access: Public, Virtual
//  Description: Allocates a new Lens just like this one.
////////////////////////////////////////////////////////////////////
PT(Lens) CylindricalLens::
make_copy() const {
  return new CylindricalLens(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CylindricalLens::extrude_impl
//       Access: Protected, Virtual
//  Description: Given a 2-d point in the range (-1,1) in both
//               dimensions, where (0,0) is the center of the
//               lens and (-1,-1) is the lower-left corner,
//               compute the corresponding vector in space that maps
//               to this point, if such a vector can be determined.
//               The vector is returned by indicating the points on
//               the near plane and far plane that both map to the
//               indicated 2-d point.
//
//               The z coordinate of the 2-d point is ignored.
//
//               Returns true if the vector is defined, or false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool CylindricalLens::
extrude_impl(const LPoint3f &point2d, LPoint3f &near_point, LPoint3f &far_point) const {
  // Undo the shifting from film offsets, etc.  This puts the point
  // into the range [-film_size/2, film_size/2] in x and y.
  LPoint3f f = point2d * get_film_mat_inv();

  float focal_length = get_focal_length();
  float angle = f[0] * cylindrical_k / focal_length;
  float sinAngle, cosAngle;
  csincos(deg_2_rad(angle), &sinAngle, &cosAngle);

  // Define a unit vector (well, a unit vector in the XY plane, at
  // least) that represents the vector corresponding to this point.
  LPoint3f v(sinAngle, cosAngle, f[1] / focal_length);

  // And we'll need to account for the lens's rotations, etc. at the
  // end of the day.
  const LMatrix4f &lens_mat = get_lens_mat();

  near_point = (v * get_near()) * lens_mat;
  far_point = (v * get_far()) * lens_mat;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CylindricalLens::extrude_vec_impl
//       Access: Protected, Virtual
//  Description: Given a 2-d point in the range (-1,1) in both
//               dimensions, where (0,0) is the center of the
//               lens and (-1,-1) is the lower-left corner,
//               compute the vector that corresponds to the view
//               direction.  This will be parallel to the normal on
//               the surface (the far plane) corresponding to the lens
//               shape at this point.
//
//               See the comment block on Lens::extrude_vec_impl() for
//               a more in-depth comment on the meaning of this
//               vector.
//
//               The z coordinate of the 2-d point is ignored.
//
//               Returns true if the vector is defined, or false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool CylindricalLens::
extrude_vec_impl(const LPoint3f &point2d, LVector3f &vec) const {
  // Undo the shifting from film offsets, etc.  This puts the point
  // into the range [-film_size/2, film_size/2] in x and y.
  LPoint3f f = point2d * get_film_mat_inv();

  float focal_length = get_focal_length();
  float angle = f[0] * cylindrical_k / focal_length;
  float sinAngle, cosAngle;
  csincos(deg_2_rad(angle), &sinAngle, &cosAngle);

  vec = LVector3f(sinAngle, cosAngle, 0.0f) * get_lens_mat();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CylindricalLens::project_impl
//       Access: Protected, Virtual
//  Description: Given a 3-d point in space, determine the 2-d point
//               this maps to, in the range (-1,1) in both dimensions,
//               where (0,0) is the center of the lens and
//               (-1,-1) is the lower-left corner.
//
//               Some lens types also set the z coordinate of the 2-d
//               point to a value in the range (-1, 1), where 1
//               represents a point on the near plane, and -1
//               represents a point on the far plane.
//
//               Returns true if the 3-d point is in front of the lens
//               and within the viewing frustum (in which case point2d
//               is filled in), or false otherwise.
////////////////////////////////////////////////////////////////////
bool CylindricalLens::
project_impl(const LPoint3f &point3d, LPoint3f &point2d) const {
  // First, account for any rotations, etc. on the lens.
  LPoint3f p = point3d * get_lens_mat_inv();

  // To compute the x position on the frame, we only need to consider
  // the angle of the vector about the Z axis.  Project the vector
  // into the XY plane to do this.
  LVector2f xy(p[0], p[1]);

  // The perspective distance is the length of this vector in the XY
  // plane.
  float pdist = xy.length();
  if (pdist == 0.0f) {
    point2d.set(0.0f, 0.0f, 0.0f);
    return false;
  }

  float focal_length = get_focal_length();

  point2d.set
    (
     // The x position is the angle about the Z axis.
     rad_2_deg(catan2(xy[0], xy[1])) * focal_length / cylindrical_k,
     // The y position is the Z height divided by the perspective
     // distance.
     p[2] * focal_length / pdist,
     // Z is the perspective distance scaled into the range (1, -1).
     (get_near() - pdist) / (get_far() - get_near())
     );

  // Now we have to transform the point according to the film
  // adjustments.
  point2d = point2d * get_film_mat();

  return
    point2d[0] >= -1.0f && point2d[0] <= 1.0f && 
    point2d[1] >= -1.0f && point2d[1] <= 1.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: CylindricalLens::fov_to_film
//       Access: Protected, Virtual
//  Description: Given a field of view in degrees and a focal length,
//               compute the correspdonding width (or height) on the
//               film.  If horiz is true, this is in the horizontal
//               direction; otherwise, it is in the vertical direction
//               (some lenses behave differently in each direction).
////////////////////////////////////////////////////////////////////
float CylindricalLens::
fov_to_film(float fov, float focal_length, bool horiz) const {
  if (horiz) {
    return focal_length * fov / cylindrical_k;
  } else {
    return (ctan(deg_2_rad(fov * 0.5f)) * focal_length) * 2.0f;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CylindricalLens::fov_to_focal_length
//       Access: Protected, Virtual
//  Description: Given a field of view in degrees and a width (or
//               height) on the film, compute the focal length of the
//               lens.  If horiz is true, this is in the horizontal
//               direction; otherwise, it is in the vertical direction
//               (some lenses behave differently in each direction).
////////////////////////////////////////////////////////////////////
float CylindricalLens::
fov_to_focal_length(float fov, float film_size, bool horiz) const {
  if (horiz) {
    return film_size * cylindrical_k / fov;
  } else {
    return film_size * 0.5f / ctan(deg_2_rad(fov * 0.5f));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CylindricalLens::film_to_fov
//       Access: Protected, Virtual
//  Description: Given a width (or height) on the film and a focal
//               length, compute the field of view in degrees.  If
//               horiz is true, this is in the horizontal direction;
//               otherwise, it is in the vertical direction (some
//               lenses behave differently in each direction).
////////////////////////////////////////////////////////////////////
float CylindricalLens::
film_to_fov(float film_size, float focal_length, bool horiz) const {
  if (horiz) {
    return film_size * cylindrical_k / focal_length;
  } else {
    return rad_2_deg(catan(film_size * 0.5f / focal_length)) * 2.0f;
  }
}
