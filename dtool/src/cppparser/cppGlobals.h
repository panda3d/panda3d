// Filename: cppGlobals.h
// Created by:  drose (16May00)
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

#ifndef CPPGLOBALS_H
#define CPPGLOBALS_H

#include "dtoolbase.h"

// Some compilers (notably VC++) define a special keyword to represent
// a 64-bit integer, but don't recognize "long long int".  To parse
// (and generate) code for these compilers, set this string to the
// 64-bit integer typename keyword.
extern string cpp_longlong_keyword;


#endif


