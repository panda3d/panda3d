////////////////////////////////////////////////////////////////////
// Filename: compose_matrix_src.h
// Created by:  drose (21Feb99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: compose_matrix
//  Description: Computes the 4x4 matrix according to scale, rotation,
//               and translation.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH void
compose_matrix(FLOATNAME(LMatrix4) &mat,
               const FLOATNAME(LVecBase3) &scale,
               const FLOATNAME(LVecBase3) &hpr,
               const FLOATNAME(LVecBase3) &translate,
               CoordinateSystem cs) {
  FLOATNAME(LMatrix3) upper3;
  compose_matrix(upper3, scale, hpr, cs);
  mat = FLOATNAME(LMatrix4)(upper3, translate);
}

INLINE_LINMATH void
compose_matrix(FLOATNAME(LMatrix4) &mat,
               const FLOATTYPE components[9],
               CoordinateSystem cs) {
  FLOATNAME(LVector3) scale(components[0],
                            components[1],
                            components[2]);
  FLOATNAME(LVector3) hpr(components[3],
                          components[4],
                          components[5]);
  FLOATNAME(LVector3) translate(components[6],
                                components[7],
                                components[8]);
  compose_matrix(mat, scale, hpr, translate, cs);
}


////////////////////////////////////////////////////////////////////
//     Function: decompose_matrix
//  Description: Extracts out the components of an affine matrix.
//               Returns true if the scale, hpr, translate
//               completely describe the matrix, or false if there is
//               also a shear component or if the matrix is not
//               affine.
////////////////////////////////////////////////////////////////////
INLINE_LINMATH bool
decompose_matrix(const FLOATNAME(LMatrix4) &mat,
                 FLOATNAME(LVecBase3) &scale,
                 FLOATNAME(LVecBase3) &hpr,
                 FLOATNAME(LVecBase3) &translate,
                 CoordinateSystem cs) {
  // Get the translation first.
  mat.get_row3(translate,3);
  return decompose_matrix(mat.get_upper_3(), scale, hpr, cs);
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
INLINE_LINMATH bool
decompose_matrix(const FLOATNAME(LMatrix4) &mat,
                 FLOATNAME(LVecBase3) &scale,
                 FLOATNAME(LVecBase3) &hpr,
                 FLOATNAME(LVecBase3) &translate,
                 FLOATTYPE roll,
                 CoordinateSystem cs) {
  // Get the translation first.
  mat.get_row3(translate,3);
  return decompose_matrix(mat.get_upper_3(), scale, hpr, roll, cs);
}

INLINE_LINMATH bool
decompose_matrix(const FLOATNAME(LMatrix4) &mat,
                 FLOATTYPE components[9],
                 CoordinateSystem cs) {
  FLOATNAME(LVector3) scale, hpr, translate;
  if (!decompose_matrix(mat, scale, hpr, translate, cs)) {
    return false;
  }
  components[0] = scale[0];
  components[1] = scale[1];
  components[2] = scale[2];
  components[3] = hpr[0];
  components[4] = hpr[1];
  components[5] = hpr[2];
  components[6] = translate[0];
  components[7] = translate[1];
  components[8] = translate[2];
  return true;
}
