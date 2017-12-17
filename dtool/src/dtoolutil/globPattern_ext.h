/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file globPattern_ext.h
 * @author rdb
 * @date 2014-09-17
 */

#ifndef GLOBPATTERN_EXT_H
#define GLOBPATTERN_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "globPattern.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for GlobPattern, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<GlobPattern> : public ExtensionBase<GlobPattern> {
public:
  PyObject *match_files(const Filename &cwd = Filename()) const;
};

#endif  // HAVE_PYTHON

#endif  // GLOBPATTERN_EXT_H
