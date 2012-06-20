// Filename: pystub.h
// Created by:  drose (08Aug00)
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

#ifndef PYSTUB_H
#define PYSTUB_H

#include "dtoolbase.h"

// The sole purpose of this header file is to allow a program other
// than Python to load in a module that includes Python wrappers.

// We need this if we build the Panda libraries with Python wrappers,
// but want to run a standalone program with those libraries.

// This header file just stubs out the Python functions that these
// wrappers will call.  You should include this header file in exactly
// one .C file in your project, preferably in the .C file that defines
// main(), and then link with -lpystub.  Do not include this header
// file in a .C or .h file that will become part of an .so that might
// eventually link with Python.


// You might need to call this function in main() or somewhere to
// force the .so to be linked in--some OS'es try to be smart about not
// pulling in shared libraries whose symbols aren't referenced
// anywhere.
EXPCL_PYSTUB void pystub();


#endif

