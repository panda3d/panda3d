/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamWriter_ext.h
 * @author rdb
 * @date 2023-05-01
 */

#ifndef BAMWRITER_EXT_H
#define BAMWRITER_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "bamWriter.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for BamWriter, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<BamWriter> : public ExtensionBase<BamWriter> {
public:
  PyObject *get_file_version() const;
};

#endif  // HAVE_PYTHON

#endif  // BAMWRITER_EXT_H
