// Filename: default_font.h
// Created by:  drose (31Jan03)
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

#ifndef DEFAULT_FONT_H
#define DEFAULT_FONT_H

#include "pandabase.h"

#if defined(HAVE_FREETYPE) && defined(COMPILE_IN_DEFAULT_FONT) && !defined(CPPPARSER)

extern const unsigned char default_font_data[];
extern const int default_font_size;

#endif  // HAVE_FREETYPE && COMPILE_IN_DEFAULT_FONT && !CPPPARSER

#endif

