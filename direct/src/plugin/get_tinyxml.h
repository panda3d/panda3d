// Filename: get_tinyxml.h
// Created by:  drose (01Jul09)
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

#ifndef GET_TINYXML_H
#define GET_TINYXML_H

// This header file exists just to include tinyxml.h safely.  We need
// this since tinyxml.h requires having the symbol TIXML_USE_STL
// already defined before you include it.

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#include <tinyxml.h>

#endif
