// Filename: lmatrix.h
// Created by:  drose (15Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef LMATRIX_H
#define LMATRIX_H

#include <pandabase.h>
#include "config_linmath.h"

#include "lmatrix3.h"
#include "lmatrix4.h"

/*
typedef LMatrix3<float> LMatrix3f;
typedef LMatrix4<float> LMatrix4f;

typedef LMatrix3<double> LMatrix3d;
typedef LMatrix4<double> LMatrix4d;
*/

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
