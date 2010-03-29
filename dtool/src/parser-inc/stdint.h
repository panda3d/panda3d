// Filename: stdint.h
// Created by:  rdb (29Mar10)
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

#ifndef _STDINT_H
#define _STDINT_H

#ifdef _LP64
#define __WORDSIZE 64
#else
#define __WORDSIZE 32
#endif

typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;
#if __WORDSIZE == 64
typedef long int int64_t;
#else
typedef long long int int64_t;
#endif

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
#if __WORDSIZE == 64
typedef unsigned long int uint64_t;
#else
typedef unsigned long long int uint64_t;
#endif

#endif

