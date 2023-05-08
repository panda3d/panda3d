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
#include "bamWriter_ext.h"

#ifdef HAVE_PYTHON

/**
 * Returns the version number of the Bam file currently being written.
 */
PyObject *Extension<BamFile>::
get_file_version() const {
  return Py_BuildValue("(ii)", _this->get_file_major_ver(),
                               _this->get_file_minor_ver());
}

#endif
