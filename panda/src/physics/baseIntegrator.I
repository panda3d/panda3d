// Filename: baseIntegrator.I
// Created by:  charles (11Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Function : get_precomputed_linear_matrices
//   Access : protected
////////////////////////////////////////////////////////////////////
INLINE const pvector< LMatrix4f > &BaseIntegrator::
get_precomputed_linear_matrices() const {
  return _precomputed_linear_matrices;
}

////////////////////////////////////////////////////////////////////
// Function : get_precomputed_angular_matrices
//   Access : protected
////////////////////////////////////////////////////////////////////
INLINE const pvector< LMatrix4f > &BaseIntegrator::
get_precomputed_angular_matrices() const {
  return _precomputed_angular_matrices;
}
