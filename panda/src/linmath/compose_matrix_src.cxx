// Filename: compose_matrix_src.cxx
// Created by:  drose (27Jan99)
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
//     Function: compose_matrix
//  Description: Computes the 3x3 matrix from scale, shear, and
//               rotation.
////////////////////////////////////////////////////////////////////
void
compose_matrix(FLOATNAME(LMatrix3) &mat,
               const FLOATNAME(LVecBase3) &scale,
               const FLOATNAME(LVecBase3) &shear,
               const FLOATNAME(LVecBase3) &hpr,
               CoordinateSystem cs) {

  // temp_hpr_fix blocks use the correct way.  need to keep other way
  // as default until legacy tools are fixed to work with correct way

  if (temp_hpr_fix) {
    mat =
      FLOATNAME(LMatrix3)::scale_shear_mat(scale, shear, cs) *
      FLOATNAME(LMatrix3)::rotate_mat_normaxis(hpr[2], FLOATNAME(LVector3)::forward(cs), cs) *
      FLOATNAME(LMatrix3)::rotate_mat_normaxis(hpr[1], FLOATNAME(LVector3)::right(cs), cs) *
      FLOATNAME(LMatrix3)::rotate_mat_normaxis(hpr[0], FLOATNAME(LVector3)::up(cs), cs);

  } else {
    mat = 
      FLOATNAME(LMatrix3)::scale_shear_mat(scale, shear, cs) *
      FLOATNAME(LMatrix3)::rotate_mat_normaxis(hpr[1], FLOATNAME(LVector3)::right(cs), cs) *
      FLOATNAME(LMatrix3)::rotate_mat_normaxis(hpr[0], FLOATNAME(LVector3)::up(cs), cs) *
      FLOATNAME(LMatrix3)::rotate_mat_normaxis(hpr[2], FLOATNAME(LVector3)::back(cs), cs);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: unwind_yup_rotation
//  Description: Extracts the rotation about the x, y, and z axes from
//               the given hpr & scale matrix.  Adjusts the matrix
//               to eliminate the rotation.
//
//               This function assumes the matrix is stored in a
//               right-handed Y-up coordinate system.
////////////////////////////////////////////////////////////////////
static void
unwind_yup_rotation(FLOATNAME(LMatrix3) &mat, FLOATNAME(LVecBase3) &hpr) {

  typedef FLOATNAME(LMatrix3) Matrix;

  if (temp_hpr_fix) {
    // Extract the axes from the matrix.
    FLOATNAME(LVector3) x, y, z;
    mat.get_row(x,0);
    mat.get_row(y,1);
    mat.get_row(z,2);

    // Project Z into the XZ plane.
    FLOATNAME(LVector2) xz(z[0], z[2]);
    xz = normalize(xz);

    // Compute the rotation about the +Y (up) axis.  This is yaw, or
    // "heading".
    FLOATTYPE heading = rad_2_deg(((FLOATTYPE)atan2(xz[0], xz[1])));

    // Unwind the heading, and continue.
    Matrix rot_y;
    rot_y = Matrix::rotate_mat_normaxis(-heading, FLOATNAME(LVector3)(0.0f, 1.0f, 0.0f),
                                        CS_yup_right);

    x = x * rot_y;
    y = y * rot_y;
    z = z * rot_y;

    // Project the rotated Z into the YZ plane.
    FLOATNAME(LVector2) yz(z[1], z[2]);
    yz = normalize(yz);

    // Compute the rotation about the +X (right) axis.  This is pitch.
    FLOATTYPE pitch = rad_2_deg((FLOATTYPE)(-atan2(yz[0], yz[1])));

    // Unwind the pitch.
    Matrix rot_x;
    rot_x = Matrix::rotate_mat_normaxis(-pitch, FLOATNAME(LVector3)(1.0f, 0.0f, 0.0f),
                                        CS_yup_right);

    x = x * rot_x;
    y = y * rot_x;
    z = z * rot_x;

    // Project the rotated X onto the XY plane.
    FLOATNAME(LVector2) xy(x[0], x[1]);
    xy = normalize(xy);

    // Compute the rotation about the +Z (back) axis.  This is roll.
    FLOATTYPE roll = -rad_2_deg(((FLOATTYPE)atan2(xy[1], xy[0])));

    // Unwind the roll from the axes, and continue.
    Matrix rot_z;
    rot_z = Matrix::rotate_mat_normaxis(roll, FLOATNAME(LVector3)(0.0f, 0.0f, 1.0f),
                                        CS_yup_right);

    x = x * rot_z;
    y = y * rot_z;
    z = z * rot_z;

    // Reset the matrix to reflect the unwinding.
    mat.set_row(0, x);
    mat.set_row(1, y);
    mat.set_row(2, z);

    // Return the three rotation components.
    hpr[0] = heading;
    hpr[1] = pitch;
    hpr[2] = roll;
  } else {

    // Extract the axes from the matrix.
    FLOATNAME(LVector3) x, y, z;
    mat.get_row(x,0);
    mat.get_row(y,1);
    mat.get_row(z,2);

    // Project X onto the XY plane.
    FLOATNAME(LVector2) xy(x[0], x[1]);
    xy = normalize(xy);

    // Compute the rotation about the +Z (back) axis.  This is roll.
    FLOATTYPE roll = rad_2_deg(((FLOATTYPE)atan2(xy[1], xy[0])));

    // Unwind the roll from the axes, and continue.
    Matrix rot_z;
    rot_z = Matrix::rotate_mat_normaxis(-roll, FLOATNAME(LVector3)(0.0f, 0.0f, 1.0f),
                                        CS_yup_right);

    x = x * rot_z;
    y = y * rot_z;
    z = z * rot_z;

    // Project the rotated X into the XZ plane.
    FLOATNAME(LVector2) xz(x[0], x[2]);
    xz = normalize(xz);

    // Compute the rotation about the +Y (up) axis.  This is yaw, or
    // "heading".
    FLOATTYPE heading = rad_2_deg(((FLOATTYPE)-atan2(xz[1], xz[0])));

    // Unwind the heading, and continue.
    Matrix rot_y;
    rot_y = Matrix::rotate_mat_normaxis(-heading, FLOATNAME(LVector3)(0.0f, 1.0f, 0.0f),
                                        CS_yup_right);

    x = x * rot_y;
    y = y * rot_y;
    z = z * rot_y;

    // Project the rotated Z into the YZ plane.
    FLOATNAME(LVector2) yz(z[1], z[2]);
    yz = normalize(yz);

    // Compute the rotation about the +X (right) axis.  This is pitch.
    FLOATTYPE pitch = rad_2_deg(((FLOATTYPE)-atan2(yz[0], yz[1])));

    // Unwind the pitch.
    Matrix rot_x;
    rot_x = Matrix::rotate_mat_normaxis(-pitch, FLOATNAME(LVector3)(1.0f, 0.0f, 0.0f),
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
}

////////////////////////////////////////////////////////////////////
//     Function: unwind_yup_rotation
//  Description: Extracts the rotation about the x, y, and z axes from
//               the given hpr & scale matrix, given the indicated
//               roll amount as a hint.  Adjusts the matrix to
//               eliminate the rotation.
//
//               This function assumes the matrix is stored in a
//               right-handed Y-up coordinate system.
////////////////////////////////////////////////////////////////////
static void
unwind_yup_rotation(FLOATNAME(LMatrix3) &mat, FLOATNAME(LVecBase3) &hpr,
                    FLOATTYPE roll) {
  if (temp_hpr_fix) {
    unwind_yup_rotation(mat, hpr);
    return;
  }

  typedef FLOATNAME(LMatrix3) Matrix;

  // Extract the axes from the matrix.
  FLOATNAME(LVector3) x, y, z;
  mat.get_row(x,0);
  mat.get_row(y,1);
  mat.get_row(z,2);

  // Unwind the roll from the axes, and continue.
  Matrix rot_z;
  rot_z = Matrix::rotate_mat_normaxis(-roll, FLOATNAME(LVector3)(0.0f, 0.0f, 1.0f),
                                      CS_yup_right);

  x = x * rot_z;
  y = y * rot_z;
  z = z * rot_z;

  // Project the rotated X into the XZ plane.
  FLOATNAME(LVector2) xz(x[0], x[2]);
  xz = normalize(xz);

  // Compute the rotation about the +Y (up) axis.  This is yaw, or
  // "heading".
  FLOATTYPE heading = rad_2_deg(((FLOATTYPE)-atan2(xz[1], xz[0])));

  // Unwind the heading, and continue.
  Matrix rot_y;
  rot_y = Matrix::rotate_mat_normaxis(-heading, FLOATNAME(LVector3)(0.0f, 1.0f, 0.0f),
                                      CS_yup_right);

  x = x * rot_y;
  y = y * rot_y;
  z = z * rot_y;

  // Project the rotated Z into the YZ plane.
  FLOATNAME(LVector2) yz(z[1], z[2]);
  yz = normalize(yz);

  // Compute the rotation about the +X (right) axis.  This is pitch.
  FLOATTYPE pitch = rad_2_deg(((FLOATTYPE)-atan2(yz[0], yz[1])));

  // Unwind the pitch.
  Matrix rot_x;
  rot_x = Matrix::rotate_mat_normaxis(-pitch, FLOATNAME(LVector3)(1.0f, 0.0f, 0.0f),
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

////////////////////////////////////////////////////////////////////
//     Function: unwind_zup_rotation
//  Description: Extracts the rotation about the x, y, and z axes from
//               the given hpr & scale matrix.  Adjusts the matrix
//               to eliminate the rotation.
//
//               This function assumes the matrix is stored in a
//               right-handed Z-up coordinate system.
////////////////////////////////////////////////////////////////////
static void
unwind_zup_rotation(FLOATNAME(LMatrix3) &mat, FLOATNAME(LVecBase3) &hpr) {
  if (temp_hpr_fix) {
    typedef FLOATNAME(LMatrix3) Matrix;

    // Extract the axes from the matrix.
    FLOATNAME(LVector3) x, y, z;
    mat.get_row(x,0);
    mat.get_row(y,1);
    mat.get_row(z,2);

    // Project Y into the XY plane.
    FLOATNAME(LVector2) xy(y[0], y[1]);
    xy = normalize(xy);

    // Compute the rotation about the +Z (up) axis.  This is yaw, or
    // "heading".
    FLOATTYPE heading = -rad_2_deg(((FLOATTYPE)atan2(xy[0], xy[1])));

    // Unwind the heading, and continue.
    Matrix rot_z;
    rot_z = Matrix::rotate_mat_normaxis(-heading, FLOATNAME(LVector3)(0.0f, 0.0f, 1.0f),
                                        CS_zup_right);

    x = x * rot_z;
    y = y * rot_z;
    z = z * rot_z;

    // Project the rotated Y into the YZ plane.
    FLOATNAME(LVector2) yz(y[1], y[2]);
    yz = normalize(yz);

    // Compute the rotation about the +X (right) axis.  This is pitch.
    FLOATTYPE pitch = rad_2_deg(((FLOATTYPE)atan2(yz[1], yz[0])));

    // Unwind the pitch.
    Matrix rot_x;
    rot_x = Matrix::rotate_mat_normaxis(-pitch, FLOATNAME(LVector3)(1.0f, 0.0f, 0.0f),
                                        CS_zup_right);

    x = x * rot_x;
    y = y * rot_x;
    z = z * rot_x;

    // Project X into the XZ plane.
    FLOATNAME(LVector2) xz(x[0], x[2]);
    xz = normalize(xz);

    // Compute the rotation about the -Y (back) axis.  This is roll.
    FLOATTYPE roll = -rad_2_deg(((FLOATTYPE)atan2(xz[1], xz[0])));

    // Unwind the roll from the axes, and continue.
    Matrix rot_y;
    rot_y = Matrix::rotate_mat_normaxis(-roll, FLOATNAME(LVector3)(0.0f, 1.0f, 0.0f),
                                        CS_zup_right);

    x = x * rot_y;
    y = y * rot_y;
    z = z * rot_y;

    // Reset the matrix to reflect the unwinding.
    mat.set_row(0, x);
    mat.set_row(1, y);
    mat.set_row(2, z);

    // Return the three rotation components.
    hpr[0] = heading;
    hpr[1] = pitch;
    hpr[2] = roll;

  } else {
    typedef FLOATNAME(LMatrix3) Matrix;

    // Extract the axes from the matrix.
    FLOATNAME(LVector3) x, y, z;
    mat.get_row(x,0);
    mat.get_row(y,1);
    mat.get_row(z,2);


    // Project X into the XZ plane.
    FLOATNAME(LVector2) xz(x[0], x[2]);
    xz = normalize(xz);

    // Compute the rotation about the -Y (back) axis.  This is roll.
    FLOATTYPE roll = rad_2_deg(((FLOATTYPE)atan2(xz[1], xz[0])));

    if (y[1] < 0.0f) {
      if (roll < 0.0f) {
        roll += 180.0;
      } else {
        roll -= 180.0;
      }
    }

    // Unwind the roll from the axes, and continue.
    Matrix rot_y;
    rot_y = Matrix::rotate_mat_normaxis(roll, FLOATNAME(LVector3)(0.0f, 1.0f, 0.0f),
                                        CS_zup_right);

    x = x * rot_y;
    y = y * rot_y;
    z = z * rot_y;

    // Project the rotated X into the XY plane.
    FLOATNAME(LVector2) xy(x[0], x[1]);
    xy = normalize(xy);

    // Compute the rotation about the +Z (up) axis.  This is yaw, or
    // "heading".
    FLOATTYPE heading = rad_2_deg(((FLOATTYPE)atan2(xy[1], xy[0])));

    // Unwind the heading, and continue.
    Matrix rot_z;
    rot_z = Matrix::rotate_mat_normaxis(-heading, FLOATNAME(LVector3)(0.0f, 0.0f, 1.0f),
                                        CS_zup_right);

    x = x * rot_z;
    y = y * rot_z;
    z = z * rot_z;

    // Project the rotated Y into the YZ plane.
    FLOATNAME(LVector2) yz(y[1], y[2]);
    yz = normalize(yz);

    // Compute the rotation about the +X (right) axis.  This is pitch.
    FLOATTYPE pitch = rad_2_deg(((FLOATTYPE)atan2(yz[1], yz[0])));

    // Unwind the pitch.
    Matrix rot_x;
    rot_x = Matrix::rotate_mat_normaxis(-pitch, FLOATNAME(LVector3)(1.0f, 0.0f, 0.0f),
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
}

////////////////////////////////////////////////////////////////////
//     Function: unwind_zup_rotation
//  Description: Extracts the rotation about the x, y, and z axes from
//               the given hpr & scale matrix, given the indicated
//               roll amount as a hint.  Adjusts the matrix to
//               eliminate the rotation.
//
//               This function assumes the matrix is stored in a
//               right-handed Z-up coordinate system.
////////////////////////////////////////////////////////////////////
static void
unwind_zup_rotation(FLOATNAME(LMatrix3) &mat, FLOATNAME(LVecBase3) &hpr,
                    FLOATTYPE roll) {
  if (temp_hpr_fix) {
    unwind_zup_rotation(mat, hpr);
    return;
  }

  typedef FLOATNAME(LMatrix3) Matrix;

  // Extract the axes from the matrix.
  FLOATNAME(LVector3) x, y, z;
  mat.get_row(x,0);
  mat.get_row(y,1);
  mat.get_row(z,2);

  // Unwind the roll from the axes, and continue.
  Matrix rot_y;
  rot_y = Matrix::rotate_mat_normaxis(roll, FLOATNAME(LVector3)(0.0f, 1.0f, 0.0f),
                                      CS_zup_right);

  x = x * rot_y;
  y = y * rot_y;
  z = z * rot_y;

  // Project the rotated X into the XY plane.
  FLOATNAME(LVector2) xy(x[0], x[1]);
  xy = normalize(xy);

  // Compute the rotation about the +Z (up) axis.  This is yaw, or
  // "heading".
  FLOATTYPE heading = rad_2_deg(((FLOATTYPE)atan2(xy[1], xy[0])));

  // Unwind the heading, and continue.
  Matrix rot_z;
  rot_z = Matrix::rotate_mat_normaxis(-heading, FLOATNAME(LVector3)(0.0f, 0.0f, 1.0f),
                                      CS_zup_right);

  x = x * rot_z;
  y = y * rot_z;
  z = z * rot_z;

  // Project the rotated Y into the YZ plane.
  FLOATNAME(LVector2) yz(y[1], y[2]);
  yz = normalize(yz);

  // Compute the rotation about the +X (right) axis.  This is pitch.
  FLOATTYPE pitch = rad_2_deg(((FLOATTYPE)atan2(yz[1], yz[0])));

  // Unwind the pitch.
  Matrix rot_x;
  rot_x = Matrix::rotate_mat_normaxis(-pitch, FLOATNAME(LVector3)(1.0f, 0.0f, 0.0f),
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

////////////////////////////////////////////////////////////////////
//     Function: decompose_matrix
//  Description: Extracts out the components of a 3x3 rotation matrix.
//               Returns true if successful, or false if there was an
//               error.  Since a 3x3 matrix always contains an affine
//               transform, this should succeed in the normal case;
//               singular transforms are not treated as an error.
////////////////////////////////////////////////////////////////////
bool
decompose_matrix(const FLOATNAME(LMatrix3) &mat,
                 FLOATNAME(LVecBase3) &scale,
                 FLOATNAME(LVecBase3) &shear,
                 FLOATNAME(LVecBase3) &hpr,
                 CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  if (linmath_cat.is_debug()) {
    linmath_cat.debug()
      << "decomposing " << mat << " via cs " << cs << "\n";
  }

  // Extract the rotation and scale, according to the coordinate
  // system of choice.
  bool is_left_handed;

  FLOATNAME(LMatrix3) new_mat(mat);

  switch (cs) {
  case CS_zup_right:
    {
      unwind_zup_rotation(new_mat, hpr);
      is_left_handed = false;
    }
    break;

  case CS_yup_right:
    {
      unwind_yup_rotation(new_mat, hpr);
      is_left_handed = false;
    }
    break;

  case CS_zup_left:
    {
      new_mat._m.m._02 = -new_mat._m.m._02;
      new_mat._m.m._12 = -new_mat._m.m._12;
      new_mat._m.m._20 = -new_mat._m.m._20;
      new_mat._m.m._21 = -new_mat._m.m._21;
      /*
        FLOATNAME(LMatrix3) lm(mat(0, 0), mat(0, 1), -mat(0, 2),
        mat(1, 0), mat(1, 1), -mat(1, 2),
        -mat(2, 0), -mat(2, 1), mat(2, 2));
      */
      unwind_zup_rotation(new_mat, hpr);
      hpr[0] = -hpr[0];
      hpr[2] = -hpr[2];
      is_left_handed = true;
    }
    break;

  case CS_yup_left:
    {
      new_mat._m.m._02 = -new_mat._m.m._02;
      new_mat._m.m._12 = -new_mat._m.m._12;
      new_mat._m.m._20 = -new_mat._m.m._20;
      new_mat._m.m._21 = -new_mat._m.m._21;
      /*
        FLOATNAME(LMatrix3) lm(mat(0, 0), mat(0, 1), -mat(0, 2),
        mat(1, 0), mat(1, 1), -mat(1, 2),
        -mat(2, 0), -mat(2, 1), mat(2, 2));
      */
      unwind_yup_rotation(new_mat, hpr);
      is_left_handed = true;
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

  // Normalize the scale out of the shear components, and return the
  // shear.
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

////////////////////////////////////////////////////////////////////
//     Function: decompose_matrix
//  Description: Extracts out the components of a 3x3 rotation matrix.
//               Returns true if the scale and hpr completely describe
//               the matrix, or false if there is also a shear
//               component or if the matrix is not affine.
//
//               This flavor of the function accepts an expected roll
//               amount.  This amount will be used as the roll
//               component, rather than attempting to determine roll
//               by examining the matrix; this helps alleviate roll
//               instability due to roundoff errors or gimbal lock.
//
//               This function is deprecated and will soon be removed,
//               especially when the need for temp_hpr_fix is
//               eliminated.
////////////////////////////////////////////////////////////////////
bool
decompose_matrix(const FLOATNAME(LMatrix3) &mat,
                 FLOATNAME(LVecBase3) &scale,
                 FLOATNAME(LVecBase3) &hpr,
                 FLOATTYPE roll,
                 CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  if (linmath_cat.is_debug()) {
    linmath_cat.debug()
      << "decomposing " << mat << " via cs " << cs
      << " with roll = " << roll << "\n";
  }

  // Extract the rotation and scale, according to the coordinate
  // system of choice.
  bool is_left_handed;

  FLOATNAME(LMatrix3) new_mat(mat);

  switch (cs) {
  case CS_zup_right:
    {
      unwind_zup_rotation(new_mat, hpr, roll);
      is_left_handed = false;
    }
    break;

  case CS_yup_right:
    {
      unwind_yup_rotation(new_mat, hpr, roll);
      is_left_handed = false;
    }
    break;

  case CS_zup_left:
    {
      new_mat._m.m._02 = -new_mat._m.m._02;
      new_mat._m.m._12 = -new_mat._m.m._12;
      new_mat._m.m._20 = -new_mat._m.m._20;
      new_mat._m.m._21 = -new_mat._m.m._21;
      /*
        FLOATNAME(LMatrix3) lm(mat(0, 0), mat(0, 1), -mat(0, 2),
        mat(1, 0), mat(1, 1), -mat(1, 2),
        -mat(2, 0), -mat(2, 1), mat(2, 2));
      */
      unwind_zup_rotation(new_mat, hpr, roll);
      is_left_handed = true;
    }
    break;

  case CS_yup_left:
    {
      new_mat._m.m._02 = -new_mat._m.m._02;
      new_mat._m.m._12 = -new_mat._m.m._12;
      new_mat._m.m._20 = -new_mat._m.m._20;
      new_mat._m.m._21 = -new_mat._m.m._21;
      /*
        FLOATNAME(LMatrix3) lm(mat(0, 0), mat(0, 1), -mat(0, 2),
        mat(1, 0), mat(1, 1), -mat(1, 2),
        -mat(2, 0), -mat(2, 1), mat(2, 2));
      */
      unwind_yup_rotation(new_mat, hpr, roll);
      is_left_handed = true;
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

  scale[0] = new_mat._m.m._00;
  scale[1] = new_mat._m.m._11;
  scale[2] = new_mat._m.m._22;
  /*
  if (is_left_handed) {
    scale[0] = -new_mat._m.m._00;
    scale[1] = -new_mat._m.m._11;
  }
  */

  bool has_no_shear =
    (fabs(new_mat(0, 1)) + fabs(new_mat(0, 2)) +
     fabs(new_mat(1, 0)) + fabs(new_mat(1, 2)) +
     fabs(new_mat(2, 0)) + fabs(new_mat(2, 1))) < 0.0001;

  return has_no_shear;
}

