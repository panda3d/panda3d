// Filename: stdtypedefs.h
// Created by:  drose (12May00)
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

typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;

#define NULL ((void *)0)

struct FILE;

namespace std {
}

#endif

