/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamReader_ext.h
 * @author rdb
 * @date 2016-04-09
 */

#ifndef BAMREADER_EXT_H
#define BAMREADER_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "bamReader.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for BamReader, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<BamReader> : public ExtensionBase<BamReader> {
public:
  PyObject *get_file_version() const;

  static void register_factory(TypeHandle handle, PyObject *func);
};

#endif  // HAVE_PYTHON

#endif  // BAMREADER_EXT_H
