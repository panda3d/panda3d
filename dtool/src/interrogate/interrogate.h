// Filename: interrogate.h
// Created by:  drose (31Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef INTERROGATE_H
#define INTERROGATE_H

#include "dtoolbase.h"

#include "cppParser.h"
#include "cppVisibility.h"
#include "filename.h"

extern CPPParser parser;

// A few global variables that control the interrogate process.
extern Filename output_code_filename;
extern Filename output_data_filename;
extern string output_data_basename;
extern bool output_module_specific;
extern bool output_function_pointers;
extern bool output_function_names;
extern bool convert_strings;
extern bool manage_reference_counts;
extern bool watch_asserts;
extern bool true_wrapper_names;
extern bool build_c_wrappers;
extern bool build_python_wrappers;
extern bool build_python_obj_wrappers;
extern bool track_interpreter;
extern bool save_unique_names;
extern bool no_database;
extern bool generate_spam;
extern bool left_inheritance_requires_upcast;
extern CPPVisibility min_vis;
extern string library_name;
extern string module_name;

#endif

