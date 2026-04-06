/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vector_writable.h
 * @author jason
 * @date 2000-06-14
 */

#ifndef VECTOR_WRITABLE_H
#define VECTOR_WRITABLE_H

#include "pandabase.h"

#include "pvector.h"

class Writable;

/**
 * A vector of Writable *.  This class is defined once here, and exported to
 * PANDA.DLL; other packages that want to use a vector of this type (whether
 * they need to export it or not) should include this header file, rather than
 * defining the vector again.
 */

#define EXPCL EXPCL_PANDA_PUTIL
#define EXPTP EXPTP_PANDA_PUTIL
#define TYPE Writable *
#define NAME vector_writable

#include "vector_src.h"

#endif
