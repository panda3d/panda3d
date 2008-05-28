// Filename: compose_matrix.cxx
// Created by:  drose (27Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "deg_2_rad.h"
#include "config_linmath.h"
#include "compose_matrix.h"

#include "fltnames.h"
#include "compose_matrix_src.cxx"

#include "dblnames.h"
#include "compose_matrix_src.cxx"


const char * const matrix_component_letters = "ijkabchprxyz";
const double matrix_component_defaults[num_matrix_components] = {
  1.0, 1.0, 1.0,
  0.0, 0.0, 0.0,
  0.0, 0.0, 0.0,
  0.0, 0.0, 0.0
};
