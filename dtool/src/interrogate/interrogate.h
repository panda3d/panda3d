// Filename: interrogate.h
// Created by:  drose (31Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef INTERROGATE_H
#define INTERROGATE_H

#include <dtoolbase.h>

#include <cppParser.h>
#include <cppVisibility.h>
#include <filename.h>

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
extern bool save_unique_names;
extern bool no_database;
extern bool generate_spam;
extern bool left_inheritance_requires_upcast;
extern CPPVisibility min_vis;
extern string library_name;
extern string module_name;

#endif

