// Filename: pre_collada_include.h
// Created by:  rdb (23May11)
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

// This header file should be included before including any of the
// COLLADA DOM headers.  It should only be included in a .cxx file
// (not in a header file) and no Panda3D headers should be included
// after the pre_collada_include.h include.

#ifdef PRE_COLLADA_INCLUDE_H
#error Don't include any Panda headers after including pre_collada_include.h!
#endif
#define PRE_COLLADA_INCLUDE_H

// Undef some macros that conflict with COLLADA.
#undef INLINE
#undef tolower

#include <dae.h>
