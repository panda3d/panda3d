// Filename: dtool_super_base.cxx
// Created by:  drose (04Jul05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "py_panda.h"

#ifdef HAVE_PYTHON

class EmptyClass {
};
Define_Module_Class_Private(dtoolconfig, DTOOL_SUPER_BASE, EmptyClass, DTOOL_SUPER_BASE111);

static PyObject *GetSuperBase(PyObject *self) {
  Py_INCREF((PyTypeObject *)&Dtool_DTOOL_SUPER_BASE); // order is important .. this is used for static functions
  return (PyObject *) &Dtool_DTOOL_SUPER_BASE;
};

PyMethodDef Dtool_Methods_DTOOL_SUPER_BASE[] = {
  { "DtoolGetSuperBase", (PyCFunction) &GetSuperBase, METH_NOARGS, "Will Return SUPERbase Class"},
  { NULL, NULL }
};

EXPCL_INTERROGATEDB void Dtool_PyModuleClassInit_DTOOL_SUPER_BASE(PyObject *module) {
  static bool initdone = false;
  if (!initdone) {

    initdone = true;
    Dtool_DTOOL_SUPER_BASE._PyType.tp_dict = PyDict_New();
    PyDict_SetItemString(Dtool_DTOOL_SUPER_BASE._PyType.tp_dict, "DtoolClassDict", Dtool_DTOOL_SUPER_BASE._PyType.tp_dict);

    if (PyType_Ready((PyTypeObject *)&Dtool_DTOOL_SUPER_BASE) < 0) {
      PyErr_SetString(PyExc_TypeError, "PyType_Ready(Dtool_DTOOL_SUPER_BASE)");
      return;
    }
    Py_INCREF((PyTypeObject *)&Dtool_DTOOL_SUPER_BASE);

    PyDict_SetItemString(Dtool_DTOOL_SUPER_BASE._PyType.tp_dict, "DtoolGetSuperBase", PyCFunction_New(&Dtool_Methods_DTOOL_SUPER_BASE[0], (PyObject *)&Dtool_DTOOL_SUPER_BASE));
  }

  if (module != NULL) {
    Py_INCREF((PyTypeObject *)&Dtool_DTOOL_SUPER_BASE);
    PyModule_AddObject(module, "DTOOL_SUPER_BASE", (PyObject *)&Dtool_DTOOL_SUPER_BASE);
  }
}

inline void *Dtool_DowncastInterface_DTOOL_SUPER_BASE(void *from_this, Dtool_PyTypedObject *from_type) {
  return (void *) NULL;
}

inline void *Dtool_UpcastInterface_DTOOL_SUPER_BASE(PyObject *self, Dtool_PyTypedObject *requested_type) {
  return NULL;
}

int Dtool_Init_DTOOL_SUPER_BASE(PyObject *self, PyObject *args, PyObject *kwds) {
  PyErr_SetString(PyExc_TypeError, "cannot init super base");
  return -1;
}

EXPORT_THIS Dtool_PyTypedObject Dtool_DTOOL_SUPER_BASE = {
  {
    PyVarObject_HEAD_INIT(NULL, 0)
    "dtoolconfig.DTOOL_SUPER_BASE",
    sizeof(Dtool_PyInstDef),
    0,
    &Dtool_FreeInstance_DTOOL_SUPER_BASE,
    0,
    0,
    0,
#if PY_MAJOR_VERSION >= 3
    0,
#else
    &DTOOL_PyObject_ComparePointers,
#endif
    0,
    0,
    0,
    0,
    &DTOOL_PyObject_HashPointer,
    0,
    0,
    PyObject_GenericGetAttr,
    PyObject_GenericSetAttr,
    0,
    (Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_CHECKTYPES),
    0,
    0,
    0,
#if PY_MAJOR_VERSION >= 3
    &DTOOL_PyObject_RichCompare,
#else
    0,
#endif
    0,
    0,
    0,
    Dtool_Methods_DTOOL_SUPER_BASE,
    standard_type_members,
    0,
    0,
    0,
    0,
    0,
    0,
    Dtool_Init_DTOOL_SUPER_BASE,
    PyType_GenericAlloc,
    Dtool_new_DTOOL_SUPER_BASE,
    PyObject_Del,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
  },
  TypeHandle::none(),
  Dtool_PyModuleClassInit_DTOOL_SUPER_BASE,
  Dtool_UpcastInterface_DTOOL_SUPER_BASE,
  Dtool_DowncastInterface_DTOOL_SUPER_BASE,
  NULL,
  NULL,
};

#endif  // HAVE_PYTHON
