// Filename: compose_matrix.cxx
// Created by:  drose (27Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "compose_matrix.h"
#include "deg_2_rad.h"
#include "config_linmath.h"

#include <math.h>

////////////////////////////////////////////////////////////////////
//     Function: compose_matrix
//  Description: Computes the 3x3 matrix from scale and rotation.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void
_compose_matrix(LMatrix3<NumType> &mat,
		const LVecBase3<NumType> &scale,
		const LVecBase3<NumType> &hpr,
		CoordinateSystem cs) {
  mat =
    LMatrix3<NumType>::scale_mat(scale) *
    LMatrix3<NumType>::rotate_mat(hpr[1], LVector3<NumType>::right(cs), cs) *
    LMatrix3<NumType>::rotate_mat(hpr[0], LVector3<NumType>::up(cs), cs) *
    LMatrix3<NumType>::rotate_mat(hpr[2], LVector3<NumType>::back(cs), cs);
}

////////////////////////////////////////////////////////////////////
//     Function: compose_matrix
//  Description: Computes the 4x4 matrix according to scale, rotation,
//               and translation.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE void
_compose_matrix(LMatrix4<NumType> &mat,
		const LVecBase3<NumType> &scale,
		const LVecBase3<NumType> &hpr,
		const LVecBase3<NumType> &translate,
		CoordinateSystem cs) {
  LMatrix3<NumType> upper3;
  _compose_matrix(upper3, scale, hpr, cs);
  mat = LMatrix4<NumType>(upper3, translate);
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
template<class NumType>
static void
unwind_yup_rotation(LMatrix3<NumType> &mat, LVecBase3<NumType> &hpr) {
  typedef LMatrix3<NumType> Matrix;

  // Extract the axes from the matrix.
  LVector3<NumType> x, y, z;
  x = mat.get_row(0);
  y = mat.get_row(1);
  z = mat.get_row(2);

  // Project X onto the XY plane.
  LVector2<NumType> xy(x[0], x[1]);
  xy = normalize(xy);

  // Compute the rotation about the +Z (back) axis.  This is roll.
  NumType roll = rad_2_deg(atan2(xy[1], xy[0]));

  // Unwind the roll from the axes, and continue.
  Matrix rot_z;
  rot_z = Matrix::rotate_mat(-roll, LVector3<NumType>(0.0, 0.0, 1.0),
			     CS_yup_right);

  x = x * rot_z;
  y = y * rot_z;
  z = z * rot_z;

  // Project the rotated X into the XZ plane.
  LVector2<NumType> xz(x[0], x[2]);
  xz = normalize(xz);

  // Compute the rotation about the +Y (up) axis.  This is yaw, or
  // "heading".
  NumType heading = rad_2_deg(-atan2(xz[1], xz[0]));

  // Unwind the heading, and continue.
  Matrix rot_y;
  rot_y = Matrix::rotate_mat(-heading, LVector3<NumType>(0.0, 1.0, 0.0),
			     CS_yup_right);

  x = x * rot_y;
  y = y * rot_y;
  z = z * rot_y;

  // Project the rotated Z into the YZ plane.
  LVector2<NumType> yz(z[1], z[2]);
  yz = normalize(yz);

  // Compute the rotation about the +X (right) axis.  This is pitch.
  NumType pitch = rad_2_deg(-atan2(yz[0], yz[1]));
 
  // Unwind the pitch.
  Matrix rot_x;
  rot_x = Matrix::rotate_mat(-pitch, LVector3<NumType>(1.0, 0.0, 0.0), 
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
template<class NumType>
static void
unwind_yup_rotation(LMatrix3<NumType> &mat, LVecBase3<NumType> &hpr,
		    NumType roll) {
  typedef LMatrix3<NumType> Matrix;

  // Extract the axes from the matrix.
  LVector3<NumType> x, y, z;
  x = mat.get_row(0);
  y = mat.get_row(1);
  z = mat.get_row(2);

  // Unwind the roll from the axes, and continue.
  Matrix rot_z;
  rot_z = Matrix::rotate_mat(-roll, LVector3<NumType>(0.0, 0.0, 1.0),
			     CS_yup_right);

  x = x * rot_z;
  y = y * rot_z;
  z = z * rot_z;

  // Project the rotated X into the XZ plane.
  LVector2<NumType> xz(x[0], x[2]);
  xz = normalize(xz);

  // Compute the rotation about the +Y (up) axis.  This is yaw, or
  // "heading".
  NumType heading = rad_2_deg(-atan2(xz[1], xz[0]));

  // Unwind the heading, and continue.
  Matrix rot_y;
  rot_y = Matrix::rotate_mat(-heading, LVector3<NumType>(0.0, 1.0, 0.0),
			     CS_yup_right);

  x = x * rot_y;
  y = y * rot_y;
  z = z * rot_y;

  // Project the rotated Z into the YZ plane.
  LVector2<NumType> yz(z[1], z[2]);
  yz = normalize(yz);

  // Compute the rotation about the +X (right) axis.  This is pitch.
  NumType pitch = rad_2_deg(-atan2(yz[0], yz[1]));
 
  // Unwind the pitch.
  Matrix rot_x;
  rot_x = Matrix::rotate_mat(-pitch, LVector3<NumType>(1.0, 0.0, 0.0), 
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
template<class NumType>
static void
unwind_zup_rotation(LMatrix3<NumType> &mat, LVecBase3<NumType> &hpr) {
  typedef LMatrix3<NumType> Matrix;

  // Extract the axes from the matrix.
  LVector3<NumType> x, y, z;
  x = mat.get_row(0);
  y = mat.get_row(1);
  z = mat.get_row(2);


  // Project X into the XZ plane.
  LVector2<NumType> xz(x[0], x[2]);
  xz = normalize(xz);
  
  // Compute the rotation about the -Y (back) axis.  This is roll.
  NumType roll = rad_2_deg(atan2(xz[1], xz[0]));
  
  if (y[1] < 0.0) {
    if (roll < 0.0) {
      roll += 180.0;
    } else {
      roll -= 180.0;
    }
  }
  
  // Unwind the roll from the axes, and continue.
  Matrix rot_y;
  rot_y = Matrix::rotate_mat(roll, LVector3<NumType>(0.0, 1.0, 0.0),
			     CS_zup_right);
  
  x = x * rot_y;
  y = y * rot_y;
  z = z * rot_y;
  
  // Project the rotated X into the XY plane.
  LVector2<NumType> xy(x[0], x[1]);
  xy = normalize(xy);

  // Compute the rotation about the +Z (up) axis.  This is yaw, or
  // "heading".
  NumType heading = rad_2_deg(atan2(xy[1], xy[0]));

  // Unwind the heading, and continue.
  Matrix rot_z;
  rot_z = Matrix::rotate_mat(-heading, LVector3<NumType>(0.0, 0.0, 1.0),
			     CS_zup_right);

  x = x * rot_z;
  y = y * rot_z;
  z = z * rot_z;

  // Project the rotated Y into the YZ plane.
  LVector2<NumType> yz(y[1], y[2]);
  yz = normalize(yz);

  // Compute the rotation about the +X (right) axis.  This is pitch.
  NumType pitch = rad_2_deg(atan2(yz[1], yz[0]));

  // Unwind the pitch.
  Matrix rot_x;
  rot_x = Matrix::rotate_mat(-pitch, LVector3<NumType>(1.0, 0.0, 0.0), 
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
template<class NumType>
static void
unwind_zup_rotation(LMatrix3<NumType> &mat, LVecBase3<NumType> &hpr,
		    NumType roll) {
  typedef LMatrix3<NumType> Matrix;

  // Extract the axes from the matrix.
  LVector3<NumType> x, y, z;
  x = mat.get_row(0);
  y = mat.get_row(1);
  z = mat.get_row(2);
  
  // Unwind the roll from the axes, and continue.
  Matrix rot_y;
  rot_y = Matrix::rotate_mat(roll, LVector3<NumType>(0.0, 1.0, 0.0),
			     CS_zup_right);
  
  x = x * rot_y;
  y = y * rot_y;
  z = z * rot_y;
  
  // Project the rotated X into the XY plane.
  LVector2<NumType> xy(x[0], x[1]);
  xy = normalize(xy);

  // Compute the rotation about the +Z (up) axis.  This is yaw, or
  // "heading".
  NumType heading = rad_2_deg(atan2(xy[1], xy[0]));

  // Unwind the heading, and continue.
  Matrix rot_z;
  rot_z = Matrix::rotate_mat(-heading, LVector3<NumType>(0.0, 0.0, 1.0),
			     CS_zup_right);

  x = x * rot_z;
  y = y * rot_z;
  z = z * rot_z;

  // Project the rotated Y into the YZ plane.
  LVector2<NumType> yz(y[1], y[2]);
  yz = normalize(yz);

  // Compute the rotation about the +X (right) axis.  This is pitch.
  NumType pitch = rad_2_deg(atan2(yz[1], yz[0]));

  // Unwind the pitch.
  Matrix rot_x;
  rot_x = Matrix::rotate_mat(-pitch, LVector3<NumType>(1.0, 0.0, 0.0), 
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
template<class NumType>
static bool
_decompose_matrix(const LMatrix3<NumType> &mat,
		  LVecBase3<NumType> &scale,
		  LVecBase3<NumType> &hpr,
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
      LMatrix3<NumType> rm(mat);
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
      LMatrix3<NumType> rm(mat);
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
      LMatrix3<NumType> lm(mat(0, 0), mat(0, 1), -mat(0, 2),
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
      LMatrix3<NumType> lm(mat(0, 0), mat(0, 1), -mat(0, 2),
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
template<class NumType>
static bool
_decompose_matrix(const LMatrix3<NumType> &mat,
		  LVecBase3<NumType> &scale,
		  LVecBase3<NumType> &hpr,
		  NumType roll,
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
      LMatrix3<NumType> rm(mat);
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
      LMatrix3<NumType> rm(mat);
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
      LMatrix3<NumType> lm(mat(0, 0), mat(0, 1), -mat(0, 2),
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
      LMatrix3<NumType> lm(mat(0, 0), mat(0, 1), -mat(0, 2),
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

////////////////////////////////////////////////////////////////////
//     Function: decompose_matrix
//  Description: Extracts out the components of an affine matrix.
//               Returns true if the scale, hpr, translate
//               completely describe the matrix, or false if there is
//               also a shear component or if the matrix is not
//               affine.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool
_decompose_matrix(const LMatrix4<NumType> &mat,
		  LVecBase3<NumType> &scale,
		  LVecBase3<NumType> &hpr,
		  LVecBase3<NumType> &translate,
		  CoordinateSystem cs) {
  // Get the translation first.
  translate = mat.get_row3(3);
  return _decompose_matrix(mat.get_upper_3(), scale, hpr, cs);
}

////////////////////////////////////////////////////////////////////
//     Function: decompose_matrix
//  Description: Extracts out the components of an affine matrix.
//               Returns true if the scale, hpr, translate
//               completely describe the matrix, or false if there is
//               also a shear component or if the matrix is not
//               affine.
//
//               This flavor of the function accepts an expected roll
//               amount.  This amount will be used as the roll
//               component, rather than attempting to determine roll
//               by examining the matrix; this helps alleviate roll
//               instability due to roundoff errors or gimbal lock.
////////////////////////////////////////////////////////////////////
template<class NumType>
INLINE bool
_decompose_matrix(const LMatrix4<NumType> &mat,
		  LVecBase3<NumType> &scale,
		  LVecBase3<NumType> &hpr,
		  LVecBase3<NumType> &translate,
		  NumType roll,
		  CoordinateSystem cs) {
  // Get the translation first.
  translate = mat.get_row3(3);
  return _decompose_matrix(mat.get_upper_3(), scale, hpr, roll, cs);
}

void
compose_matrix(LMatrix3f &mat,
	       const LVecBase3f &scale,
	       const LVecBase3f &hpr,
	       CoordinateSystem cs) {
  _compose_matrix(mat, scale, hpr, cs);
}

bool
decompose_matrix(const LMatrix3f &mat,
		 LVecBase3f &scale,
		 LVecBase3f &hpr,
		 CoordinateSystem cs) {
  return _decompose_matrix(mat, scale, hpr, cs);
}

bool
decompose_matrix(const LMatrix3f &mat,
		 LVecBase3f &scale,
		 LVecBase3f &hpr,
		 float roll,
		 CoordinateSystem cs) {
  return _decompose_matrix(mat, scale, hpr, roll, cs);
}

void
compose_matrix(LMatrix3d &mat,
	       const LVecBase3d &scale,
	       const LVecBase3d &hpr,
	       CoordinateSystem cs) {
  _compose_matrix(mat, scale, hpr, cs);
}

bool
decompose_matrix(const LMatrix3d &mat,
		 LVecBase3d &scale,
		 LVecBase3d &hpr,
		 CoordinateSystem cs) {
  return _decompose_matrix(mat, scale, hpr, cs);
}

bool
decompose_matrix(const LMatrix3d &mat,
		 LVecBase3d &scale,
		 LVecBase3d &hpr,
		 double roll,
		 CoordinateSystem cs) {
  return _decompose_matrix(mat, scale, hpr, roll, cs);
}

void
compose_matrix(LMatrix4f &mat,
	       const LVecBase3f &scale,
	       const LVecBase3f &hpr,
	       const LVecBase3f &translate,
	       CoordinateSystem cs) {
  _compose_matrix(mat, scale, hpr, translate, cs);
}

bool
decompose_matrix(const LMatrix4f &mat,
		 LVecBase3f &scale,
		 LVecBase3f &hpr,
		 LVecBase3f &translate,
		 CoordinateSystem cs) {
  return _decompose_matrix(mat, scale, hpr, translate, cs);
}

bool
decompose_matrix(const LMatrix4f &mat,
		 LVecBase3f &scale,
		 LVecBase3f &hpr,
		 LVecBase3f &translate,
		 float roll,
		 CoordinateSystem cs) {
  return _decompose_matrix(mat, scale, hpr, translate, roll, cs);
}

void
compose_matrix(LMatrix4d &mat,
	       const LVecBase3d &scale,
	       const LVecBase3d &hpr,
	       const LVecBase3d &translate,
	       CoordinateSystem cs) {
  _compose_matrix(mat, scale, hpr, translate, cs);
}

bool
decompose_matrix(const LMatrix4d &mat,
		 LVecBase3d &scale,
		 LVecBase3d &hpr,
		 LVecBase3d &translate,
		 CoordinateSystem cs) {
  return _decompose_matrix(mat, scale, hpr, translate, cs);
}

bool
decompose_matrix(const LMatrix4d &mat,
		 LVecBase3d &scale,
		 LVecBase3d &hpr,
		 LVecBase3d &translate,
		 double roll,
		 CoordinateSystem cs) {
  return _decompose_matrix(mat, scale, hpr, translate, roll, cs);
}

