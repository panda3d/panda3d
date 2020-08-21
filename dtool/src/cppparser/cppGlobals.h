/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cppGlobals.h
 * @author drose
 * @date 2000-05-16
 */

#ifndef CPPGLOBALS_H
#define CPPGLOBALS_H

#include "dtoolbase.h"

// Some compilers (notably VC++) define a special keyword to represent a
// 64-bit integer, but don't recognize "long long int".  To parse (and
// generate) code for these compilers, set this string to the 64-bit integer
// typename keyword.
extern std::string cpp_longlong_keyword;


#endif
