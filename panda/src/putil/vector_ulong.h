/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vector_ulong.h
 * @author drose
 * @date 2000-05-10
 */

#ifndef VECTOR_ULONG_H
#define VECTOR_ULONG_H

#include "pandabase.h"

#include "pvector.h"

/**
 * A vector of ushorts.  This class is defined once here, and exported to
 * PANDA.DLL; other packages that want to use a vector of this type (whether
 * they need to export it or not) should include this header file, rather than
 * defining the vector again.
 */

#define EXPCL EXPCL_PANDA_PUTIL
#define EXPTP EXPTP_PANDA_PUTIL
#define TYPE unsigned long
#define NAME vector_ulong

#include "vector_src.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
