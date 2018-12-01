/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagram_ext.h
 * @author rdb
 * @date 2018-08-19
 */

#ifndef DATAGRAM_EXT_H
#define DATAGRAM_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "datagram.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for Datagram, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<Datagram> : public ExtensionBase<Datagram> {
public:
  INLINE PyObject *get_message() const;
  INLINE PyObject *__bytes__() const;
};

#include "datagram_ext.I"

#endif  // HAVE_PYTHON

#endif  // DATAGRAM_EXT_H
