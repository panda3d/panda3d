/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pre_fcollada_include.h
 * @author rdb
 * @date 2008-10-04
 */

// This file defines some stuff that need to be defined before one includes
// FCollada.h

#ifndef PRE_FCOLLADA_INCLUDE_H
#define PRE_FCOLLADA_INCLUDE_H

#ifdef FCOLLADA_VERSION
  #error You must include pre_fcollada_include.h before including FCollada.h!
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <winsock2.h>
#endif

// FCollada expects LINUX to be defined on linux
#ifdef IS_LINUX
  #ifndef LINUX
    #define LINUX
  #endif
#endif

#define NO_LIBXML
#define FCOLLADA_NOMINMAX

// FCollada does use global min/max.
using std::min;
using std::max;

#endif
