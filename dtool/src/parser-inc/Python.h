/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file Python.h
 * @author drose
 * @date 2000-05-12
 */

// This file, and all the other files in this directory, aren't
// intended to be compiled--they're just parsed by CPPParser (and
// interrogate) in lieu of the actual system headers, to generate the
// interrogate database.

#ifndef PYTHON_H
#define PYTHON_H

struct _object;
typedef _object PyObject;

struct _typeobject;
typedef _typeobject PyTypeObject;

struct _frame;
typedef _frame PyFrameObject;

typedef struct {} PyStringObject;
typedef struct {} PyUnicodeObject;

typedef struct _ts PyThreadState;
typedef int Py_ssize_t;
typedef struct bufferinfo Py_buffer;

// We need to define these accurately since interrogate may want to
// write these out to default value assignments.
PyObject _Py_NoneStruct;
PyObject _Py_TrueStruct;
#define Py_None (&_Py_NoneStruct)
#define Py_True ((PyObject *) &_Py_TrueStruct)

#if PY_MAJOR_VERSION >= 3
PyObject _Py_ZeroStruct;
#define Py_False ((PyObject *) &_Py_ZeroStruct)
#else
PyObject _Py_FalseStruct;
#define Py_False ((PyObject *) &_Py_FalseStruct)
#endif

typedef void *visitproc;

#endif  // PYTHON_H
