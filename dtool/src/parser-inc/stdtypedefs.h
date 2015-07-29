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
typedef unsigned long size_t;
typedef long ssize_t;
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

#ifdef __cplusplus
#define NULL 0L
#else
#define NULL ((void *)0)
#endif

// One day, we might extend interrogate to be able to parse this,
// but we currently don't need it.
#define alignas(x)

#endif
