// Filename: get_config_path.cxx
// Created by:  drose (01Jul00)
// 
////////////////////////////////////////////////////////////////////

#include "get_config_path.h"
#include "config_express.h"


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
const DSearchPath &
get_config_path(const string &config_var_name, DSearchPath *&static_ptr) {
  if (static_ptr == (DSearchPath *)NULL) {
    static_ptr = new DSearchPath;

    Config::ConfigTable::Symbol all_defs;
    config_express.GetAll(config_var_name, all_defs);
    if (!all_defs.empty()) {
      Config::ConfigTable::Symbol::reverse_iterator si =
	all_defs.rbegin();
      (*static_ptr).append_path((*si).Val());
      ++si;
      while (si != all_defs.rend()) {
	(*static_ptr).append_path((*si).Val());
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
