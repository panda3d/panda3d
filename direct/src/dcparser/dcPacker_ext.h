/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcPacker_ext.h
 * @author CFSworks
 * @date 2019-07-03
 */

#ifndef DCPACKER_EXT_H
#define DCPACKER_EXT_H

#include "dtoolbase.h"

#ifdef HAVE_PYTHON

#include "extension.h"
#include "dcPacker.h"
#include "py_panda.h"

/**
 * This class defines the extension methods for DCPacker, which are called
 * instead of any C++ methods with the same prototype.
 */
template<>
class Extension<DCPacker> : public ExtensionBase<DCPacker> {
public:
  void pack_object(PyObject *object);
  PyObject *unpack_object();

  void pack_class_object(const DCClass *dclass, PyObject *object);
  PyObject *unpack_class_object(const DCClass *dclass);
  void set_class_element(PyObject *class_def, PyObject *&object,
                         const DCField *field);
  void get_class_element(const DCClass *dclass, PyObject *object,
                         const DCField *field);
};

#endif  // HAVE_PYTHON

#endif  // DCPACKER_EXT_H
