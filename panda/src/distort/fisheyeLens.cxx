// Filename: fisheyeLens.cxx
// Created by:  drose (12Dec01)
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

#include "fisheyeLens.h"
#include "deg_2_rad.h"

TypeHandle FisheyeLens::_type_handle;

// This is the focal-length constant for fisheye lenses.  The focal
// length of a fisheye lens relates to its fov by the equation:

//   w = Fd/k

// Where w is the width of the negative, F is the focal length, and d
// is the total field of view in degrees.

// k is chosen to make the focal lengths for a fisheye lens roughly
// correspond to the equivalent field of view for a conventional,
// perspective lens.  It was determined empirically by simple
// examination of a couple of actual lenses for 35mm film.  I don't
// know how well this extends to other lenses and other negative
// sizes.

static const PN_stdfloat fisheye_k = 60.0f;
// focal_length = film_size * fisheye_k / fov;


////////////////////////////////////////////////////////////////////
//     Function: FisheyeLens::make_copy
//       Access: Public, Virtual
//  Description: Allocates a new Lens just like this one.
////////////////////////////////////////////////////////////////////
PT(Lens) FisheyeLens::
make_copy() const {
  return new FisheyeLens(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: FisheyeLens::do_extrude
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
bool FisheyeLens::
do_extrude(const Lens::CData *lens_cdata, 
           const LPoint3 &point2d, LPoint3 &near_point, LPoint3 &far_point) const {
  // Undo the shifting from film offsets, etc.  This puts the point
  // into the range [-film_size/2, film_size/2] in x and y.
  LPoint3 f = point2d * do_get_film_mat_inv(lens_cdata);

  // First, get the vector from the center of the film to the point,
  // and normalize it.
  LVector2 v2(f[0], f[1]);

  LPoint3 v;

  PN_stdfloat r = v2.length();
  if (r == 0.0f) {
    // Special case: directly forward.
    v.set(0.0f, 1.0f, 0.0f);

  } else {
    v2 /= r;

    // Now get the point r units around the circle in the YZ plane.
    PN_stdfloat focal_length = do_get_focal_length(lens_cdata);
    PN_stdfloat angle = r * fisheye_k / focal_length;
    PN_stdfloat sinAngle, cosAngle;
    csincos(deg_2_rad(angle), &sinAngle, &cosAngle);

    LVector3 p(0.0, cosAngle, sinAngle);

    // And rotate this point around the Y axis.
    v.set(p[0]*v2[1] + p[2]*v2[0],
          p[1],
          p[2]*v2[1] - p[0]*v2[0]);
  }

  // And we'll need to account for the lens's rotations, etc. at the
  // end of the day.
  const LMatrix4 &lens_mat = do_get_lens_mat(lens_cdata);
  const LMatrix4 &proj_inv_mat = do_get_projection_mat_inv(lens_cdata);

  near_point = (v * do_get_near(lens_cdata)) * proj_inv_mat * lens_mat;
  far_point = (v * do_get_far(lens_cdata)) * proj_inv_mat * lens_mat;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FisheyeLens::do_extrude_vec
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
bool FisheyeLens::
do_extrude_vec(const Lens::CData *lens_cdata, const LPoint3 &point2d, LVector3 &vec) const {
  LPoint3 near_point, far_point;
  if (!do_extrude(lens_cdata, point2d, near_point, far_point)) {
    return false;
  }

  vec = far_point - near_point;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FisheyeLens::do_project
//       Access: Protected, Virtual
//  Description: Given a 3-d point in space, determine the 2-d point
//               this maps to, in the range (-1,1) in both dimensions,
//               where (0,0) is the center of the lens and
//               (-1,-1) is the lower-left corner.
//
//               Some lens types also set the z coordinate of the 2-d
//               point to a value in the range (-1, 1), where -1
//               represents a point on the near plane, and 1
//               represents a point on the far plane.
//
//               Returns true if the 3-d point is in front of the lens
//               and within the viewing frustum (in which case point2d
//               is filled in), or false otherwise.
////////////////////////////////////////////////////////////////////
bool FisheyeLens::
do_project(const Lens::CData *lens_cdata, const LPoint3 &point3d, LPoint3 &point2d) const {
  // First, account for any rotations, etc. on the lens.
  LVector3 v2 = point3d * do_get_lens_mat_inv(lens_cdata) * do_get_projection_mat(lens_cdata);

  // A fisheye lens projection has the property that the distance from
  // the center point to any other point on the projection is
  // proportional to the actual distance on the sphere along the great
  // circle.  Also, the angle to the point on the projection is equal
  // to the angle to the point on the sphere.

  // First, get the straight-line distance from the lens, and use it
  // to normalize the vector.
  PN_stdfloat dist = v2.length();
  v2 /= dist;

  // Now, project the point into the XZ plane and measure its angle
  // to the Z axis.  This is the same angle it will have to the
  // vertical axis on the film.
  LVector2 y(v2[0], v2[2]);
  y.normalize();

  if (y == LVector2(0.0f, 0.0f)) {
    // Special case.  This point is either directly ahead or directly
    // behind.
    point2d.set(0.0f, 0.0f, 
                (do_get_near(lens_cdata) - dist) / (do_get_far(lens_cdata) - do_get_near(lens_cdata)));
    return v2[1] >= 0.0f;
  }

  // Now bring the vector into the YZ plane by rotating about the Y
  // axis.
  LVector2 x(v2[1], v2[0]*y[0]+v2[2]*y[1]);

  // Now the angle of x to the forward vector represents the distance
  // along the great circle to the point.
  PN_stdfloat r = 90.0f - rad_2_deg(catan2(x[0], x[1]));

  PN_stdfloat focal_length = do_get_focal_length(lens_cdata);
  PN_stdfloat factor = r * focal_length / fisheye_k;

  // Compute the depth as a linear distance in the range 0 .. 1.
  PN_stdfloat z = (dist - do_get_near(lens_cdata)) / (do_get_far(lens_cdata) - do_get_near(lens_cdata));

  point2d.set
    (y[0] * factor,
     y[1] * factor,
     // Z is the distance scaled into the range -1 .. 1.
     2.0 * z - 1.0
     );

  // Now we have to transform the point according to the film
  // adjustments.
  point2d = point2d * do_get_film_mat(lens_cdata);

  return
    point2d[0] >= -1.0f && point2d[0] <= 1.0f && 
    point2d[1] >= -1.0f && point2d[1] <= 1.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: FisheyeLens::fov_to_film
//       Access: Protected, Virtual
//  Description: Given a field of view in degrees and a focal length,
//               compute the correspdonding width (or height) on the
//               film.  If horiz is true, this is in the horizontal
//               direction; otherwise, it is in the vertical direction
//               (some lenses behave differently in each direction).
////////////////////////////////////////////////////////////////////
PN_stdfloat FisheyeLens::
fov_to_film(PN_stdfloat fov, PN_stdfloat focal_length, bool) const {
  return focal_length * fov / fisheye_k;
}

////////////////////////////////////////////////////////////////////
//     Function: FisheyeLens::fov_to_focal_length
//       Access: Protected, Virtual
//  Description: Given a field of view in degrees and a width (or
//               height) on the film, compute the focal length of the
//               lens.  If horiz is true, this is in the horizontal
//               direction; otherwise, it is in the vertical direction
//               (some lenses behave differently in each direction).
////////////////////////////////////////////////////////////////////
PN_stdfloat FisheyeLens::
fov_to_focal_length(PN_stdfloat fov, PN_stdfloat film_size, bool) const {
  return film_size * fisheye_k / fov;
}

////////////////////////////////////////////////////////////////////
//     Function: FisheyeLens::film_to_fov
//       Access: Protected, Virtual
//  Description: Given a width (or height) on the film and a focal
//               length, compute the field of view in degrees.  If
//               horiz is true, this is in the horizontal direction;
//               otherwise, it is in the vertical direction (some
//               lenses behave differently in each direction).
////////////////////////////////////////////////////////////////////
PN_stdfloat FisheyeLens::
film_to_fov(PN_stdfloat film_size, PN_stdfloat focal_length, bool) const {
  return film_size * fisheye_k / focal_length;
}

