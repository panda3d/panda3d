// Filename: load_dso.h
// Created by:  drose (12May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LOAD_DSO_H
#define LOAD_DSO_H

#include <dtoolbase.h>

#include "filename.h"

// Loads in a dynamic library like an .so or .dll.  Returns NULL if
// failure, otherwise on success.

EXPCL_DTOOL void *
load_dso(const Filename &filename);

// Returns the error message from the last failed load_dso() call.

EXPCL_DTOOL string
load_dso_error();

#endif

