/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vector_float.h
 * @author drose
 * @date 2000-05-10
 */

#ifndef VECTOR_FLOAT_H
#define VECTOR_FLOAT_H

#include "dtoolbase.h"

/**
 * A vector of floats.  This class is defined once here, and exported to
 * DTOOL.DLL; other packages that want to use a vector of this type (whether
 * they need to export it or not) should include this header file, rather than
 * defining the vector again.
 */

#define EXPCL EXPCL_DTOOL_DTOOLUTIL
#define EXPTP EXPTP_DTOOL_DTOOLUTIL
#define TYPE float
#define NAME vector_float

#include "vector_src.h"

#endif
