// Filename: standard_nodes.cxx
// Created by:  drose (01Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
