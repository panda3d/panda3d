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
// The functions here come in two flavors, for doubles and floats.
// They're actually implemented as template functions so could be
// instantiated on any numeric type, but I only bothered to
// instantiate and expose these two.  It saves on some cross-library
// pre-linking stuff to have all the templates be hidden.
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

#include "fltnames.I"
#include "compose_matrix.I"

#include "dblnames.I"
#include "compose_matrix.I"

#endif

