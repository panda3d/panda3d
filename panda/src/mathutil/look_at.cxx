// Filename: lookAt.cxx
// Created by:  drose (25Apr97)
// 
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "look_at.h"
#include <cmath.h>

template<class NumType>
INLINE LMatrix3<NumType>
make_xi_mat(const LVector2<NumType> &x) {
  return LMatrix3<NumType>(1,     0,     0,
			   0,  x[0],  x[1],
			   0, -x[1],  x[0]);
}

template<class NumType>
INLINE LMatrix3<NumType>
make_x_mat(const LVector2<NumType> &x) {
  return LMatrix3<NumType>(1,     0,     0,
			   0,  x[1],  x[0],
			   0, -x[0],  x[1]);
}

template<class NumType>
INLINE LMatrix3<NumType>
make_y_mat(const LVector2<NumType> &y) {
  return LMatrix3<NumType>(y[1],     0, -y[0],
  			      0,     1,     0,
			   y[0],     0,  y[1]);
}

template<class NumType>
INLINE LMatrix3<NumType>
make_z_mat(const LVector2<NumType> &z) {
  return LMatrix3<NumType>(z[1], -z[0],     0,
			   z[0],  z[1],     0,
			      0,     0,     1);
}

////////////////////////////////////////////////////////////////////
//     Function: heads_up
//  Description: Given two vectors defining a forward direction and an
//               up vector, constructs the matrix that rotates things
//               from the defined coordinate system to y-forward and
//               z-up.  The up vector will be rotated to z-up first,
//               then the forward vector will be rotated as nearly to
//               y-forward as possible.  This will only have a
//               different effect from look_at() if the forward and up
//               vectors are not perpendicular.
////////////////////////////////////////////////////////////////////
template<class NumType>
static void
_heads_up(LMatrix3<NumType> &mat, const LVector3<NumType> &fwd, 
	  const LVector3<NumType> &up, CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }

  if (cs == CS_zup_right || cs == CS_zup_left) {
    // Z-up.

    // y is the projection of the up vector into the XZ plane.  Its
    // angle to the Z axis is the amount to rotate about the Y axis to
    // bring the up vector into the YZ plane.

    LVector2<NumType> y(up[0], up[2]);
    NumType d = dot(y, y);
    if (d==0.0) {
      y = LVector2<NumType>(0.0, 1.0);
    } else {
      y /= csqrt(d);
    }
    
    // x is the up vector rotated into the YZ plane.  Its angle to the Z
    // axis is the amount to rotate about the X axis to bring the up
    // vector to the Z axis.
    
    LVector2<NumType> x(up[1], up[0]*y[0]+up[2]*y[1]);
    d = dot(x, x);
    if (d==0.0) {
      x = LVector2<NumType>(0.0, 1.0);
    } else {
      x /= csqrt(d);
    }
    
    // Now apply both rotations to the forward vector.  This will rotate
    // the forward vector by the same amount we would have had to rotate
    // the up vector to bring it to the Z axis.  If the vectors were
    // perpendicular, this will put the forward vector somewhere in the
    // XY plane.
    
    // z is the projection of the newly rotated fwd vector into the XY
    // plane.  Its angle to the Y axis is the amount to rotate about the
    // Z axis in order to bring the fwd vector to the Y axis.
    LVector2<NumType> z(fwd[0]*y[1] - fwd[2]*y[0],
			-fwd[0]*y[0]*x[0] + fwd[1]*x[1] - fwd[2]*y[1]*x[0]);
    d = dot(z, z);
    if (d==0.0) {
      z = LVector2<NumType>(0.0, 1.0);
    } else {
      z /= csqrt(d);
    }
    
    // Now build the net rotation matrix.
    if (cs == CS_zup_right) {
      mat = 
	make_z_mat(z) *
	make_x_mat(x) *
	make_y_mat(y);
    } else { // cs == CS_zup_left
      mat = 
	make_z_mat(z) *
	make_x_mat(-x) *
	make_y_mat(-y);
    }
  } else {
    // Y-up.

    // z is the projection of the forward vector into the XY plane.  Its
    // angle to the Y axis is the amount to rotate about the Z axis to
    // bring the forward vector into the YZ plane.

    LVector2<NumType> z(up[0], up[1]);
    NumType d = dot(z, z);
    if (d==0.0) {
      z = LVector2<NumType>(0.0, 1.0);
    } else {
      z /= csqrt(d);
    }

    // x is the forward vector rotated into the YZ plane.  Its angle to
    // the Y axis is the amount to rotate about the X axis to bring the
    // forward vector to the Y axis.

    LVector2<NumType> x(up[0]*z[0] + up[1]*z[1], up[2]);
    d = dot(x, x);
    if (d==0.0) {
      x = LVector2<NumType>(1.0, 0.0);
    } else {
      x /= csqrt(d);
    }

    // Now apply both rotations to the up vector.  This will rotate
    // the up vector by the same amount we would have had to rotate
    // the forward vector to bring it to the Y axis.  If the vectors were
    // perpendicular, this will put the up vector somewhere in the
    // XZ plane.

    // y is the projection of the newly rotated up vector into the XZ
    // plane.  Its angle to the Z axis is the amount to rotate about the
    // Y axis in order to bring the up vector to the Z axis.
    LVector2<NumType> y(fwd[0]*z[1] - fwd[1]*z[0],
			-fwd[0]*x[1]*z[0] - fwd[1]*x[1]*z[1] + fwd[2]*x[0]);
    d = dot(y, y);
    if (d==0.0) {
      y = LVector2<NumType>(0.0, 1.0);
    } else {
      y /= csqrt(d);
    }

    // Now build the net rotation matrix.
    if (cs == CS_yup_right) {
      mat = 
	make_y_mat(y) *
	make_xi_mat(-x) *
	make_z_mat(-z);
    } else { // cs == CS_yup_left
      mat = 
	make_y_mat(y) *
	make_xi_mat(x) *
	make_z_mat(z);
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: look_at
//  Description: Given two vectors defining a forward direction and an
//               up vector, constructs the matrix that rotates things
//               from the defined coordinate system to y-forward and
//               z-up.  The forward vector will be rotated to
//               y-forward first, then the up vector will be rotated
//               as nearly to z-up as possible.  This will only have a
//               different effect from heads_up() if the forward and
//               up vectors are not perpendicular.
////////////////////////////////////////////////////////////////////
template<class NumType>
static void
_look_at(LMatrix3<NumType> &mat, const LVector3<NumType> &fwd, 
	 const LVector3<NumType> &up, CoordinateSystem cs) {
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }

  if (cs == CS_zup_right || cs == CS_zup_left) {
    // Z-up.

    // z is the projection of the forward vector into the XY plane.  Its
    // angle to the Y axis is the amount to rotate about the Z axis to
    // bring the forward vector into the YZ plane.
    
    LVector2<NumType> z(fwd[0], fwd[1]);
    NumType d = dot(z, z);
    if (d==0.0) {
      z = LVector2<NumType>(0.0, 1.0);
    } else {
      z /= csqrt(d);
    }

    // x is the forward vector rotated into the YZ plane.  Its angle to
    // the Y axis is the amount to rotate about the X axis to bring the
    // forward vector to the Y axis.
    
    LVector2<NumType> x(fwd[0]*z[0] + fwd[1]*z[1], fwd[2]);
    d = dot(x, x);
    if (d==0.0) {
      x = LVector2<NumType>(1.0, 0.0);
    } else {
      x /= csqrt(d);
    }
    
    // Now apply both rotations to the up vector.  This will rotate
    // the up vector by the same amount we would have had to rotate
    // the forward vector to bring it to the Y axis.  If the vectors were
    // perpendicular, this will put the up vector somewhere in the
    // XZ plane.
    
    // y is the projection of the newly rotated up vector into the XZ
    // plane.  Its angle to the Z axis is the amount to rotate about the
    // Y axis in order to bring the up vector to the Z axis.
    LVector2<NumType> y(up[0]*z[1] - up[1]*z[0],
			-up[0]*x[1]*z[0] - up[1]*x[1]*z[1] + up[2]*x[0]);
    d = dot(y, y);
    if (d==0.0) {
      y = LVector2<NumType>(0.0, 1.0);
    } else {
      y /= csqrt(d);
    }

    // Now build the net rotation matrix.
    if (cs == CS_zup_right) {
      mat =
	make_y_mat(y) *
	make_xi_mat(x) *
	make_z_mat(z);
    } else { // cs == CS_zup_left
      mat =
	make_y_mat(-y) *
	make_xi_mat(-x) *
	make_z_mat(z);
    }
  } else {
    // Y-up.

    // y is the projection of the up vector into the XZ plane.  Its
    // angle to the Z axis is the amount to rotate about the Y axis to
    // bring the up vector into the YZ plane.
    
    LVector2<NumType> y(fwd[0], fwd[2]);
    NumType d = dot(y, y);
    if (d==0.0) {
      y = LVector2<NumType>(0.0, 1.0);
    } else {
      y /= csqrt(d);
    }
    
    // x is the up vector rotated into the YZ plane.  Its angle to the Z
    // axis is the amount to rotate about the X axis to bring the up
    // vector to the Z axis.
    
    LVector2<NumType> x(fwd[1], fwd[0]*y[0]+fwd[2]*y[1]);
    d = dot(x, x);
    if (d==0.0) {
      x = LVector2<NumType>(0.0, 1.0);
    } else {
      x /= csqrt(d);
    }

    // Now apply both rotations to the forward vector.  This will rotate
    // the forward vector by the same amount we would have had to rotate
    // the up vector to bring it to the Z axis.  If the vectors were
    // perpendicular, this will put the forward vector somewhere in the
    // XY plane.
    
    // z is the projection of the newly rotated fwd vector into the XY
    // plane.  Its angle to the Y axis is the amount to rotate about the
    // Z axis in order to bring the fwd vector to the Y axis.
    LVector2<NumType> z(up[0]*y[1] - up[2]*y[0],
			-up[0]*y[0]*x[0] + up[1]*x[1] - up[2]*y[1]*x[0]);
    d = dot(z, z);
    if (d==0.0) {
      z = LVector2<NumType>(0.0, 1.0);
    } else {
      z /= csqrt(d);
    }

    // Now build the net rotation matrix.
    if (cs == CS_yup_right) {
      mat =
	make_z_mat(z) *
	make_x_mat(x) *
	make_y_mat(-y);
    } else { // cs == CS_yup_left
      mat =
	make_z_mat(-z) *
	make_x_mat(-x) *
	make_y_mat(-y);
    }
  }
}

// The following functions are the non-template functions that are
// actually exported.

void
heads_up(LMatrix3f &mat, const LVector3f &fwd,
	 const LVector3f &up, CoordinateSystem cs) {
  _heads_up(mat, fwd, up, cs);
}

void
look_at(LMatrix3f &mat, const LVector3f &fwd,
	const LVector3f &up, CoordinateSystem cs) {
  _look_at(mat, fwd, up, cs);
}

void
heads_up(LMatrix3d &mat, const LVector3d &fwd,
	 const LVector3d &up, CoordinateSystem cs) {
  _heads_up(mat, fwd, up, cs);
}

void
look_at(LMatrix3d &mat, const LVector3d &fwd,
	const LVector3d &up, CoordinateSystem cs) {
  _look_at(mat, fwd, up, cs);
}
