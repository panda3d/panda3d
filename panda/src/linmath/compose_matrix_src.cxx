/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file compose_matrix_src.cxx
 * @author drose
 * @date 1999-01-27
 */

/**
 * Extracts the rotation about the x, y, and z axes from the given hpr & scale
 * matrix.  Adjusts the matrix to eliminate the rotation.
 *
 * This function assumes the matrix is stored in a right-handed Y-up
 * coordinate system.
 */
static void
unwind_yup_rotation_old_hpr(FLOATNAME(LMatrix3) &mat, FLOATNAME(LVecBase3) &hpr) {
  TAU_PROFILE("void unwind_yup_rotation_old_hpr(LMatrix3 &, LVecBase3 &)", " ", TAU_USER);

  // Extract the axes from the matrix.
  FLOATNAME(LVector3) x, y, z;
  mat.get_row(x,0);
  mat.get_row(y,1);
  mat.get_row(z,2);

  // Project X onto the XY plane.
  FLOATNAME(LVector2) xy(x[0], x[1]);
  xy = normalize(xy);

  // Compute the rotation about the +Z (back) axis.  This is roll.
  FLOATTYPE roll = rad_2_deg(((FLOATTYPE)catan2(xy[1], xy[0])));

  // Unwind the roll from the axes, and continue.
  FLOATNAME(LMatrix3) rot_z;
  rot_z.set_rotate_mat_normaxis(-roll, FLOATNAME(LVector3)(0.0f, 0.0f, 1.0f),
                                CS_yup_right);

  x = x * rot_z;
  y = y * rot_z;
  z = z * rot_z;

  // Project the rotated X into the XZ plane.
  FLOATNAME(LVector2) xz(x[0], x[2]);
  xz = normalize(xz);

  // Compute the rotation about the +Y (up) axis.  This is yaw, or "heading".
  FLOATTYPE heading = rad_2_deg(((FLOATTYPE)-catan2(xz[1], xz[0])));

  // Unwind the heading, and continue.
  FLOATNAME(LMatrix3) rot_y;
  rot_y.set_rotate_mat_normaxis(-heading, FLOATNAME(LVector3)(0.0f, 1.0f, 0.0f),
                                CS_yup_right);

  x = x * rot_y;
  y = y * rot_y;
  z = z * rot_y;

  // Project the rotated Z into the YZ plane.
  FLOATNAME(LVector2) yz(z[1], z[2]);
  yz = normalize(yz);

  // Compute the rotation about the +X (right) axis.  This is pitch.
  FLOATTYPE pitch = rad_2_deg(((FLOATTYPE)-catan2(yz[0], yz[1])));

  // Unwind the pitch.
  FLOATNAME(LMatrix3) rot_x;
  rot_x.set_rotate_mat_normaxis(-pitch, FLOATNAME(LVector3)(1.0f, 0.0f, 0.0f),
                                CS_yup_right);

  x = x * rot_x;
  y = y * rot_x;
  z = z * rot_x;

  // Reset the matrix to reflect the unwinding.
  mat.set_row(0, x);
  mat.set_row(1, y);
  mat.set_row(2, z);

  // Return the three rotation components.
  hpr[0] = heading;
  hpr[1] = pitch;
  hpr[2] = roll;
}

/**
 * Extracts the rotation about the x, y, and z axes from the given hpr & scale
 * matrix.  Adjusts the matrix to eliminate the rotation.
 *
 * This function assumes the matrix is stored in a right-handed Z-up
 * coordinate system.
 */
static void
unwind_zup_rotation_old_hpr(FLOATNAME(LMatrix3) &mat, FLOATNAME(LVecBase3) &hpr) {
  TAU_PROFILE("void unwind_zup_rotation_old_hpr(LMatrix3 &, LVecBase3 &)", " ", TAU_USER);
  // Extract the axes from the matrix.
  FLOATNAME(LVector3) x, y, z;
  mat.get_row(x,0);
  mat.get_row(y,1);
  mat.get_row(z,2);


  // Project X into the XZ plane.
  FLOATNAME(LVector2) xz(x[0], x[2]);
  xz = normalize(xz);

  // Compute the rotation about the -Y (back) axis.  This is roll.
  FLOATTYPE roll = rad_2_deg(((FLOATTYPE)catan2(xz[1], xz[0])));

  if (y[1] < 0.0f) {
    if (roll < 0.0f) {
      roll += 180.0;
    } else {
      roll -= 180.0;
    }
  }

  // Unwind the roll from the axes, and continue.
  FLOATNAME(LMatrix3) rot_y;
  rot_y.set_rotate_mat_normaxis(roll, FLOATNAME(LVector3)(0.0f, 1.0f, 0.0f),
                                CS_zup_right);

  x = x * rot_y;
  y = y * rot_y;
  z = z * rot_y;

  // Project the rotated X into the XY plane.
  FLOATNAME(LVector2) xy(x[0], x[1]);
  xy = normalize(xy);

  // Compute the rotation about the +Z (up) axis.  This is yaw, or "heading".
  FLOATTYPE heading = rad_2_deg(((FLOATTYPE)catan2(xy[1], xy[0])));

  // Unwind the heading, and continue.
  FLOATNAME(LMatrix3) rot_z;
  rot_z.set_rotate_mat_normaxis(-heading, FLOATNAME(LVector3)(0.0f, 0.0f, 1.0f),
                                CS_zup_right);

  x = x * rot_z;
  y = y * rot_z;
  z = z * rot_z;

  // Project the rotated Y into the YZ plane.
  FLOATNAME(LVector2) yz(y[1], y[2]);
  yz = normalize(yz);

  // Compute the rotation about the +X (right) axis.  This is pitch.
  FLOATTYPE pitch = rad_2_deg(((FLOATTYPE)catan2(yz[1], yz[0])));

  // Unwind the pitch.
  FLOATNAME(LMatrix3) rot_x;
  rot_x.set_rotate_mat_normaxis(-pitch, FLOATNAME(LVector3)(1.0f, 0.0f, 0.0f),
                                CS_zup_right);

  x = x * rot_x;
  y = y * rot_x;
  z = z * rot_x;

  // Reset the matrix to reflect the unwinding.
  mat.set_row(0, x);
  mat.set_row(1, y);
  mat.set_row(2, z);

  // Return the three rotation components.
  hpr[0] = heading;
  hpr[1] = pitch;
  hpr[2] = roll;
}

/**
 * Extracts out the components of a 3x3 rotation matrix.  Returns true if
 * successful, or false if there was an error.  Since a 3x3 matrix always
 * contains an affine transform, this should succeed in the normal case;
 * singular transforms are not treated as an error.
 */
bool
decompose_matrix_old_hpr(const FLOATNAME(LMatrix3) &mat,
                         FLOATNAME(LVecBase3) &scale,
                         FLOATNAME(LVecBase3) &shear,
                         FLOATNAME(LVecBase3) &hpr,
                         CoordinateSystem cs) {
  TAU_PROFILE("bool decompose_matrix_old_hpr(LMatrix3 &, LVecBase3 &, LVecBase3 &, LVecBase3 &)", " ", TAU_USER);
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  if (linmath_cat.is_debug()) {
    linmath_cat.debug()
      << "decomposing " << mat << " via cs " << cs << "\n";
  }

  // Extract the rotation and scale, according to the coordinate system of
  // choice.

  FLOATNAME(LMatrix3) new_mat(mat);

  switch (cs) {
  case CS_zup_right:
    {
      unwind_zup_rotation_old_hpr(new_mat, hpr);
    }
    break;

  case CS_yup_right:
    {
      unwind_yup_rotation_old_hpr(new_mat, hpr);
    }
    break;

  case CS_zup_left:
    {
      new_mat._m(0, 2) = -new_mat._m(0, 2);
      new_mat._m(1, 2) = -new_mat._m(1, 2);
      new_mat._m(2, 0) = -new_mat._m(2, 0);
      new_mat._m(2, 1) = -new_mat._m(2, 1);
      /*
        FLOATNAME(LMatrix3) lm(mat(0, 0), mat(0, 1), -mat(0, 2),
        mat(1, 0), mat(1, 1), -mat(1, 2),
        -mat(2, 0), -mat(2, 1), mat(2, 2));
      */
      unwind_zup_rotation_old_hpr(new_mat, hpr);
      hpr[0] = -hpr[0];
      hpr[2] = -hpr[2];
    }
    break;

  case CS_yup_left:
    {
      new_mat._m(0, 2) = -new_mat._m(0, 2);
      new_mat._m(1, 2) = -new_mat._m(1, 2);
      new_mat._m(2, 0) = -new_mat._m(2, 0);
      new_mat._m(2, 1) = -new_mat._m(2, 1);
      /*
        FLOATNAME(LMatrix3) lm(mat(0, 0), mat(0, 1), -mat(0, 2),
        mat(1, 0), mat(1, 1), -mat(1, 2),
        -mat(2, 0), -mat(2, 1), mat(2, 2));
      */
      unwind_yup_rotation_old_hpr(new_mat, hpr);
    }
    break;

  default:
    linmath_cat.error()
      << "Unexpected coordinate system: " << (int)cs << "\n";
    return false;
  }

  if (linmath_cat.is_debug()) {
    linmath_cat.debug()
      << "after unwind, mat is " << new_mat << "\n";
  }

  scale.set(new_mat(0, 0), new_mat(1, 1), new_mat(2, 2));

  // Normalize the scale out of the shear components, and return the shear.
  if (scale[0] != 0.0) {
    new_mat(0, 1) /= scale[0];
    new_mat(0, 2) /= scale[0];
  }
  if (scale[1] != 0.0) {
    new_mat(1, 0) /= scale[1];
    new_mat(1, 2) /= scale[1];
  }
  if (scale[2] != 0.0) {
    new_mat(2, 0) /= scale[2];
    new_mat(2, 1) /= scale[2];
  }

  shear.set(new_mat(0, 1) + new_mat(1, 0),
            new_mat(2, 0) + new_mat(0, 2),
            new_mat(2, 1) + new_mat(1, 2));

  return true;
}

/**
 * Computes the 3x3 matrix from scale, shear, and rotation.
 */
void
compose_matrix(FLOATNAME(LMatrix3) &mat,
               const FLOATNAME(LVecBase3) &scale,
               const FLOATNAME(LVecBase3) &shear,
               const FLOATNAME(LVecBase3) &hpr,
               CoordinateSystem cs) {
  TAU_PROFILE("void compose_matrix(LMatrix3 &, const LVecBase3 &, const LVecBase3 &, const LVecBase3 &)", " ", TAU_USER);
  mat.set_scale_shear_mat(scale, shear, cs);
  if (!IS_NEARLY_ZERO(hpr[2])) {
    FLOATNAME(LMatrix3) r;
    r.set_rotate_mat_normaxis(hpr[2], FLOATNAME(LVector3)::forward(cs), cs);
    mat *= r;
  }
  if (!IS_NEARLY_ZERO(hpr[1])) {
    FLOATNAME(LMatrix3) r;
    r.set_rotate_mat_normaxis(hpr[1], FLOATNAME(LVector3)::right(cs), cs);
    mat *= r;
  }
  if (!IS_NEARLY_ZERO(hpr[0])) {
    FLOATNAME(LMatrix3) r;
    r.set_rotate_mat_normaxis(hpr[0], FLOATNAME(LVector3)::up(cs), cs);
    mat *= r;
  }
}

/**
 * Extracts the rotation about the x, y, and z axes from the given hpr & scale
 * matrix.  Adjusts the matrix to eliminate the rotation.
 *
 * This function assumes the matrix is stored in a right-handed Y-up
 * coordinate system.
 */
static void
unwind_yup_rotation(FLOATNAME(LMatrix3) &mat, FLOATNAME(LVecBase3) &hpr) {
  TAU_PROFILE("void unwind_yup_rotation(LMatrix3 &, LVecBase3 &)", " ", TAU_USER);

  // Extract the axes from the matrix.
  FLOATNAME(LVector3) x, y, z;
  mat.get_row(x,0);
  mat.get_row(y,1);
  mat.get_row(z,2);

  // Project Z into the XZ plane.
  FLOATTYPE heading = 0;
  FLOATNAME(LVector2) xz(z[0], z[2]);
  if (xz.normalize()) {
    // Compute the rotation about the +Y (up) axis.  This is yaw, or "heading".
    heading = catan2(xz[0], xz[1]);

    // Unwind the heading, and continue.
    FLOATNAME(LMatrix3) rot_y;
    rot_y._m(0, 0) = xz[1];
    rot_y._m(0, 1) = 0;
    rot_y._m(0, 2) = xz[0];

    rot_y._m(1, 0) = 0;
    rot_y._m(1, 1) = 1;
    rot_y._m(1, 2) = 0;

    rot_y._m(2, 0) = -xz[0];
    rot_y._m(2, 1) = 0;
    rot_y._m(2, 2) = xz[1];

    x = x * rot_y;
    y = y * rot_y;
    z = z * rot_y;
  }

  // Project the rotated Z into the YZ plane.
  FLOATTYPE pitch = 0;
  FLOATNAME(LVector2) yz(z[1], z[2]);
  if (yz.normalize()) {
    // Compute the rotation about the +X (right) axis.  This is pitch.
    pitch = -catan2(yz[0], yz[1]);

    // Unwind the pitch.
    FLOATNAME(LMatrix3) rot_x;
    rot_x._m(0, 0) = 1;
    rot_x._m(0, 1) = 0;
    rot_x._m(0, 2) = 0;

    rot_x._m(1, 0) = 0;
    rot_x._m(1, 1) = yz[1];
    rot_x._m(1, 2) = yz[0];

    rot_x._m(2, 0) = 0;
    rot_x._m(2, 1) = -yz[0];
    rot_x._m(2, 2) = yz[1];

    x = x * rot_x;
    y = y * rot_x;
    z = z * rot_x;
  }

  // Project the rotated X onto the XY plane.
  FLOATTYPE roll = 0;
  FLOATNAME(LVector2) xy(x[0], x[1]);
  if (xy.normalize()) {
    // Compute the rotation about the +Z (back) axis.  This is roll.
    roll = -catan2(xy[1], xy[0]);

    // Unwind the roll from the axes, and continue.
    FLOATNAME(LMatrix3) rot_z;
    rot_z._m(0, 0) = xy[0];
    rot_z._m(0, 1) = -xy[1];
    rot_z._m(0, 2) = 0;

    rot_z._m(1, 0) = xy[1];
    rot_z._m(1, 1) = xy[0];
    rot_z._m(1, 2) = 0;

    rot_z._m(2, 0) = 0;
    rot_z._m(2, 1) = 0;
    rot_z._m(2, 2) = 1;

    x = x * rot_z;
    y = y * rot_z;
    z = z * rot_z;
  }

  // Reset the matrix to reflect the unwinding.
  mat.set_row(0, x);
  mat.set_row(1, y);
  mat.set_row(2, z);

  // Return the three rotation components.
  hpr[0] = rad_2_deg(heading);
  hpr[1] = rad_2_deg(pitch);
  hpr[2] = rad_2_deg(roll);
}

/**
 * Extracts the rotation about the x, y, and z axes from the given hpr & scale
 * matrix.  Adjusts the matrix to eliminate the rotation.
 *
 * This function assumes the matrix is stored in a right-handed Z-up
 * coordinate system.
 */
static void
unwind_zup_rotation(FLOATNAME(LMatrix3) &mat, FLOATNAME(LVecBase3) &hpr) {
  TAU_PROFILE("void unwind_zup_rotation(LMatrix3 &, LVecBase3 &)", " ", TAU_USER);
  // Extract the axes from the matrix.
  FLOATNAME(LVector3) x, y, z;
  mat.get_row(x,0);
  mat.get_row(y,1);
  mat.get_row(z,2);

  // Project Y into the XY plane.
  FLOATTYPE heading = 0;
  FLOATNAME(LVector2) xy(y[0], y[1]);
  if (xy.normalize()) {
    // Compute the rotation about the +Z (up) axis.  This is yaw, or "heading".
    heading = -catan2(xy[0], xy[1]);

    // Unwind the heading, and continue.
    FLOATNAME(LMatrix3) rot_z;
    rot_z._m(0, 0) = xy[1];
    rot_z._m(0, 1) = xy[0];
    rot_z._m(0, 2) = 0;

    rot_z._m(1, 0) = -xy[0];
    rot_z._m(1, 1) = xy[1];
    rot_z._m(1, 2) = 0;

    rot_z._m(2, 0) = 0;
    rot_z._m(2, 1) = 0;
    rot_z._m(2, 2) = 1;

    x = x * rot_z;
    y = y * rot_z;
    z = z * rot_z;
  }

  // Project the rotated Y into the YZ plane.
  FLOATTYPE pitch = 0;
  FLOATNAME(LVector2) yz(y[1], y[2]);
  if (yz.normalize()) {
    // Compute the rotation about the +X (right) axis.  This is pitch.
    pitch = catan2(yz[1], yz[0]);

    // Unwind the pitch.
    FLOATNAME(LMatrix3) rot_x;
    rot_x._m(0, 0) = 1;
    rot_x._m(0, 1) = 0;
    rot_x._m(0, 2) = 0;

    rot_x._m(1, 0) = 0;
    rot_x._m(1, 1) = yz[0];
    rot_x._m(1, 2) = -yz[1];

    rot_x._m(2, 0) = 0;
    rot_x._m(2, 1) = yz[1];
    rot_x._m(2, 2) = yz[0];

    x = x * rot_x;
    y = y * rot_x;
    z = z * rot_x;
  }

  // Project X into the XZ plane.
  FLOATTYPE roll = 0;
  FLOATNAME(LVector2) xz(x[0], x[2]);
  if (xz.normalize()) {
  // Compute the rotation about the -Y (back) axis.  This is roll.
    roll = -catan2(xz[1], xz[0]);

    // Unwind the roll from the axes, and continue.
    FLOATNAME(LMatrix3) rot_y;
    rot_y._m(0, 0) = xz[0];
    rot_y._m(0, 1) = 0;
    rot_y._m(0, 2) = -xz[1];

    rot_y._m(1, 0) = 0;
    rot_y._m(1, 1) = 1;
    rot_y._m(1, 2) = 0;

    rot_y._m(2, 0) = xz[1];
    rot_y._m(2, 1) = 0;
    rot_y._m(2, 2) = xz[0];

    x = x * rot_y;
    y = y * rot_y;
    z = z * rot_y;
  }

  // Reset the matrix to reflect the unwinding.
  mat.set_row(0, x);
  mat.set_row(1, y);
  mat.set_row(2, z);

  // Return the three rotation components.
  hpr[0] = rad_2_deg(heading);
  hpr[1] = rad_2_deg(pitch);
  hpr[2] = rad_2_deg(roll);
}

/**
 * Extracts out the components of a 3x3 rotation matrix.  Returns true if
 * successful, or false if there was an error.  Since a 3x3 matrix always
 * contains an affine transform, this should succeed in the normal case;
 * singular transforms are not treated as an error.
 */
bool
decompose_matrix(const FLOATNAME(LMatrix3) &mat,
                 FLOATNAME(LVecBase3) &scale,
                 FLOATNAME(LVecBase3) &shear,
                 FLOATNAME(LVecBase3) &hpr,
                 CoordinateSystem cs) {
  TAU_PROFILE("bool decompose_matrix(LMatrix3 &, LVecBase3 &, LVecBase3 &, LVecBase3 &)", " ", TAU_USER);
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

#ifdef _DEBUG
  if (linmath_cat.is_debug()) {
    linmath_cat.debug()
      << "decomposing " << mat << " via cs " << cs << "\n";
  }
#endif

  // Extract the rotation and scale, according to the coordinate system of
  // choice.

  FLOATNAME(LMatrix3) new_mat(mat);

  switch (cs) {
  case CS_zup_right:
    {
      unwind_zup_rotation(new_mat, hpr);
    }
    break;

  case CS_yup_right:
    {
      unwind_yup_rotation(new_mat, hpr);
    }
    break;

  case CS_zup_left:
    {
      new_mat._m(0, 2) = -new_mat._m(0, 2);
      new_mat._m(1, 2) = -new_mat._m(1, 2);
      new_mat._m(2, 0) = -new_mat._m(2, 0);
      new_mat._m(2, 1) = -new_mat._m(2, 1);
      /*
        FLOATNAME(LMatrix3) lm(mat(0, 0), mat(0, 1), -mat(0, 2),
        mat(1, 0), mat(1, 1), -mat(1, 2),
        -mat(2, 0), -mat(2, 1), mat(2, 2));
      */
      unwind_zup_rotation(new_mat, hpr);
      hpr[0] = -hpr[0];
      hpr[2] = -hpr[2];
    }
    break;

  case CS_yup_left:
    {
      new_mat._m(0, 2) = -new_mat._m(0, 2);
      new_mat._m(1, 2) = -new_mat._m(1, 2);
      new_mat._m(2, 0) = -new_mat._m(2, 0);
      new_mat._m(2, 1) = -new_mat._m(2, 1);
      /*
        FLOATNAME(LMatrix3) lm(mat(0, 0), mat(0, 1), -mat(0, 2),
        mat(1, 0), mat(1, 1), -mat(1, 2),
        -mat(2, 0), -mat(2, 1), mat(2, 2));
      */
      unwind_yup_rotation(new_mat, hpr);
    }
    break;

  default:
    linmath_cat.error()
      << "Unexpected coordinate system: " << (int)cs << "\n";
    return false;
  }

#ifdef _DEBUG
  if (linmath_cat.is_debug()) {
    linmath_cat.debug()
      << "after unwind, mat is " << new_mat << "\n";
  }
#endif

  scale.set(new_mat(0, 0), new_mat(1, 1), new_mat(2, 2));

  // Normalize the scale out of the shear components, and return the shear.
  if (scale[0] != 0.0) {
    new_mat(0, 1) /= scale[0];
    new_mat(0, 2) /= scale[0];
  }
  if (scale[1] != 0.0) {
    new_mat(1, 0) /= scale[1];
    new_mat(1, 2) /= scale[1];
  }
  if (scale[2] != 0.0) {
    new_mat(2, 0) /= scale[2];
    new_mat(2, 1) /= scale[2];
  }

  shear.set(new_mat(0, 1) + new_mat(1, 0),
            new_mat(2, 0) + new_mat(0, 2),
            new_mat(2, 1) + new_mat(1, 2));

  return true;
}

/**
 * Converts the HPR as represented in the old, broken way to the new, correct
 * representation.  Returns the new HPR.
 *
 * This function is provided to ease transition from old systems that relied
 * on Panda's original broken HPR calculation.
 */
FLOATNAME(LVecBase3)
old_to_new_hpr(const FLOATNAME(LVecBase3) &old_hpr) {
  TAU_PROFILE("LVecBase3 old_to_new_hpr(const LVecBase3 &)", " ", TAU_USER);
  FLOATNAME(LMatrix3) mat =
    FLOATNAME(LMatrix3)::rotate_mat_normaxis(old_hpr[1], FLOATNAME(LVector3)::right()) *
    FLOATNAME(LMatrix3)::rotate_mat_normaxis(old_hpr[0], FLOATNAME(LVector3)::up()) *
    FLOATNAME(LMatrix3)::rotate_mat_normaxis(old_hpr[2], FLOATNAME(LVector3)::back());

  FLOATNAME(LVecBase3) new_scale;
  FLOATNAME(LVecBase3) new_shear;
  FLOATNAME(LVecBase3) new_hpr;

  decompose_matrix(mat, new_scale, new_shear, new_hpr);
  return new_hpr;
}
