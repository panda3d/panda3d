// Filename: fltnames.h
// Created by:  cxgeorge (04Apr01)
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

#define FLOATTYPE float
#define FLOATNAME(ARG) ARG##f
#define FLOATTOKEN 'f'
#define FLOATCONST(ARG) ARG##f
