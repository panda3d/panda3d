/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bamFile_ext.cxx
 * @author rdb
 * @date 2023-05-03
 */

#include "bamFile_ext.h"
#include "bamReader_ext.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern Dtool_PyTypedObject Dtool_TypedWritable;
#endif  // CPPPARSER

/**
 * Reads an object from the BamFile.
 */
PyObject *Extension<BamFile>::
read_object() {
  BamReader *reader = _this->get_reader();
  if (reader == nullptr) {
    PyErr_SetString(PyExc_ValueError, "BamFile not open for reading");
    return nullptr;
  }

  return invoke_extension(reader).read_object();
}

/**
 * Returns the version number of the Bam file currently being written.
 */
PyObject *Extension<BamFile>::
get_file_version() const {
  return Py_BuildValue("(ii)", _this->get_file_major_ver(),
                               _this->get_file_minor_ver());
}

#endif
