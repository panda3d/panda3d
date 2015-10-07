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
typedef int off_t;
typedef long time_t;
typedef long clock_t;

struct FILE;

namespace std {
}

typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;

#ifdef _WIN64
typedef unsigned long long size_t;
typedef long long ssize_t;
typedef long long ptrdiff_t;
#else
typedef unsigned long size_t;
typedef long ssize_t;
typedef long ptrdiff_t;
#endif

#ifdef __cplusplus
#define NULL 0L
#else
#define NULL ((void *)0)
#endif

// One day, we might extend interrogate to be able to parse this,
// but we currently don't need it.
#define alignas(x)

#endif
