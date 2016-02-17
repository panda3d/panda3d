/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file frustum.h
 * @author mike
 * @date 1997-01-09
 */

#ifndef FRUSTUM_H
#define FRUSTUM_H

// Includes
#include "pandabase.h"
#include <math.h>
#include "luse.h"
#include "config_mathutil.h"

#include "fltnames.h"
#include "frustum_src.h"

#include "dblnames.h"
#include "frustum_src.h"

#ifndef STDFLOAT_DOUBLE
typedef LFrustumf LFrustum;
#else
typedef LFrustumd LFrustum;
#endif

// Bogus typedefs for interrogate and legacy Python code.
#ifdef CPPPARSER
typedef LFrustumf FrustumF;
typedef LFrustumd FrustumD;
#ifndef STDFLOAT_DOUBLE
typedef LFrustumf Frustum;
#else
typedef LFrustumd Frustum;
#endif
#endif  // CPPPARSER

#endif
