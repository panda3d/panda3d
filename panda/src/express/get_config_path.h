// Filename: get_config_path.h
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

#ifndef GET_CONFIG_PATH_H
#define GET_CONFIG_PATH_H

#include "pandabase.h"

#include "dSearchPath.h"

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
EXPCL_PANDAEXPRESS DSearchPath &
get_config_path(const string &config_var_name, DSearchPath *&static_ptr);

#endif
