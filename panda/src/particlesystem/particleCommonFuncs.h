// Filename: particleCommonFuncs.h
// Created by:  darren (02Oct00)
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

#endif // PARTICLECOMMONFUNCS_H
