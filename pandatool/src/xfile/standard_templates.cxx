// Filename: standard_templates.cxx
// Created by:  drose (04Oct04)
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

#include "standard_templates.h"

// The binary data included here was generated from standardTemplates.x
// (in this directory) file via the utility program bin2c (defined in
// pandatool).  It contains the set of template definitions that must be
// loaded before any standard template file can be properly interpreted.

#ifndef CPPPARSER

#if defined(HAVE_ZLIB)

// If we have zlib available, we can store this file compressed, which
// is much smaller.

// Regenerate this file with:

// pcompress standardTemplates.x standardTemplates.x.pz
// bin2c -n standard_templates_data -o standardTemplates.x.pz.c standardTemplates.x.pz

#include "standardTemplates.x.pz.c"

#else  // HAVE_ZLIB

// If we don't have zlib, just include the whole uncompressed file.

// Regenerate this file with:

// bin2c -n standard_templates_data -o standardTemplates.x.c standardTemplates.x

#include "standardTemplates.x.c"

#endif  // HAVE_ZLIB

#endif  // CPPPARSER

