// Filename: compose_matrix.cxx
// Created by:  drose (27Jan99)
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
