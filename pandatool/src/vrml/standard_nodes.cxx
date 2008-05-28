// Filename: standard_nodes.cxx
// Created by:  drose (01Oct04)
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

#include "standard_nodes.h"

// The binary data included here was generated from standardNodes.wrl
// (in this directory) file via the utility program bin2c (defined in
// pandatool).  It contains the set of VRML definitions that must be
// loaded before any standard VRML file can be properly interpreted.

#ifndef CPPPARSER

#if defined(HAVE_ZLIB)

// If we have zlib available, we can store this file compressed, which
// is much smaller.

// Regenerate this file with:

// pcompress standardNodes.wrl standardNodes.wrl.pz
// bin2c -n standard_nodes_data -o standardNodes.wrl.pz.c standardNodes.wrl.pz

#include "standardNodes.wrl.pz.c"

#else  // HAVE_ZLIB

// If we don't have zlib, just include the whole uncompressed file.

// Regenerate this file with:

// bin2c -n standard_nodes_data -o standardNodes.wrl.c standardNodes.wrl

#include "standardNodes.wrl.c"

#endif  // HAVE_ZLIB

#endif  // CPPPARSER
