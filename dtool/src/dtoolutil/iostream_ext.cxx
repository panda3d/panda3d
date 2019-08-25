/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file iostream_ext.cxx
 * @author rdb
 * @date 2017-07-24
 */

#include "iostream_ext.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern struct Dtool_PyTypedObject Dtool_std_istream;
#endif

/**
 * Reads the given number of bytes from the stream, returned as bytes object.
 * If the given size is -1, all bytes are read from the stream.
 */
PyObject *Extension<istream>::
read(Py_ssize_t size) {
  if (size < 0) {
    return readall();
  }

  char *buffer = nullptr;
  std::streamsize read_bytes = 0;

  if (size > 0) {
    std::streambuf *buf = _this->rdbuf();
    nassertr(buf != nullptr, nullptr);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    PyThreadState *_save;
    Py_UNBLOCK_THREADS
#endif

    buffer = (char *)alloca((size_t)size);
    read_bytes = buf->sgetn(buffer, (size_t)size);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
    Py_BLOCK_THREADS
#endif
  }

#if PY_MAJOR_VERSION >= 3
  return PyBytes_FromStringAndSize(buffer, read_bytes);
#else
  return PyString_FromStringAndSize(buffer, read_bytes);
#endif
}

/**
 * Reads from the underlying stream, but using at most one call.  The number
 * of returned bytes may therefore be less than what was requested, but it
 * will always be greater than 0 until EOF is reached.
 */
PyObject *Extension<istream>::
read1(Py_ssize_t size) {
  std::streambuf *buf = _this->rdbuf();
  nassertr(buf != nullptr, nullptr);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyThreadState *_save;
  Py_UNBLOCK_THREADS
#endif

  std::streamsize avail = buf->in_avail();
  if (avail == 0) {
    avail = 4096;
  }

  if (size >= 0 && (std::streamsize)size < avail) {
    avail = (std::streamsize)size;
  }

  // Don't read more than 4K at a time
  if (avail > 4096) {
    avail = 4096;
  }

  char *buffer = (char *)alloca(avail);
  std::streamsize read_bytes = buf->sgetn(buffer, avail);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  Py_BLOCK_THREADS
#endif

#if PY_MAJOR_VERSION >= 3
  return PyBytes_FromStringAndSize(buffer, read_bytes);
#else
  return PyString_FromStringAndSize(buffer, read_bytes);
#endif
}

/**
 * Reads all of the bytes in the stream.
 */
PyObject *Extension<istream>::
readall() {
  std::streambuf *buf = _this->rdbuf();
  nassertr(buf != nullptr, nullptr);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyThreadState *_save;
  Py_UNBLOCK_THREADS
#endif

  std::vector<unsigned char> result;

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  std::streamsize count = buf->sgetn(buffer, buffer_size);
  while (count != 0) {
    thread_consider_yield();
    result.insert(result.end(), buffer, buffer + count);
    count = buf->sgetn(buffer, buffer_size);
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  Py_BLOCK_THREADS
#endif

#if PY_MAJOR_VERSION >= 3
  return PyBytes_FromStringAndSize((char *)result.data(), result.size());
#else
  return PyString_FromStringAndSize((char *)result.data(), result.size());
#endif
}

/**
 * Reads bytes into a preallocated, writable, bytes-like object, returning the
 * number of bytes read.
 */
std::streamsize Extension<istream>::
readinto(PyObject *b) {
  std::streambuf *buf = _this->rdbuf();
  nassertr(buf != nullptr, 0);

  Py_buffer view;
  if (PyObject_GetBuffer(b, &view, PyBUF_CONTIG) == -1) {
    PyErr_SetString(PyExc_TypeError,
      "write() requires a contiguous, read-write bytes-like object");
    return 0;
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyThreadState *_save;
  Py_UNBLOCK_THREADS
#endif

  std::streamsize count = buf->sgetn((char *)view.buf, view.len);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  Py_BLOCK_THREADS
#endif

  PyBuffer_Release(&view);
  return count;
}

/**
 * Extracts one line up to and including the trailing newline character.
 * Returns empty string when the end of file is reached.
 */
PyObject *Extension<istream>::
readline(Py_ssize_t size) {
  std::streambuf *buf = _this->rdbuf();
  nassertr(buf != nullptr, nullptr);

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyThreadState *_save;
  Py_UNBLOCK_THREADS
#endif

  std::string line;
  int ch = buf->sbumpc();
  while (ch != EOF && (--size) != 0) {
    line.push_back(ch);
    if (ch == '\n') {
      // Here's the newline character.
      break;
    }
    ch = buf->sbumpc();
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  Py_BLOCK_THREADS
#endif

#if PY_MAJOR_VERSION >= 3
  return PyBytes_FromStringAndSize(line.data(), line.size());
#else
  return PyString_FromStringAndSize(line.data(), line.size());
#endif
}

/**
 * Reads all the lines at once and returns a list.  Also see the documentation
 * for readline().
 */
PyObject *Extension<istream>::
readlines(Py_ssize_t hint) {
  PyObject *lst = PyList_New(0);
  if (lst == nullptr) {
    return nullptr;
  }

  PyObject *py_line = readline(-1);

  if (hint < 0) {
    while (Py_SIZE(py_line) > 0) {
      PyList_Append(lst, py_line);
      Py_DECREF(py_line);

      py_line = readline(-1);
    }
  } else {
    Py_ssize_t totchars = 0;
    while (Py_SIZE(py_line) > 0) {
      totchars += Py_SIZE(py_line);
      PyList_Append(lst, py_line);
      Py_DECREF(py_line);

      if (totchars > hint) {
        break;
      }

      py_line = readline(-1);
    }
  }

  return lst;
}

/**
 * Yields continuously to read all the lines from the istream.
 */
static PyObject *gen_next(PyObject *self) {
  istream *stream = nullptr;
  if (!Dtool_Call_ExtractThisPointer(self, Dtool_std_istream, (void **)&stream)) {
    return nullptr;
  }

  PyObject *line = invoke_extension(stream).readline();
  if (Py_SIZE(line) > 0) {
    return line;
  } else {
    PyErr_SetObject(PyExc_StopIteration, nullptr);
    return nullptr;
  }
}

/**
 * Iterates over the lines of the file.
 */
PyObject *Extension<istream>::
__iter__(PyObject *self) {
  return Dtool_NewGenerator(self, &gen_next);
}

/**
 * Writes the bytes object to the stream.
 */
void Extension<ostream>::
write(PyObject *b) {
  std::streambuf *buf = _this->rdbuf();
  nassertv(buf != nullptr);

  Py_buffer view;
  if (PyObject_GetBuffer(b, &view, PyBUF_CONTIG_RO) == -1) {
    PyErr_SetString(PyExc_TypeError, "write() requires a contiguous buffer");
    return;
  }

#if defined(HAVE_THREADS) && !defined(SIMPLE_THREADS)
  PyThreadState *_save;
  Py_UNBLOCK_THREADS
  buf->sputn((const char *)view.buf, view.len);
  Py_BLOCK_THREADS
#else
  buf->sputn((const char *)view.buf, view.len);
#endif

  PyBuffer_Release(&view);
}

/**
 * Write a list of lines to the stream.  Line separators are not added, so it
 * is usual for each of the lines provided to have a line separator at the
 * end.
 */
void Extension<ostream>::
writelines(PyObject *lines) {
  PyObject *seq = PySequence_Fast(lines, "writelines() expects a sequence");
  if (seq == nullptr) {
    return;
  }

  PyObject **items = PySequence_Fast_ITEMS(seq);
  Py_ssize_t len = PySequence_Fast_GET_SIZE(seq);

  for (Py_ssize_t i = 0; i < len; ++i) {
    write(items[i]);
  }

  Py_DECREF(seq);
}

#endif  // HAVE_PYTHON
