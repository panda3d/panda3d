// Filename: vector_string.cxx
// Created by:  drose (15May00)
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

#include "vector_string.h"

#define EXPCL EXPCL_DTOOLCONFIG
#define EXPTP EXPTP_DTOOLCONFIG
#define TYPE std::string
#define NAME vector_string

#include "vector_src.cxx"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif
