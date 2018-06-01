/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dtool_super_base.cxx
 * @author drose
 * @date 2005-07-04
 */

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
  { nullptr, nullptr, 0, nullptr }
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

  if (module != nullptr) {
    Py_INCREF((PyTypeObject *)&Dtool_DTOOL_SUPER_BASE);
    PyModule_AddObject(module, "DTOOL_SUPER_BASE", (PyObject *)&Dtool_DTOOL_SUPER_BASE);
  }
}

inline void *Dtool_DowncastInterface_DTOOL_SUPER_BASE(void *from_this, Dtool_PyTypedObject *from_type) {
  return nullptr;
}

inline void *Dtool_UpcastInterface_DTOOL_SUPER_BASE(PyObject *self, Dtool_PyTypedObject *requested_type) {
  return nullptr;
}

int Dtool_Init_DTOOL_SUPER_BASE(PyObject *self, PyObject *args, PyObject *kwds) {
  assert(self != nullptr);
  PyErr_Format(PyExc_TypeError, "cannot init constant class %s", Py_TYPE(self)->tp_name);
  return -1;
}

EXPORT_THIS Dtool_PyTypedObject Dtool_DTOOL_SUPER_BASE = {
  {
    PyVarObject_HEAD_INIT(nullptr, 0)
    "dtoolconfig.DTOOL_SUPER_BASE",
    sizeof(Dtool_PyInstDef),
    0, // tp_itemsize
    &Dtool_FreeInstance_DTOOL_SUPER_BASE,
    nullptr, // tp_print
    nullptr, // tp_getattr
    nullptr, // tp_setattr
#if PY_MAJOR_VERSION >= 3
    nullptr, // tp_compare
#else
    &DTOOL_PyObject_ComparePointers,
#endif
    nullptr, // tp_repr
    nullptr, // tp_as_number
    nullptr, // tp_as_sequence
    nullptr, // tp_as_mapping
    &DTOOL_PyObject_HashPointer,
    nullptr, // tp_call
    nullptr, // tp_str
    PyObject_GenericGetAttr,
    PyObject_GenericSetAttr,
    nullptr, // tp_as_buffer
    (Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_CHECKTYPES),
    nullptr, // tp_doc
    nullptr, // tp_traverse
    nullptr, // tp_clear
#if PY_MAJOR_VERSION >= 3
    &DTOOL_PyObject_RichCompare,
#else
    nullptr, // tp_richcompare
#endif
    0, // tp_weaklistoffset
    nullptr, // tp_iter
    nullptr, // tp_iternext
    Dtool_Methods_DTOOL_SUPER_BASE,
    standard_type_members,
    nullptr, // tp_getset
    nullptr, // tp_base
    nullptr, // tp_dict
    nullptr, // tp_descr_get
    nullptr, // tp_descr_set
    0, // tp_dictoffset
    Dtool_Init_DTOOL_SUPER_BASE,
    PyType_GenericAlloc,
    Dtool_new_DTOOL_SUPER_BASE,
    PyObject_Del,
    nullptr, // tp_is_gc
    nullptr, // tp_bases
    nullptr, // tp_mro
    nullptr, // tp_cache
    nullptr, // tp_subclasses
    nullptr, // tp_weaklist
    nullptr, // tp_del
  },
  TypeHandle::none(),
  Dtool_PyModuleClassInit_DTOOL_SUPER_BASE,
  Dtool_UpcastInterface_DTOOL_SUPER_BASE,
  Dtool_DowncastInterface_DTOOL_SUPER_BASE,
  nullptr,
  nullptr,
};

#endif  // HAVE_PYTHON
