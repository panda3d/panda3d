// Filename: rotate_to.cxx
// Created by:  drose (04Nov99)
// 
////////////////////////////////////////////////////////////////////

#include <pandabase.h>
#include "rotate_to.h"
#include "luse.h"

#include <math.h>

////////////////////////////////////////////////////////////////////
//     Function: _rotate_to
//  Description: Computes the matrix necessary to rotate vector a onto
//               vector b.  It is assumed that both vectors are
//               normalized.
////////////////////////////////////////////////////////////////////
template<class NumType>
static void
_rotate_to(LMatrix3<NumType> &mat, 
	   const LVector3<NumType> &a, const LVector3<NumType> &b) {
  NumType cos_theta = a.dot(b);

  LVector3<NumType> axis = a.cross(b);
  NumType sin_theta = length(axis);

  // Check for collinear vectors
  if (sin_theta < 0.0001) {
    // The vectors are collinear.

    if (cos_theta < 0.0) {
      // The vectors are opposite; choose an arbitrary axis
      // perpendicular to a.
      LVector3<NumType> absa(fabs(a[0]), fabs(a[1]), fabs(a[2]));
      LVector3<NumType> lca(0., 0., 0.);
      lca[absa[0]<=absa[1] ? absa[0]<=absa[2] ? 0 : 2
	 : absa[1]<=absa[2] ? 1 : 2] = 1.0;
      
      axis = normalize(a.cross(lca));
    } else {
      mat = LMatrix3<NumType>::ident_mat();
      return;
    }

  } else {
    // The vectors are not collinear; determine the best axis.
    axis /= sin_theta;
  }

  NumType x = axis[0];
  NumType y = axis[1];
  NumType z = axis[2];
    
  NumType t = 1.0 - cos_theta;

  mat(0, 0) = t * x * x + cos_theta;
  mat(0, 1) = t * x * y + sin_theta * z;
  mat(0, 2) = t * x * z - sin_theta * y;

  mat(1, 0) = t * y * x - sin_theta * z;
  mat(1, 1) = t * y * y + cos_theta;
  mat(1, 2) = t * y * z + sin_theta * x;

  mat(2, 0) = t * z * x + sin_theta * y;
  mat(2, 1) = t * z * y - sin_theta * x;
  mat(2, 2) = t * z * z + cos_theta;
}


void
rotate_to(LMatrix3f &mat, const LVector3f &a, const LVector3f &b) {
  _rotate_to(mat, a, b);
}

void
rotate_to(LMatrix3d &mat, const LVector3d &a, const LVector3d &b) {
  _rotate_to(mat, a, b);
}

void
rotate_to(LMatrix4f &mat, const LVector3f &a, const LVector3f &b) {
  LMatrix3f m3;
  _rotate_to(m3, a, b);
  mat = LMatrix4f(m3);
}

void
rotate_to(LMatrix4d &mat, const LVector3d &a, const LVector3d &b) {
  LMatrix3d m3;
  _rotate_to(m3, a, b);
  mat = LMatrix4d(m3);
}
