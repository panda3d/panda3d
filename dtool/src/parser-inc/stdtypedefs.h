// Filename: stdtypedefs.h
// Created by:  drose (12May00)
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

// This file, and all the other files in this directory, aren't
// intended to be compiled--they're just parsed by CPPParser (and
// interrogate) in lieu of the actual system headers, to generate the
// interrogate database.

#ifndef STDTYPEDEFS_H
#define STDTYPEDEFS_H
#ifndef __APPLE__
typedef unsigned int size_t;
typedef int ssize_t;
typedef int off_t;
typedef unsigned int time_t;
typedef int clock_t;

struct FILE;

namespace std {
}
#endif
typedef int ptrdiff_t;

typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;

#define NULL ((void *)0)

typedef int fd_set;

#endif

