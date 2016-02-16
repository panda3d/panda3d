/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file baseIntegrator.I
 * @author charles
 * @date 2000-08-11
 */

////////////////////////////////////////////////////////////////////
//     Function: get_precomputed_linear_matrices
//       Access: Protected
////////////////////////////////////////////////////////////////////
INLINE const BaseIntegrator::MatrixVector &BaseIntegrator::
get_precomputed_linear_matrices() const {
  return _precomputed_linear_matrices;
}

////////////////////////////////////////////////////////////////////
//     Function: get_precomputed_angular_matrices
//       Access: Protected
////////////////////////////////////////////////////////////////////
INLINE const BaseIntegrator::MatrixVector &BaseIntegrator::
get_precomputed_angular_matrices() const {
  return _precomputed_angular_matrices;
}
