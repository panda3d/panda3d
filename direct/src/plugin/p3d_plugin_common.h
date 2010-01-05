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
// that contribute to p3d_plugin; it provides some common symbol
// declarations.

#define P3D_FUNCTION_PROTOTYPES
//#define BUILDING_P3D_PLUGIN
#define TIXML_USE_STL

// It's a good idea to pick up this header file, even though we don't
// actually link with dtool.  This header file defines useful
// system-wide config settings.
#include "dtool_config.h"

#include "p3d_plugin.h"
#include "p3d_lock.h"

#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>

using namespace std;

// Appears in p3dInstanceManager.cxx.
extern ostream *nout_stream;
#define nout (*nout_stream)

// Appears in p3d_plugin.cxx.
extern LOCK _api_lock;

// A convenience function for formatting a generic P3D_object to an
// ostream.
inline ostream &
operator << (ostream &out, P3D_object &value) {
  int size = P3D_OBJECT_GET_REPR(&value, NULL, 0);
  char *buffer = new char[size];
  P3D_OBJECT_GET_REPR(&value, buffer, size);
  out.write(buffer, size);
  delete[] buffer;

  return out;
}

#endif

