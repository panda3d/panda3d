/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stdtypedefs.h
 * @author drose
 * @date 2000-05-12
 */

// This file, and all the other files in this directory, aren't
// intended to be compiled--they're just parsed by CPPParser (and
// interrogate) in lieu of the actual system headers, to generate the
// interrogate database.

#ifndef STDTYPEDEFS_H
#define STDTYPEDEFS_H

typedef long time_t;
typedef long clock_t;

#ifdef _WIN64
#define __SIZE_TYPE__ unsigned long long
#define __PTRDIFF_TYPE__ long long
#else
#define __SIZE_TYPE__ unsigned long
#define __PTRDIFF_TYPE__ long
#endif

inline namespace std {
  typedef __SIZE_TYPE__ size_t;
  typedef __PTRDIFF_TYPE__ ssize_t;
  typedef __PTRDIFF_TYPE__ ptrdiff_t;
}

struct timeval;

#ifdef __cplusplus
#define NULL 0L
#else
#define NULL ((void *)0)
#endif
namespace std {
  typedef decltype(nullptr) nullptr_t;
}

#endif
