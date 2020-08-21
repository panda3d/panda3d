/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file globPattern_ext.cxx
 * @author rdb
 * @date 2014-09-17
 */

#include "globPattern_ext.h"

#ifdef HAVE_PYTHON

/**
 * This variant on match_files returns a Python list of strings.
 */
PyObject *Extension<GlobPattern>::
match_files(const Filename &cwd) const {
  vector_string contents;
  _this->match_files(contents, cwd);

  PyObject *result = PyList_New(contents.size());
  for (size_t i = 0; i < contents.size(); ++i) {
    PyList_SET_ITEM(result, i, Dtool_WrapValue(contents[i]));
  }

  return result;
}

#endif  // HAVE_PYTHON
