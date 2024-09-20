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

static PyMemberDef standard_type_members[] = {
  {(char *)"this", (sizeof(void*) == sizeof(int)) ? T_UINT : T_ULONGLONG, offsetof(Dtool_PyInstDef, _ptr_to_object), READONLY, (char *)"C++ 'this' pointer, if any"},
  {(char *)"this_ownership", T_BOOL, offsetof(Dtool_PyInstDef, _memory_rules), READONLY, (char *)"C++ 'this' ownership rules"},
  {(char *)"this_const", T_BOOL, offsetof(Dtool_PyInstDef, _is_const), READONLY, (char *)"C++ 'this' const flag"},
// {(char *)"this_signature", T_INT, offsetof(Dtool_PyInstDef, _signature),
// READONLY, (char *)"A type check signature"},
  {(char *)"this_metatype", T_OBJECT_EX, offsetof(Dtool_PyInstDef, _My_Type), READONLY, (char *)"The dtool meta object"},
  {nullptr}  /* Sentinel */
};

static PyObject *GetSuperBase(PyObject *self) {
  Dtool_PyTypedObject *super_base = Dtool_GetSuperBase();
  return Py_XNewRef((PyObject *)&super_base->_PyType);
};

static void Dtool_PyModuleClassInit_DTOOL_SUPER_BASE(PyObject *module) {
  if (module != nullptr) {
    Dtool_PyTypedObject *super_base = Dtool_GetSuperBase();
    PyModule_AddObjectRef(module, "DTOOL_SUPER_BASE", (PyObject *)&super_base->_PyType);
  }
}

static void *Dtool_UpcastInterface_DTOOL_SUPER_BASE(PyObject *self, Dtool_PyTypedObject *requested_type) {
  return nullptr;
}

static PyObject *Dtool_Wrap_DTOOL_SUPER_BASE(void *from_this, PyTypeObject *from_type) {
  return nullptr;
}

static int Dtool_Init_DTOOL_SUPER_BASE(PyObject *self, PyObject *args, PyObject *kwds) {
  assert(self != nullptr);
  PyErr_Format(PyExc_TypeError, "cannot init constant class %s", Py_TYPE(self)->tp_name);
  return -1;
}

static void Dtool_FreeInstance_DTOOL_SUPER_BASE(PyObject *self) {
  Py_TYPE(self)->tp_free(self);
}

/**
 * Returns a pointer to the DTOOL_SUPER_BASE class that is the base class of
 * all Panda types.  This pointer is shared by all modules.
 */
Dtool_PyTypedObject *Dtool_GetSuperBase() {
  Dtool_TypeMap *type_map = Dtool_GetGlobalTypeMap();

  // If we don't have the GIL, we have to protect this with a lock to make
  // sure that there is only one DTOOL_SUPER_BASE instance in the world.
#ifdef Py_GIL_DISABLED
  PyMutex_Lock(&type_map->_lock);
#endif

  auto it = type_map->find("DTOOL_SUPER_BASE");
  if (it != type_map->end()) {
#ifdef Py_GIL_DISABLED
    PyMutex_Unlock(&type_map->_lock);
#endif
    return it->second;
  }

  static PyMethodDef methods[] = {
    { "DtoolGetSuperBase", (PyCFunction)&GetSuperBase, METH_NOARGS, "Will Return SUPERbase Class"},
    { nullptr, nullptr, 0, nullptr }
  };

  static Dtool_PyTypedObject super_base_type = {
    {
      PyVarObject_HEAD_INIT(nullptr, 0)
      "dtoolconfig.DTOOL_SUPER_BASE",
      sizeof(Dtool_PyInstDef),
      0, // tp_itemsize
      &Dtool_FreeInstance_DTOOL_SUPER_BASE,
      0, // tp_vectorcall_offset
      nullptr, // tp_getattr
      nullptr, // tp_setattr
#if PY_MAJOR_VERSION >= 3
      nullptr, // tp_compare
#else
      &DtoolInstance_ComparePointers,
#endif
      nullptr, // tp_repr
      nullptr, // tp_as_number
      nullptr, // tp_as_sequence
      nullptr, // tp_as_mapping
      &DtoolInstance_HashPointer,
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
      &DtoolInstance_RichComparePointers,
#else
      nullptr, // tp_richcompare
#endif
      0, // tp_weaklistoffset
      nullptr, // tp_iter
      nullptr, // tp_iternext
      methods,
      standard_type_members,
      nullptr, // tp_getset
      nullptr, // tp_base
      nullptr, // tp_dict
      nullptr, // tp_descr_get
      nullptr, // tp_descr_set
      0, // tp_dictoffset
      Dtool_Init_DTOOL_SUPER_BASE,
      PyType_GenericAlloc,
      nullptr, // tp_new
      PyObject_Del,
      nullptr, // tp_is_gc
      nullptr, // tp_bases
      nullptr, // tp_mro
      nullptr, // tp_cache
      nullptr, // tp_subclasses
      nullptr, // tp_weaklist
      nullptr, // tp_del
      0, // tp_version_tag,
#if PY_VERSION_HEX >= 0x03040000
      nullptr, // tp_finalize
#endif
#if PY_VERSION_HEX >= 0x03080000
      nullptr, // tp_vectorcall
#endif
    },
    TypeHandle::none(),
    Dtool_PyModuleClassInit_DTOOL_SUPER_BASE,
    Dtool_UpcastInterface_DTOOL_SUPER_BASE,
    Dtool_Wrap_DTOOL_SUPER_BASE,
    nullptr,
    nullptr,
  };

  super_base_type._PyType.tp_dict = PyDict_New();
  PyDict_SetItemString(super_base_type._PyType.tp_dict, "DtoolClassDict", super_base_type._PyType.tp_dict);

  if (PyType_Ready((PyTypeObject *)&super_base_type) < 0) {
    PyErr_SetString(PyExc_TypeError, "PyType_Ready(Dtool_DTOOL_SUPER_BASE)");
#ifdef Py_GIL_DISABLED
    PyMutex_Unlock(&type_map->_lock);
#endif
    return nullptr;
  }
  Py_INCREF(&super_base_type._PyType);

  PyDict_SetItemString(super_base_type._PyType.tp_dict, "DtoolGetSuperBase", PyCFunction_New(&methods[0], (PyObject *)&super_base_type));

  (*type_map)["DTOOL_SUPER_BASE"] = &super_base_type;
#ifdef Py_GIL_DISABLED
  PyMutex_Unlock(&type_map->_lock);
#endif
  return &super_base_type;
}

#endif  // HAVE_PYTHON
