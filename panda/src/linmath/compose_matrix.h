// Filename: compose_matrix.h
// Created by:  drose (27Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef COMPOSE_MATRIX_H
#define COMPOSE_MATRIX_H

////////////////////////////////////////////////////////////////////
//
// compose_matrix(), decompose_matrix()
//
// These two functions build and/or extract an affine matrix into
// its constituent parts: scale, hpr, and translate.
//
// There are also two additional flavors for 3x3 matrices.  These are
// treated as the upper 3x3 part of a general 4x4 matrix, and so can
// only represent rotations and scales.
//
////////////////////////////////////////////////////////////////////

#include <pandabase.h>
#include <math.h>
#include "lmatrix.h"
#include "luse.h"

#include "fltnames.h"
#include "compose_matrix_src.h"

#include "dblnames.h"
#include "compose_matrix_src.h"

#endif

