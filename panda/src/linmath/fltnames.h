// Filename: fltnames.h
// Created by:  cxgeorge (04Apr01)
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

#define FLOATTYPE float
#define FLOATNAME(ARG) ARG##f
#define FLOATTOKEN 'f'
