/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file streamReader_ext.cxx
 * @author rdb
 * @date 2013-12-09
 */

#include "streamReader_ext.h"

#ifdef HAVE_PYTHON

#include "vector_string.h"

/**
 * Extracts the indicated number of bytes in the stream and returns them as a
 * string (or bytes, in Python 3).  Returns empty string at end-of-file.
 */
PyObject *Extension<StreamReader>::
extract_bytes(size_t size) {
  std::istream *in = _this->get_istream();
  if (in->eof() || in->fail() || size == 0) {
    // Note that this is only safe to call with size 0 while the GIL is held.
    return PyBytes_FromStringAndSize(nullptr, 0);
  }

  PyObject *bytes = PyBytes_FromStringAndSize(nullptr, size);
  char *buffer = (char *)PyBytes_AS_STRING(bytes);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyThreadState *_save;
  Py_UNBLOCK_THREADS
#endif  // HAVE_THREADS && !SIMPLE_THREADS

  in->read(buffer, size);
  size_t read_bytes = in->gcount();

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  Py_BLOCK_THREADS
#endif  // HAVE_THREADS && !SIMPLE_THREADS

  if (read_bytes == size || _PyBytes_Resize(&bytes, read_bytes) == 0) {
    return bytes;
  } else {
    return nullptr;
  }
}

/**
 * Assumes the stream represents a text file, and extracts one line up to and
 * including the trailing newline character.  Returns empty string when the
 * end of file is reached.
 *
 * The interface here is intentionally designed to be similar to that for
 * Python's File.readline() function.
 */
PyObject *Extension<StreamReader>::
readline() {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyThreadState *_save;
  Py_UNBLOCK_THREADS
#endif  // HAVE_THREADS && !SIMPLE_THREADS

  std::istream *in = _this->get_istream();

  std::string line;
  int ch = in->get();
  while (ch != EOF && !in->fail()) {
    line += ch;
    if (ch == '\n' || in->eof()) {
      // Here's the newline character.
      break;
    }
    ch = in->get();
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  Py_BLOCK_THREADS
#endif  // HAVE_THREADS && !SIMPLE_THREADS

  return PyBytes_FromStringAndSize(line.data(), line.size());
}

/**
 * Reads all the lines at once and returns a list.  Also see the documentation
 * for readline().
 */
PyObject *Extension<StreamReader>::
readlines() {
#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyThreadState *_save;
  Py_UNBLOCK_THREADS
#endif  // HAVE_THREADS && !SIMPLE_THREADS

  std::istream *in = _this->get_istream();
  vector_string lines;

  while (true) {
    std::string line;
    int ch = in->get();
    while (ch != EOF && !in->fail()) {
      line += ch;
      if (ch == '\n' || in->eof()) {
        // Here's the newline character.
        break;
      }
      ch = in->get();
    }

    if (line.empty()) {
      break;
    }

    lines.push_back(std::move(line));
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  Py_BLOCK_THREADS
#endif  // HAVE_THREADS && !SIMPLE_THREADS

  PyObject *lst = PyList_New(lines.size());
  if (lst == nullptr) {
    return nullptr;
  }

  Py_ssize_t i = 0;
  for (const std::string &line : lines) {
    PyObject *py_line = PyBytes_FromStringAndSize(line.data(), line.size());
    PyList_SET_ITEM(lst, i++, py_line);
  }

  return lst;
}

#endif
