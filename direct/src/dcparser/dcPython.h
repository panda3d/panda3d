/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcPython.h
 * @author drose
 * @date 2004-06-22
 */

#ifndef DCPYTHON_H
#define DCPYTHON_H

// The only purpose of this file is to serve as a common place to put the
// nonsense associated with #including <Python.h>.

#ifdef HAVE_PYTHON

#define PY_SSIZE_T_CLEAN 1

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE
#include <Python.h>

// Python 2.5 adds Py_ssize_t; earlier versions don't have it.
#if PY_VERSION_HEX < 0x02050000 && !defined(PY_SSIZE_T_MIN)
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#endif

// Several interfaces in this module that use Python also require these header
// files, so we might as well pick them up too.
#include "datagram.h"
#include "datagramIterator.h"

#endif  // HAVE_PYTHON

#endif
