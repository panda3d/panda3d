// Filename: load_dso.h
// Created by:  drose (12May00)
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

#ifndef LOAD_DSO_H
#define LOAD_DSO_H

#include "dtoolbase.h"

#include "filename.h"

// Loads in a dynamic library like an .so or .dll.  Returns NULL if
// failure, otherwise on success.

EXPCL_DTOOL void *
load_dso(const Filename &filename);

// true indicates success
EXPCL_DTOOL bool
unload_dso(void *dso_handle);

// Returns the error message from the last failed load_dso() call.

EXPCL_DTOOL string
load_dso_error();

#endif

