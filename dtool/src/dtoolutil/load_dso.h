/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file load_dso.h
 * @author drose
 * @date 2000-05-12
 */

#ifndef LOAD_DSO_H
#define LOAD_DSO_H

#include "dtoolbase.h"
#include "dSearchPath.h"
#include "filename.h"

// Loads in a dynamic library like an .so or .dll.  Returns NULL if failure,
// otherwise on success.  If the filename is not absolute, searches the path.
// If the path is empty, searches the dtool directory.

EXPCL_DTOOL_DTOOLUTIL void *
load_dso(const DSearchPath &path, const Filename &filename);

// true indicates success
EXPCL_DTOOL_DTOOLUTIL bool
unload_dso(void *dso_handle);

// Returns the error message from the last failed load_dso() call.

EXPCL_DTOOL_DTOOLUTIL std::string
load_dso_error();

// Returns a function pointer or other symbol from a loaded library.
EXPCL_DTOOL_DTOOLUTIL void *
get_dso_symbol(void *handle, const std::string &name);

#endif
