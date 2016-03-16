/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file memoryUsagePointers_ext.h
 * @author rdb
 * @date 2013-12-10
 */

#ifndef MEMORYUSAGEPOINTERS_EXT_H
#define MEMORYUSAGEPOINTERS_EXT_H

#include "dtoolbase.h"

#if defined(HAVE_PYTHON) && defined(DO_MEMORY_USAGE)

#include "extension.h"
#include "memoryUsagePointers.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for VirtualFileSystem, which are
 * called instead of any C++ methods with the same prototype.
 */
template<>
class Extension<MemoryUsagePointers> : public ExtensionBase<MemoryUsagePointers> {
public:
  PyObject *get_python_pointer(size_t n) const;
};

#endif  // HAVE_PYTHON && DO_MEMORY_USAGE

#endif  // MEMORYUSAGEPOINTERS_EXT_H
