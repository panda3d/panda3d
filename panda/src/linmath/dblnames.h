// Filename: dblnames.h
// Created by:  cxgeorge (04Apr01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//
// This file is used throughout this directory, in conjunction with
// dblnames.h, to implement a poor man's template of the linmath
// objects on numeric type.  The idea is to #include either fltnames.h
// or dblnames.h (or, in theory, intnames.h or any other numeric type
// we wanted to implement) and then to include the various *_src.h
// and/or *_src.cxx files that actually define the linmath objects.
//
// We do this instead of using actual templates to avoid some of the
// inherent problems with templates: compiler complexity and
// distributed code bloat, for instance; plus it allows us to
// implement #if-based specialization on numeric type for compilers
// (like VC++) that don't completely support template specialization.
// That and the fact that VC++ seems to have a particularly bad time
// with templates in general.
//
////////////////////////////////////////////////////////////////////


#undef FLOATTYPE
#undef FLOATNAME
#undef FLOATTOKEN
#undef FLOATCONST
#undef FLOATTYPE_IS_INT
#undef STRINGIFY
#undef FLOATNAME_STR

#define FLOATTYPE double
#define FLOATNAME(ARG) ARG##d
#define FLOATTOKEN 'd'
#define FLOATCONST(ARG) ARG

#define STRINGIFY(ARG) #ARG
#define FLOATNAME_STR(ARG) STRINGIFY(ARG##d)
