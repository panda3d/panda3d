// Filename: get_config_path.h
// Created by:  drose (01Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GET_CONFIG_PATH_H
#define GET_CONFIG_PATH_H

#include <pandabase.h>

#include <dSearchPath.h>

////////////////////////////////////////////////////////////////////
//     Function: get_config_path
//  Description: A generic function for reading path strings
//               (e.g. model-path, texture-path, etc.) from the Config
//               database.  It automatically handles concatenating
//               together multiple appearances of the indicated
//               variable name as a single long path string.
//
//               static_ptr must be a statically-defined string
//               pointer, unique to each different config_var_name.
//               It should be initialized to NULL.  This will
//               automatically be allocated and filled with the string
//               path the first time this function is called;
//               thereafter, the same string value will be returned.
//               This allows the function to work during static init
//               time when we can't be sure what has or hasn't been
//               already initialized.
////////////////////////////////////////////////////////////////////
EXPCL_PANDAEXPRESS const DSearchPath &
get_config_path(const string &config_var_name, DSearchPath *&static_ptr);

#endif
