// Filename: compose_matrix.cxx
// Created by:  drose (27Jan99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: compose_matrix
//  Description: Computes the 3x3 matrix from scale and rotation.
////////////////////////////////////////////////////////////////////
void
compose_matrix(FLOATNAME(LMatrix3) &mat,
	       const FLOATNAME(LVecBase3) &scale,
	       const FLOATNAME(LVecBase3) &hpr,
	       CoordinateSystem cs) {
  mat =
    FLOATNAME(LMatrix3)::scale_mat(scale) *
    FLOATNAME(LMatrix3)::rotate_mat(hpr[1], FLOATNAME(LVector3)::right(cs), cs) *
    FLOATNAME(LMatrix3)::rotate_mat(hpr[0], FLOATNAME(LVector3)::up(cs), cs) *
    FLOATNAME(LMatrix3)::rotate_mat(hpr[2], FLOATNAME(LVector3)::back(cs), cs);
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

  // Extract the axes from the matrix.
  FLOATNAME(LVector3) x, y, z;
  x = mat.get_row(0);
  y = mat.get_row(1);
  z = mat.get_row(2);

  // Project X onto the XY plane.
  FLOATNAME(LVector2) xy(x[0], x[1]);
  xy = normalize(xy);

  // Compute the rotation about the +Z (back) axis.  This is roll.
  FLOATTYPE roll = rad_2_deg(atan2(xy[1], xy[0]));

  // Unwind the roll from the axes, and continue.
  Matrix rot_z;
  rot_z = Matrix::rotate_mat(-roll, FLOATNAME(LVector3)(0.0, 0.0, 1.0),
			     CS_yup_right);

  x = x * rot_z;
  y = y * rot_z;
  z = z * rot_z;

  // Project the rotated X into the XZ plane.
  FLOATNAME(LVector2) xz(x[0], x[2]);
  xz = normalize(xz);

  // Compute the rotation about the +Y (up) axis.  This is yaw, or
  // "heading".
  FLOATTYPE heading = rad_2_deg(-atan2(xz[1], xz[0]));

  // Unwind the heading, and continue.
  Matrix rot_y;
  rot_y = Matrix::rotate_mat(-heading, FLOATNAME(LVector3)(0.0, 1.0, 0.0),
			     CS_yup_right);

  x = x * rot_y;
  y = y * rot_y;
  z = z * rot_y;

  // Project the rotated Z into the YZ plane.
  FLOATNAME(LVector2) yz(z[1], z[2]);
  yz = normalize(yz);

  // Compute the rotation about the +X (right) axis.  This is pitch.
  FLOATTYPE pitch = rad_2_deg(-atan2(yz[0], yz[1]));
 
  // Unwind the pitch.
  Matrix rot_x;
  rot_x = Matrix::rotate_mat(-pitch, FLOATNAME(LVector3)(1.0, 0.0, 0.0), 
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
  typedef FLOATNAME(LMatrix3) Matrix;

  // Extract the axes from the matrix.
  FLOATNAME(LVector3) x, y, z;
  x = mat.get_row(0);
  y = mat.get_row(1);
  z = mat.get_row(2);

  // Unwind the roll from the axes, and continue.
  Matrix rot_z;
  rot_z = Matrix::rotate_mat(-roll, FLOATNAME(LVector3)(0.0, 0.0, 1.0),
			     CS_yup_right);

  x = x * rot_z;
  y = y * rot_z;
  z = z * rot_z;

  // Project the rotated X into the XZ plane.
  FLOATNAME(LVector2) xz(x[0], x[2]);
  xz = normalize(xz);

  // Compute the rotation about the +Y (up) axis.  This is yaw, or
  // "heading".
  FLOATTYPE heading = rad_2_deg(-atan2(xz[1], xz[0]));

  // Unwind the heading, and continue.
  Matrix rot_y;
  rot_y = Matrix::rotate_mat(-heading, FLOATNAME(LVector3)(0.0, 1.0, 0.0),
			     CS_yup_right);

  x = x * rot_y;
  y = y * rot_y;
  z = z * rot_y;

  // Project the rotated Z into the YZ plane.
  FLOATNAME(LVector2) yz(z[1], z[2]);
  yz = normalize(yz);

  // Compute the rotation about the +X (right) axis.  This is pitch.
  FLOATTYPE pitch = rad_2_deg(-atan2(yz[0], yz[1]));
 
  // Unwind the pitch.
  Matrix rot_x;
  rot_x = Matrix::rotate_mat(-pitch, FLOATNAME(LVector3)(1.0, 0.0, 0.0), 
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
  typedef FLOATNAME(LMatrix3) Matrix;

  // Extract the axes from the matrix.
  FLOATNAME(LVector3) x, y, z;
  x = mat.get_row(0);
  y = mat.get_row(1);
  z = mat.get_row(2);


  // Project X into the XZ plane.
  FLOATNAME(LVector2) xz(x[0], x[2]);
  xz = normalize(xz);
  
  // Compute the rotation about the -Y (back) axis.  This is roll.
  FLOATTYPE roll = rad_2_deg(atan2(xz[1], xz[0]));
  
  if (y[1] < 0.0) {
    if (roll < 0.0) {
      roll += 180.0;
    } else {
      roll -= 180.0;
    }
  }
  
  // Unwind the roll from the axes, and continue.
  Matrix rot_y;
  rot_y = Matrix::rotate_mat(roll, FLOATNAME(LVector3)(0.0, 1.0, 0.0),
			     CS_zup_right);
  
  x = x * rot_y;
  y = y * rot_y;
  z = z * rot_y;
  
  // Project the rotated X into the XY plane.
  FLOATNAME(LVector2) xy(x[0], x[1]);
  xy = normalize(xy);

  // Compute the rotation about the +Z (up) axis.  This is yaw, or
  // "heading".
  FLOATTYPE heading = rad_2_deg(atan2(xy[1], xy[0]));

  // Unwind the heading, and continue.
  Matrix rot_z;
  rot_z = Matrix::rotate_mat(-heading, FLOATNAME(LVector3)(0.0, 0.0, 1.0),
			     CS_zup_right);

  x = x * rot_z;
  y = y * rot_z;
  z = z * rot_z;

  // Project the rotated Y into the YZ plane.
  FLOATNAME(LVector2) yz(y[1], y[2]);
  yz = normalize(yz);

  // Compute the rotation about the +X (right) axis.  This is pitch.
  FLOATTYPE pitch = rad_2_deg(atan2(yz[1], yz[0]));

  // Unwind the pitch.
  Matrix rot_x;
  rot_x = Matrix::rotate_mat(-pitch, FLOATNAME(LVector3)(1.0, 0.0, 0.0), 
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
  typedef FLOATNAME(LMatrix3) Matrix;

  // Extract the axes from the matrix.
  FLOATNAME(LVector3) x, y, z;
  x = mat.get_row(0);
  y = mat.get_row(1);
  z = mat.get_row(2);
  
  // Unwind the roll from the axes, and continue.
  Matrix rot_y;
  rot_y = Matrix::rotate_mat(roll, FLOATNAME(LVector3)(0.0, 1.0, 0.0),
			     CS_zup_right);
  
  x = x * rot_y;
  y = y * rot_y;
  z = z * rot_y;
  
  // Project the rotated X into the XY plane.
  FLOATNAME(LVector2) xy(x[0], x[1]);
  xy = normalize(xy);

  // Compute the rotation about the +Z (up) axis.  This is yaw, or
  // "heading".
  FLOATTYPE heading = rad_2_deg(atan2(xy[1], xy[0]));

  // Unwind the heading, and continue.
  Matrix rot_z;
  rot_z = Matrix::rotate_mat(-heading, FLOATNAME(LVector3)(0.0, 0.0, 1.0),
			     CS_zup_right);

  x = x * rot_z;
  y = y * rot_z;
  z = z * rot_z;

  // Project the rotated Y into the YZ plane.
  FLOATNAME(LVector2) yz(y[1], y[2]);
  yz = normalize(yz);

  // Compute the rotation about the +X (right) axis.  This is pitch.
  FLOATTYPE pitch = rad_2_deg(atan2(yz[1], yz[0]));

  // Unwind the pitch.
  Matrix rot_x;
  rot_x = Matrix::rotate_mat(-pitch, FLOATNAME(LVector3)(1.0, 0.0, 0.0), 
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
//               Returns true if the scale and hpr completely describe
//               the matrix, or false if there is also a shear
//               component or if the matrix is not affine.
////////////////////////////////////////////////////////////////////
bool
decompose_matrix(const FLOATNAME(LMatrix3) &mat,
		 FLOATNAME(LVecBase3) &scale,
		 FLOATNAME(LVecBase3) &hpr,
		 CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }

  // Extract the rotation and scale, according to the coordinate
  // system of choice.
  bool shear;

  switch (cs) {
  case CS_zup_right:
    {
      FLOATNAME(LMatrix3) rm(mat);
      unwind_zup_rotation(rm, hpr);
      scale[0] = rm(0, 0);
      scale[1] = rm(1, 1);
      scale[2] = rm(2, 2);
      shear = 
	(fabs(rm(0, 1)) + fabs(rm(0, 2)) +
	 fabs(rm(1, 0)) + fabs(rm(1, 2)) +
	 fabs(rm(2, 0)) + fabs(rm(2, 1))) >= 0.0001;
    }
    break;

  case CS_yup_right:
    {
      FLOATNAME(LMatrix3) rm(mat);
      unwind_yup_rotation(rm, hpr);
      scale[0] = rm(0, 0);
      scale[1] = rm(1, 1);
      scale[2] = rm(2, 2);
      shear = 
	(fabs(rm(0, 1)) + fabs(rm(0, 2)) +
	 fabs(rm(1, 0)) + fabs(rm(1, 2)) +
	 fabs(rm(2, 0)) + fabs(rm(2, 1))) >= 0.0001;
    }
    break;

  case CS_zup_left:
    {
      FLOATNAME(LMatrix3) lm(mat(0, 0), mat(0, 1), -mat(0, 2),
			   mat(1, 0), mat(1, 1), -mat(1, 2),
			   -mat(2, 0), -mat(2, 1), mat(2, 2));
      unwind_zup_rotation(lm, hpr);
      scale[0] = -lm(0, 0);
      scale[1] = -lm(1, 1);
      scale[2] = lm(2, 2);
      shear = 
	(fabs(lm(0, 1)) + fabs(lm(0, 2)) +
	 fabs(lm(1, 0)) + fabs(lm(1, 2)) +
	 fabs(lm(2, 0)) + fabs(lm(2, 1))) >= 0.0001;
    }
    break;

  case CS_yup_left:
    {
      FLOATNAME(LMatrix3) lm(mat(0, 0), mat(0, 1), -mat(0, 2),
			   mat(1, 0), mat(1, 1), -mat(1, 2),
			   -mat(2, 0), -mat(2, 1), mat(2, 2));
      unwind_yup_rotation(lm, hpr);
      scale[0] = -lm(0, 0);
      scale[1] = -lm(1, 1);
      scale[2] = lm(2, 2);
      shear = 
	(fabs(lm(0, 1)) + fabs(lm(0, 2)) +
	 fabs(lm(1, 0)) + fabs(lm(1, 2)) +
	 fabs(lm(2, 0)) + fabs(lm(2, 1))) >= 0.0001;
    }
    break;

  default:
    linmath_cat.error()
      << "Unexpected coordinate system: " << (int)cs << "\n";
    return false;
  }

  return !shear;
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
////////////////////////////////////////////////////////////////////
bool
decompose_matrix(const FLOATNAME(LMatrix3) &mat,
		 FLOATNAME(LVecBase3) &scale,
		 FLOATNAME(LVecBase3) &hpr,
		 FLOATTYPE roll,
		 CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }

  // Extract the rotation and scale, according to the coordinate
  // system of choice.
  bool shear;

  switch (cs) {
  case CS_zup_right:
    {
      FLOATNAME(LMatrix3) rm(mat);
      unwind_zup_rotation(rm, hpr, roll);
      scale[0] = rm(0, 0);
      scale[1] = rm(1, 1);
      scale[2] = rm(2, 2);
      shear = 
	(fabs(rm(0, 1)) + fabs(rm(0, 2)) +
	 fabs(rm(1, 0)) + fabs(rm(1, 2)) +
	 fabs(rm(2, 0)) + fabs(rm(2, 1))) >= 0.0001;
    }
    break;

  case CS_yup_right:
    {
      FLOATNAME(LMatrix3) rm(mat);
      unwind_yup_rotation(rm, hpr, roll);
      scale[0] = rm(0, 0);
      scale[1] = rm(1, 1);
      scale[2] = rm(2, 2);
      shear = 
	(fabs(rm(0, 1)) + fabs(rm(0, 2)) +
	 fabs(rm(1, 0)) + fabs(rm(1, 2)) +
	 fabs(rm(2, 0)) + fabs(rm(2, 1))) >= 0.0001;
    }
    break;

  case CS_zup_left:
    {
      FLOATNAME(LMatrix3) lm(mat(0, 0), mat(0, 1), -mat(0, 2),
			   mat(1, 0), mat(1, 1), -mat(1, 2),
			   -mat(2, 0), -mat(2, 1), mat(2, 2));
      unwind_zup_rotation(lm, hpr, roll);
      scale[0] = -lm(0, 0);
      scale[1] = -lm(1, 1);
      scale[2] = lm(2, 2);
      shear = 
	(fabs(lm(0, 1)) + fabs(lm(0, 2)) +
	 fabs(lm(1, 0)) + fabs(lm(1, 2)) +
	 fabs(lm(2, 0)) + fabs(lm(2, 1))) >= 0.0001;
    }
    break;

  case CS_yup_left:
    {
      FLOATNAME(LMatrix3) lm(mat(0, 0), mat(0, 1), -mat(0, 2),
			   mat(1, 0), mat(1, 1), -mat(1, 2),
			   -mat(2, 0), -mat(2, 1), mat(2, 2));
      unwind_yup_rotation(lm, hpr, roll);
      scale[0] = -lm(0, 0);
      scale[1] = -lm(1, 1);
      scale[2] = lm(2, 2);
      shear = 
	(fabs(lm(0, 1)) + fabs(lm(0, 2)) +
	 fabs(lm(1, 0)) + fabs(lm(1, 2)) +
	 fabs(lm(2, 0)) + fabs(lm(2, 1))) >= 0.0001;
    }
    break;

  default:
    linmath_cat.error()
      << "Unexpected coordinate system: " << (int)cs << "\n";
    return false;
  }

  return !shear;
}
