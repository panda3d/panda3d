/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file compose_matrix_src.I
 * @author drose
 * @date 1999-02-21
 */

/**
 * Computes the 4x4 matrix according to scale, shear, rotation, and
 * translation.
 */
INLINE_LINMATH void
compose_matrix(FLOATNAME(LMatrix4) &mat,
               const FLOATNAME(LVecBase3) &scale,
               const FLOATNAME(LVecBase3) &shear,
               const FLOATNAME(LVecBase3) &hpr,
               const FLOATNAME(LVecBase3) &translate,
               CoordinateSystem cs) {
  FLOATNAME(LMatrix3) upper3;
  compose_matrix(upper3, scale, shear, hpr, cs);
  mat = FLOATNAME(LMatrix4)(upper3, translate);
}

/**
 * Computes the 4x4 matrix according to scale, shear, rotation, and
 * translation.
 */
INLINE_LINMATH void
compose_matrix(FLOATNAME(LMatrix4) &mat,
               const FLOATTYPE components[num_matrix_components],
               CoordinateSystem cs) {
  FLOATNAME(LVector3) scale(components[0],
                            components[1],
                            components[2]);
  FLOATNAME(LVector3) shear(components[3],
                            components[4],
                            components[5]);
  FLOATNAME(LVector3) hpr(components[6],
                          components[7],
                          components[8]);
  FLOATNAME(LVector3) translate(components[9],
                                components[10],
                                components[11]);
  compose_matrix(mat, scale, shear, hpr, translate, cs);
}

/**
 * Extracts out the components of an affine matrix.  Returns true if the
 * scale, shear, hpr, and translate completely describe the matrix, or false
 * if the matrix is not affine.
 */
INLINE_LINMATH bool
decompose_matrix(const FLOATNAME(LMatrix4) &mat,
                 FLOATNAME(LVecBase3) &scale,
                 FLOATNAME(LVecBase3) &shear,
                 FLOATNAME(LVecBase3) &hpr,
                 FLOATNAME(LVecBase3) &translate,
                 CoordinateSystem cs) {
  // Get the translation first.
  mat.get_row3(translate, 3);
  if (!decompose_matrix(mat.get_upper_3(), scale, shear, hpr, cs)) {
    return false;
  }
#ifndef NDEBUG
  return mat.get_col(3).almost_equal(FLOATNAME(LVecBase4)(0.0, 0.0, 0.0, 1.0));
#else
  return true;
#endif
}

/**
 * Extracts out the components of an affine matrix.  Returns true if the
 * scale, shear, hpr, and translate completely describe the matrix, or false
 * if the matrix is not affine.
 */
INLINE_LINMATH bool
decompose_matrix(const FLOATNAME(LMatrix4) &mat,
                 FLOATTYPE components[num_matrix_components],
                 CoordinateSystem cs) {
  FLOATNAME(LVector3) scale, shear, hpr, translate;
  bool result = decompose_matrix(mat, scale, shear, hpr, translate, cs);
  components[0] = scale[0];
  components[1] = scale[1];
  components[2] = scale[2];
  components[3] = shear[0];
  components[4] = shear[1];
  components[5] = shear[2];
  components[6] = hpr[0];
  components[7] = hpr[1];
  components[8] = hpr[2];
  components[9] = translate[0];
  components[10] = translate[1];
  components[11] = translate[2];
  return result;
}

// The following functions are deprecated; they have been replaced with new
// versions, above, that accept a shear component as well.


// Deprecated function.
INLINE_LINMATH void
compose_matrix(FLOATNAME(LMatrix3) &mat,
               const FLOATNAME(LVecBase3) &scale,
               const FLOATNAME(LVecBase3) &hpr,
               CoordinateSystem cs) {
  compose_matrix(mat, scale, FLOATNAME(LVecBase3)(0, 0, 0), hpr, cs);
}

// Deprecated function.
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

// Deprecated function.
INLINE_LINMATH bool
decompose_matrix(const FLOATNAME(LMatrix3) &mat,
                 FLOATNAME(LVecBase3) &scale,
                 FLOATNAME(LVecBase3) &hpr,
                 CoordinateSystem cs) {
  FLOATNAME(LVecBase3) shear;
  if (!decompose_matrix(mat, scale, shear, hpr, cs)) {
    return false;
  }
  return shear.almost_equal(FLOATNAME(LVecBase3)::zero());
}

// Deprecated function.
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
