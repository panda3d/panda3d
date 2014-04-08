// Filename: Python.h
// Created by:  drose (12May00)
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

// This file, and all the other files in this directory, aren't
// intended to be compiled--they're just parsed by CPPParser (and
// interrogate) in lieu of the actual system headers, to generate the
// interrogate database.

#ifndef PYTHON_H
#define PYTHON_H

class PyObject;
class PyThreadState;
typedef int Py_ssize_t;
struct Py_buffer;

// This file defines PY_VERSION_HEX, which is used in some places.
#include "patchlevel.h"

#endif  // PYTHON_H
