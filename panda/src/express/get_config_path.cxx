// Filename: get_config_path.cxx
// Created by:  drose (01Jul00)
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



#include "config_express.h"

#include "executionEnvironment.h"
#include "get_config_path.h"


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
DSearchPath &
get_config_path(const string &config_var_name, DSearchPath *&static_ptr) {
  if (static_ptr == (DSearchPath *)NULL) {
    static_ptr = new DSearchPath;

    Config::ConfigTable::Symbol all_defs;
    config_express.GetAll(config_var_name, all_defs);
    if (all_defs.empty()) {
      // If the path is undefined, it is implicitly ".".
      (*static_ptr).append_path(".");

    } else {
      Config::ConfigTable::Symbol::reverse_iterator si =
        all_defs.rbegin();
      string filename = ExecutionEnvironment::expand_string((*si).Val());
      (*static_ptr).append_path(Filename::from_os_specific(filename), "");
      ++si;
      while (si != all_defs.rend()) {
        string filename = ExecutionEnvironment::expand_string((*si).Val());
        (*static_ptr).append_path(Filename::from_os_specific(filename), "");
        ++si;
      }
    }
    if (express_cat.is_debug()) {
      express_cat.debug()
        << config_var_name << " is " << *static_ptr << "\n";
    }
  }

  return *static_ptr;
}
