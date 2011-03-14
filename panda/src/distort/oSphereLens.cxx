// Filename: oSphereLens.cxx
// Created by:  drose (25Feb11)
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

#include "oSphereLens.h"
#include "deg_2_rad.h"

TypeHandle OSphereLens::_type_handle;

// This is the focal-length constant for fisheye lenses.  See
// fisheyeLens.cxx.
static const float ospherical_k = 60.0f;
// focal_length = film_size * ospherical_k / fov;


////////////////////////////////////////////////////////////////////
//     Function: OSphereLens::make_copy
//       Access: Public, Virtual
//  Description: Allocates a new Lens just like this one.
////////////////////////////////////////////////////////////////////
PT(Lens) OSphereLens::
make_copy() const {
  return new OSphereLens(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: OSphereLens::extrude_impl
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
bool OSphereLens::
extrude_impl(const LPoint3f &point2d, LPoint3f &near_point, LPoint3f &far_point) const {
  // Undo the shifting from film offsets, etc.  This puts the point
  // into the range [-film_size/2, film_size/2] in x and y.
  LPoint3f f = point2d * get_film_mat_inv();

  float focal_length = get_focal_length();
  float angle = f[0] * cylindrical_k / focal_length;
  float sinAngle, cosAngle;
  csincos(deg_2_rad(angle), &sinAngle, &cosAngle);

  // Define a unit vector that represents the vector corresponding to
  // this point.
  LPoint3f v(sinAngle, cosAngle, 0.0f);

  near_point = (v * get_near());
  far_point = (v * get_far());
  near_point[2] = f[1] / focal_length;
  far_point[2] = f[1] / focal_length;

  // And we'll need to account for the lens's rotations, etc. at the
  // end of the day.
  const LMatrix4f &lens_mat = get_lens_mat();

  near_point = near_point * lens_mat;
  far_point = far_point * lens_mat;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: OSphereLens::project_impl
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
bool OSphereLens::
project_impl(const LPoint3f &point3d, LPoint3f &point2d) const {
  // First, account for any rotations, etc. on the lens.
  LPoint3f p = point3d * get_lens_mat_inv();
  float dist = p.length();
  if (dist == 0.0f) {
    point2d.set(0.0f, 0.0f, 0.0f);
    return false;
  }

  LPoint3f v3 = p / dist;

  float focal_length = get_focal_length();

  // To compute the x position on the frame, we only need to consider
  // the angle of the vector about the Z axis.  Project the vector
  // into the XY plane to do this.
  LVector2f xy(v3[0], v3[1]);

  point2d.set
    (
     // The x position is the angle about the Z axis.
     rad_2_deg(catan2(xy[0], xy[1])) * focal_length / ospherical_k,
     // The y position is the Z height.
     // distance.
     p[2] * focal_length,
     // Z is the distance scaled into the range (1, -1).
     (get_near() - dist) / (get_far() - get_near())
     );

  // Now we have to transform the point according to the film
  // adjustments.
  point2d = point2d * get_film_mat();

  return
    point2d[0] >= -1.0f && point2d[0] <= 1.0f && 
    point2d[1] >= -1.0f && point2d[1] <= 1.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: OSphereLens::fov_to_film
//       Access: Protected, Virtual
//  Description: Given a field of view in degrees and a focal length,
//               compute the correspdonding width (or height) on the
//               film.  If horiz is true, this is in the horizontal
//               direction; otherwise, it is in the vertical direction
//               (some lenses behave differently in each direction).
////////////////////////////////////////////////////////////////////
float OSphereLens::
fov_to_film(float fov, float focal_length, bool) const {
  return focal_length * fov / ospherical_k;
}

////////////////////////////////////////////////////////////////////
//     Function: OSphereLens::fov_to_focal_length
//       Access: Protected, Virtual
//  Description: Given a field of view in degrees and a width (or
//               height) on the film, compute the focal length of the
//               lens.  If horiz is true, this is in the horizontal
//               direction; otherwise, it is in the vertical direction
//               (some lenses behave differently in each direction).
////////////////////////////////////////////////////////////////////
float OSphereLens::
fov_to_focal_length(float fov, float film_size, bool) const {
  return film_size * ospherical_k / fov;
}

////////////////////////////////////////////////////////////////////
//     Function: OSphereLens::film_to_fov
//       Access: Protected, Virtual
//  Description: Given a width (or height) on the film and a focal
//               length, compute the field of view in degrees.  If
//               horiz is true, this is in the horizontal direction;
//               otherwise, it is in the vertical direction (some
//               lenses behave differently in each direction).
////////////////////////////////////////////////////////////////////
float OSphereLens::
film_to_fov(float film_size, float focal_length, bool) const {
  return film_size * ospherical_k / focal_length;
}
