// Filename: stdtypedefs.h
// Created by:  drose (12May00)
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

// This file, and all the other files in this directory, aren't
// intended to be compiled--they're just parsed by CPPParser (and
// interrogate) in lieu of the actual system headers, to generate the
// interrogate database.

#ifndef STDTYPEDEFS_H
#define STDTYPEDEFS_H

typedef unsigned int size_t;
typedef int off_t;
typedef unsigned int time_t;
typedef int ptrdiff_t;
typedef int clock_t;
typedef int wchar_t;

typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;

#define NULL ((void *)0)

struct FILE;

namespace std {
}

#endif

