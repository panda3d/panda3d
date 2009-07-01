// Filename: p3d_plugin_common.h
// Created by:  drose (29May09)
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

#ifndef P3D_PLUGIN_COMMON
#define P3D_PLUGIN_COMMON

// This header file is included by all C++ files in this directory
// that contribute to p3d_plugin; provides some common symbol
// declarations.

#define P3D_FUNCTION_PROTOTYPES
#define BUILDING_P3D_PLUGIN
#define TIXML_USE_STL

#include "p3d_plugin.h"
#include "p3d_lock.h"

#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>

using namespace std;

// Appears in p3d_plugin.cxx.
extern ofstream log;
extern string plugin_output_filename;
extern ostream *nout_stream;
#define nout (*nout_stream)

#endif

