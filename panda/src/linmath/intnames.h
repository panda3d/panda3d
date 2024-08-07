/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file intnames.h
 * @author rdb
 * @date 2014-06-07
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

#define FLOATTYPE int
#define FLOATNAME(ARG) ARG##i
#define FLOATTOKEN 'i'
#define FLOATCONST(ARG) ARG
#define FLOATTYPE_IS_INT

#define STRINGIFY(ARG) #ARG
#define FLOATNAME_STR(ARG) STRINGIFY(ARG##i)

#define FLOATTYPE_REPR(v, str) (snprintf((str), 12, "%d", (v)))
