// Filename: particleCommonFuncs.h
// Created by:  darren (02Oct00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef PARTICLECOMMONFUNCS_H
#define PARTICLECOMMONFUNCS_H

// evaluates to a float in the range [0,1]
#define NORMALIZED_RAND() ((float)rand() / (float)RAND_MAX)

// linear interpolation
// t is in [0,1]
// result is in [X0,X1]
#define LERP(t,X0,X1) ((X0) + ((t) * ((X1) - (X0))))

// linear t -> cubic t
// t is in [0,1]
// result is in [0,1]
#define CUBIC_T(t) ((t)*(t)*(3-(2*(t))))

// cubic interpolation
// t is in [0,1]
// result is in [X0,X1]
#define CLERP(t,X0,X1) LERP(CUBIC_T(t), (X0), (X1))

// spread calculator
// spread is non-negative spread magnitude
// result is in [-spread,spread]
#define SPREAD(magnitude) ((magnitude) - (NORMALIZED_RAND() * 2.0f * (magnitude)))

// integer spread calculator
// spread is non-negative spread magnitude (integer)
// result is in [-spread,spread]
#define I_SPREAD(magnitude) ((magnitude) - ((int)rand() % ((2*(magnitude))+1)))

#endif // PARTICLECOMMONFUNCS_H
