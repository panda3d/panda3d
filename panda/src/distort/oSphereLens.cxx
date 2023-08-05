/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file oSphereLens.cxx
 * @author drose
 * @date 2011-02-25
 */

#include "oSphereLens.h"
#include "deg_2_rad.h"

TypeHandle OSphereLens::_type_handle;

// This is the focal-length constant for fisheye lenses.  See fisheyeLens.cxx.
static const PN_stdfloat ospherical_k = 60.0f;
// focal_length = film_size * ospherical_k  fov;


/**
 * Allocates a new Lens just like this one.
 */
PT(Lens) OSphereLens::
make_copy() const {
  return new OSphereLens(*this);
}

/**
 * Given a 2-d point in the range (-1,1) in both dimensions, where (0,0) is
 * the center of the lens and (-1,-1) is the lower-left corner, compute the
 * corresponding vector in space that maps to this point, if such a vector can
 * be determined.  The vector is returned by indicating the points on the near
 * plane and far plane that both map to the indicated 2-d point.
 *
 * The z coordinate of the 2-d point is ignored.
 *
 * Returns true if the vector is defined, or false otherwise.
 */
bool OSphereLens::
do_extrude(const Lens::CData *lens_cdata,
           const LPoint3 &point2d, LPoint3 &near_point, LPoint3 &far_point) const {
  // Undo the shifting from film offsets, etc.  This puts the point into the
  // range [-film_size2, film_size2] in x and y.
  LPoint3 f = point2d * do_get_film_mat_inv(lens_cdata);

  PN_stdfloat focal_length = do_get_focal_length(lens_cdata);
  PN_stdfloat angle = f[0] * ospherical_k / focal_length;
  PN_stdfloat sinAngle, cosAngle;
  csincos(deg_2_rad(angle), &sinAngle, &cosAngle);

  // Define a unit vector that represents the vector corresponding to this
  // point.
  LPoint3 v(sinAngle, cosAngle, 0.0f);

  near_point = (v * do_get_near(lens_cdata));
  far_point = (v * do_get_far(lens_cdata));
  near_point[2] = f[1];
  far_point[2] = f[1];

  // And we'll need to account for the lens's rotations, etc.  at the end of
  // the day.
  const LMatrix4 &lens_mat = do_get_lens_mat(lens_cdata);
  const LMatrix4 &proj_inv_mat = do_get_projection_mat_inv(lens_cdata);

  near_point = near_point * proj_inv_mat * lens_mat;
  far_point = far_point * proj_inv_mat * lens_mat;
  return true;
}

/**
 * Given a 3-d point in space, determine the 2-d point this maps to, in the
 * range (-1,1) in both dimensions, where (0,0) is the center of the lens and
 * (-1,-1) is the lower-left corner.
 *
 * Some lens types also set the z coordinate of the 2-d point to a value in
 * the range (-1, 1), where -1 represents a point on the near plane, and 1
 * represents a point on the far plane.
 *
 * Returns true if the 3-d point is in front of the lens and within the
 * viewing frustum (in which case point2d is filled in), or false otherwise.
 */
bool OSphereLens::
do_project(const Lens::CData *lens_cdata, const LPoint3 &point3d, LPoint3 &point2d) const {
  // First, account for any rotations, etc.  on the lens.
  LPoint3 p = point3d * do_get_lens_mat_inv(lens_cdata) * do_get_projection_mat(lens_cdata);

  // To compute the x position on the frame, we only need to consider the
  // angle of the vector about the Z axis.  Project the vector into the XY
  // plane to do this.
  LVector2 xy(p[0], p[1]);

  PN_stdfloat dist = xy.length();
  if (dist == 0.0f) {
    point2d.set(0.0f, 0.0f, 0.0f);
    return false;
  }

  PN_stdfloat focal_length = do_get_focal_length(lens_cdata);
  // Compute the depth as a linear distance in the range 0 .. 1.
  PN_stdfloat z = (dist - do_get_near(lens_cdata)) / (do_get_far(lens_cdata) - do_get_near(lens_cdata));

  point2d.set
    (
     // The x position is the angle about the Z axis.
     rad_2_deg(catan2(xy[0], xy[1])) * focal_length / ospherical_k,
     // The y position is the Z height.
     p[2],
     // Z is the distance scaled into the range -1 .. 1.
     2.0 * z - 1.0
     );

  // Now we have to transform the point according to the film adjustments.
  point2d = point2d * do_get_film_mat(lens_cdata);

  return
    point2d[0] >= -1.0f && point2d[0] <= 1.0f &&
    point2d[1] >= -1.0f && point2d[1] <= 1.0f;
}

/**
 * Given a field of view in degrees and a focal length, compute the
 * correspdonding width (or height) on the film.  If horiz is true, this is in
 * the horizontal direction; otherwise, it is in the vertical direction (some
 * lenses behave differently in each direction).
 */
PN_stdfloat OSphereLens::
fov_to_film(PN_stdfloat fov, PN_stdfloat focal_length, bool) const {
  return focal_length * fov / ospherical_k;
}

/**
 * Given a field of view in degrees and a width (or height) on the film,
 * compute the focal length of the lens.  If horiz is true, this is in the
 * horizontal direction; otherwise, it is in the vertical direction (some
 * lenses behave differently in each direction).
 */
PN_stdfloat OSphereLens::
fov_to_focal_length(PN_stdfloat fov, PN_stdfloat film_size, bool) const {
  return film_size * ospherical_k / fov;
}

/**
 * Given a width (or height) on the film and a focal length, compute the field
 * of view in degrees.  If horiz is true, this is in the horizontal direction;
 * otherwise, it is in the vertical direction (some lenses behave differently
 * in each direction).
 */
PN_stdfloat OSphereLens::
film_to_fov(PN_stdfloat film_size, PN_stdfloat focal_length, bool) const {
  return film_size * ospherical_k / focal_length;
}
