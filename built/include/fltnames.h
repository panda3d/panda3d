/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltnames.h
 * @author cxgeorge
 * @date 2001-04-04
 */

/*
 * This file is used throughout this directory, in conjunction with
 * dblnames.h, to implement a poor man's template of the linmath objects on
 * numeric type.  The idea is to #include either fltnames.h or dblnames.h (or,
 * in theory, intnames.h or any other numeric type we wanted to implement) and
 * then to include the various *_src.h andor *_src.cxx files that actually
 * define the linmath objects.  We do this instead of using actual templates
 * to avoid some of the inherent problems with templates: compiler complexity
 * and distributed code bloat, for instance; plus it allows us to implement
 * if-based specialization on numeric type for compilers (like VC++) that
 * don't completely support template specialization.  That and the fact that
 * VC++ seems to have a particularly bad time with templates in general.
 */


#undef FLOATTYPE
#undef FLOATNAME
#undef FLOATTOKEN
#undef FLOATCONST
#undef FLOATTYPE_IS_INT
#undef STRINGIFY
#undef FLOATNAME_STR
#undef FLOATTYPE_REPR

#define FLOATTYPE float
#define FLOATNAME(ARG) ARG##f
#define FLOATTOKEN 'f'
#define FLOATCONST(ARG) ARG##f

#define STRINGIFY(ARG) #ARG
#define FLOATNAME_STR(ARG) STRINGIFY(ARG##f)

#define FLOATTYPE_REPR(v, str) do { \
  float v_copy = (v); \
  char *into_str = (str); \
  if (v_copy < 1e16f && v_copy > -1e16f && \
      (float)(long long)v_copy == v_copy) { \
    snprintf(into_str, 32, "%lld", (long long)v_copy); \
  } else { \
    pftoa(v_copy, into_str); \
  } \
} while (0)

#include "pdtoa.h"
