/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file multifile_ext.h
 * @author Derzsi Daniel
 * @date 2022-07-20
 */

#ifndef MULTIFILE_EXT_H
#define MULTIFILE_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "multifile.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for Multifile, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<Multifile> : public ExtensionBase<Multifile> {
public:
  INLINE PyObject *set_encryption_password(PyObject *encryption_password) const;
  INLINE PyObject *get_encryption_password() const;
};

#include "multifile_ext.I"

#endif  // HAVE_PYTHON

#endif  // MULTIFILE_EXT_H
