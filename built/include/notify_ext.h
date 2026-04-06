/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file notify_ext.h
 * @author rdb
 * @date 2022-12-09
 */

#ifndef NOTIFY_EXT_H
#define NOTIFY_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "pnotify.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for Notify, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<Notify> : public ExtensionBase<Notify> {
public:
  void set_ostream_ptr(PyObject *ostream_ptr, bool delete_later);
};

#endif  // HAVE_PYTHON

#endif  // NOTIFY_EXT_H
