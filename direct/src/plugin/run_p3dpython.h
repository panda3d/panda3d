// Filename: run_p3dpython.h
// Created by:  drose (29Aug09)
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

#ifndef RUN_P3DPYTHON_H
#define RUN_P3DPYTHON_H

// This header file defines the prototype for run_p3dpython(), the
// main entry point to this DLL.

#include "fhandle.h"

#ifdef _WIN32
#define EXPCL_P3DPYTHON __declspec(dllexport)
#else
#define EXPCL_P3DPYTHON
#endif

typedef bool 
run_p3dpython_func(const char *program_name, const char *archive_file,
                   FHandle input_handle, FHandle output_handle, 
                   const char *log_pathname, bool interactive_console);

extern "C" EXPCL_P3DPYTHON bool
run_p3dpython(const char *program_name, const char *archive_file,
              FHandle input_handle, FHandle output_handle, 
              const char *log_pathname, bool interactive_console);

#endif

