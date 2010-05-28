// Filename: glpure.cxx
// Created by:  drose (09Feb04)
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

#include "pandabase.h"

// This is the one file in this directory which is actually compiled.
// It exists just so we can have some symbols and make the compiler
// happy.

#if (defined(WIN32_VC) || defined(WIN64_VC)) && !defined(CPPPARSER) && !defined(LINK_ALL_STATIC)
__declspec(dllexport)
#endif
int glpure;

