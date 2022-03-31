/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ode_includes.h
 * @author joswilso
 * @date 2007-01-30
 */

#ifndef _ODE_INCLUDES_H_
#define _ODE_INCLUDES_H_

#include "pandabase.h"

#ifdef int8
  #define temp_ode_int8 int8
  #undef int8
#endif

#ifdef int32
  #define temp_ode_int32 int32
  #undef int32
#endif

#ifdef uint32
  #define temp_ode_uint32 uint32
  #undef uint32
#endif

#define int8 ode_int8
#define int32 ode_int32
#define uint32 ode_uint32

#include <ode/ode.h>

// These are the ones that conflict with other defines in Panda.  It may be
// necessary to add to this list at a later time.
#undef int8
#undef int32
#undef uint32

#ifdef temp_ode_int8
  #define int8 temp_ode_int8
  #undef temp_ode_int8
#endif

#ifdef temp_ode_int32
  #define int32 temp_ode_int32
  #undef temp_ode_int32
#endif

#ifdef temp_ode_uint32
  #define uint32 temp_ode_uint32
  #undef temp_ode_uint32
#endif


#endif
