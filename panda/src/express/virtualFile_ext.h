/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file virtualFile_ext.h
 * @author rdb
 * @date 2015-09-15
 */

#ifndef VIRTUALFILE_EXT_H
#define VIRTUALFILE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "virtualFile.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for VirtualFile, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<VirtualFile> : public ExtensionBase<VirtualFile> {
public:
  PyObject *read_file(bool auto_unwrap) const;
  PyObject *write_file(PyObject *data, bool auto_wrap);
};

#endif  // HAVE_PYTHON

#endif  // VIRTUALFILE_EXT_H
