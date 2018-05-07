/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vector_uchar.h
 * @author drose
 * @date 2000-05-10
 */

#ifndef VECTOR_UCHAR_H
#define VECTOR_UCHAR_H

#include "dtoolbase.h"

/**
 * A vector of uchars.  This class is defined once here, and exported to
 * DTOOL.DLL; other packages that want to use a vector of this type
 * (whether they need to export it or not) should include this header file,
 * rather than defining the vector again.
 */

#define EXPCL EXPCL_DTOOL_DTOOLUTIL
#define EXPTP EXPTP_DTOOL_DTOOLUTIL
#define TYPE unsigned char
#define NAME vector_uchar

#include "vector_src.h"

#endif
