// Filename: py_panda.cxx
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
#include "config_interrogatedb.h"

#ifdef HAVE_PYTHON

PyMemberDef standard_type_members[] = {
  {(char *)"this", (sizeof(void*) == sizeof(int)) ? T_UINT : T_ULONGLONG, offsetof(Dtool_PyInstDef, _ptr_to_object), READONLY, (char *)"C++ 'this' pointer, if any"},
  {(char *)"this_ownership", T_BOOL, offsetof(Dtool_PyInstDef, _memory_rules), READONLY, (char *)"C++ 'this' ownership rules"},
  {(char *)"this_const", T_BOOL, offsetof(Dtool_PyInstDef, _is_const), READONLY, (char *)"C++ 'this' const flag"},
//  {(char *)"this_signature", T_INT, offsetof(Dtool_PyInstDef, _signature), READONLY, (char *)"A type check signature"},
  {(char *)"this_metatype", T_OBJECT, offsetof(Dtool_PyInstDef, _My_Type), READONLY, (char *)"The dtool meta object"},
  {NULL}  /* Sentinel */
};

static RuntimeTypeMap runtime_type_map;
static RuntimeTypeSet runtime_type_set;
static NamedTypeMap named_type_map;

////////////////////////////////////////////////////////////////////
//     Function: DtoolCanThisBeAPandaInstance
//  Description: Given a valid (non-NULL) PyObject, does a simple
//               check to see if it might be an instance of a Panda
//               type.  It does this using a signature that is
//               encoded on each instance.
////////////////////////////////////////////////////////////////////
bool DtoolCanThisBeAPandaInstance(PyObject *self) {
  // simple sanity check for the class type..size.. will stop basic foobars..
  // It is arguably better to use something like this:
  // PyType_IsSubtype(Py_TYPE(self), &Dtool_DTOOL_SUPER_BASE._PyType)
  // ...but probably not as fast.
  if (Py_TYPE(self)->tp_basicsize >= (int)sizeof(Dtool_PyInstDef)) {
    Dtool_PyInstDef *pyself = (Dtool_PyInstDef *) self;
    if (pyself->_signature == PY_PANDA_SIGNATURE) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////
//  Function : DTOOL_Call_ExtractThisPointerForType
//
//  These are the wrappers that allow for down and upcast from type ..
//      needed by the Dtool py interface.. Be very careful if you muck with these
//      as the generated code depends on how this is set up..
////////////////////////////////////////////////////////////////////////
void DTOOL_Call_ExtractThisPointerForType(PyObject *self, Dtool_PyTypedObject *classdef, void **answer) {
  if (DtoolCanThisBeAPandaInstance(self)) {
    *answer = ((Dtool_PyInstDef *)self)->_My_Type->_Dtool_UpcastInterface(self, classdef);
  } else {
    *answer = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Dtool_Call_ExtractThisPointer
//  Description: This is a support function for the Python bindings:
//               it extracts the underlying C++ pointer of the given
//               type for a given Python object.  If it was of the
//               wrong type, raises an AttributeError.
////////////////////////////////////////////////////////////////////
bool Dtool_Call_ExtractThisPointer(PyObject *self, Dtool_PyTypedObject &classdef, void **answer) {
  if (self == NULL || !DtoolCanThisBeAPandaInstance(self)) {
    Dtool_Raise_TypeError("C++ object is not yet constructed, or already destructed.");
    return false;
  }

  *answer = ((Dtool_PyInstDef *)self)->_My_Type->_Dtool_UpcastInterface(self, &classdef);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Dtool_Call_ExtractThisPointer_NonConst
//  Description: The same thing as Dtool_Call_ExtractThisPointer,
//               except that it performs the additional check that
//               the pointer is a non-const pointer.  This is called
//               by function wrappers for functions of which all
//               overloads are non-const, and saves a bit of code.
//
//               The extra method_name argument is used in formatting
//               the error message.
////////////////////////////////////////////////////////////////////
bool Dtool_Call_ExtractThisPointer_NonConst(PyObject *self, Dtool_PyTypedObject &classdef,
                                            void **answer, const char *method_name) {

  if (self == NULL || !DtoolCanThisBeAPandaInstance(self)) {
    Dtool_Raise_TypeError("C++ object is not yet constructed, or already destructed.");
    return false;
  }

  if (((Dtool_PyInstDef *)self)->_is_const) {
    // All overloads of this function are non-const.
    PyErr_Format(PyExc_TypeError,
                 "Cannot call %s() on a const object.",
                 method_name);
    return false;
  }

  *answer = ((Dtool_PyInstDef *)self)->_My_Type->_Dtool_UpcastInterface(self, &classdef);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DTOOL_Call_GetPointerThisClass
//  Description: Extracts the C++ pointer for an object, given its
//               Python wrapper object, for passing as the parameter
//               to a C++ function.
//
//               self is the Python wrapper object in question.
//
//               classdef is the Python class wrapper for the C++
//               class in which the this pointer should be returned.
//               (This may require an upcast operation, if self is not
//               already an instance of classdef.)
//
//               param and function_name are used for error reporting
//               only, and describe the particular function and
//               parameter index for this parameter.
//
//               const_ok is true if the function is declared const
//               and can therefore be called with either a const or
//               non-const "this" pointer, or false if the function is
//               declared non-const, and can therefore be called with
//               only a non-const "this" pointer.
//
//               The return value is the C++ pointer that was
//               extracted, or NULL if there was a problem (in which
//               case the Python exception state will have been set).
////////////////////////////////////////////////////////////////////
void *
DTOOL_Call_GetPointerThisClass(PyObject *self, Dtool_PyTypedObject *classdef,
                               int param, const string &function_name, bool const_ok,
                               bool report_errors) {
  //if (PyErr_Occurred()) {
  //  return NULL;
  //}
  if (self == NULL) {
    if (report_errors) {
      return Dtool_Raise_TypeError("self is NULL");
    }
    return NULL;
  }

  if (DtoolCanThisBeAPandaInstance(self)) {
    void *result = ((Dtool_PyInstDef *)self)->_My_Type->_Dtool_UpcastInterface(self, classdef);

    if (result != NULL) {
      if (const_ok || !((Dtool_PyInstDef *)self)->_is_const) {
        return result;
      }

      if (report_errors) {
        return PyErr_Format(PyExc_TypeError,
                            "%s() argument %d may not be const",
                            function_name.c_str(), param);
      }
      return NULL;
    }
  }

  if (report_errors) {
    return Dtool_Raise_ArgTypeError(self, param, function_name.c_str(), classdef->_PyType.tp_name);
  }

  return NULL;
}

void *DTOOL_Call_GetPointerThis(PyObject *self) {
  if (self != NULL) {
    if (DtoolCanThisBeAPandaInstance(self)) {
      Dtool_PyInstDef * pyself = (Dtool_PyInstDef *) self;
      return pyself->_ptr_to_object;
    }
  }
  return NULL;
}

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: Dtool_CheckErrorOccurred
//  Description: This is similar to a PyErr_Occurred() check, except
//               that it also checks Notify to see if an assertion
//               has occurred.  If that is the case, then it raises
//               an AssertionError.
//
//               Returns true if there is an active exception, false
//               otherwise.
//
//               In the NDEBUG case, this is simply a #define to
//               _PyErr_OCCURRED() (which is an undocumented inline
//               version of PyErr_Occurred()).
////////////////////////////////////////////////////////////////////
bool Dtool_CheckErrorOccurred() {
  if (_PyErr_OCCURRED()) {
    return true;
  }
  if (Notify::ptr()->has_assert_failed()) {
    Dtool_Raise_AssertionError();
    return true;
  }
  return false;
}
#endif  // NDEBUG

////////////////////////////////////////////////////////////////////
//     Function: Dtool_Raise_AssertionError
//  Description: Raises an AssertionError containing the last thrown
//               assert message, and clears the assertion flag.
//               Returns NULL.
////////////////////////////////////////////////////////////////////
PyObject *Dtool_Raise_AssertionError() {
  Notify *notify = Notify::ptr();
#if PY_MAJOR_VERSION >= 3
  PyObject *message = PyUnicode_FromString(notify->get_assert_error_message().c_str());
#else
  PyObject *message = PyString_FromString(notify->get_assert_error_message().c_str());
#endif
  Py_INCREF(PyExc_AssertionError);
  PyErr_Restore(PyExc_AssertionError, message, (PyObject *)NULL);
  notify->clear_assert_failed();
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Dtool_Raise_TypeError
//  Description: Raises a TypeError with the given message, and
//               returns NULL.
////////////////////////////////////////////////////////////////////
PyObject *Dtool_Raise_TypeError(const char *message) {
  // PyErr_Restore is what PyErr_SetString would have ended up calling
  // eventually anyway, so we might as well just get to the point.
  Py_INCREF(PyExc_TypeError);
#if PY_MAJOR_VERSION >= 3
  PyErr_Restore(PyExc_TypeError, PyUnicode_FromString(message), (PyObject *)NULL);
#else
  PyErr_Restore(PyExc_TypeError, PyString_FromString(message), (PyObject *)NULL);
#endif
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Dtool_Raise_ArgTypeError
//  Description: Raises a TypeError of the form:
//               function_name() argument n must be type, not type
//               for a given object passed to a function.
//
//               Always returns NULL so that it can be conveniently
//               used as a return expression for wrapper functions
//               that return a PyObject pointer.
////////////////////////////////////////////////////////////////////
PyObject *Dtool_Raise_ArgTypeError(PyObject *obj, int param, const char *function_name, const char *type_name) {
#if PY_MAJOR_VERSION >= 3
  PyObject *message = PyUnicode_FromFormat(
#else
  PyObject *message = PyString_FromFormat(
#endif
    "%s() argument %d must be %s, not %s",
    function_name, param, type_name,
    Py_TYPE(obj)->tp_name);

  Py_INCREF(PyExc_TypeError);
  PyErr_Restore(PyExc_TypeError, message, (PyObject *)NULL);
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Dtool_Raise_AttributeError
//  Description: Raises an AttributeError of the form:
//               'type' has no attribute 'attr'
//
//               Always returns NULL so that it can be conveniently
//               used as a return expression for wrapper functions
//               that return a PyObject pointer.
////////////////////////////////////////////////////////////////////
PyObject *Dtool_Raise_AttributeError(PyObject *obj, const char *attribute) {
#if PY_MAJOR_VERSION >= 3
  PyObject *message = PyUnicode_FromFormat(
#else
  PyObject *message = PyString_FromFormat(
#endif
    "'%.100s' object has no attribute '%.200s'",
    Py_TYPE(obj)->tp_name, attribute);

  Py_INCREF(PyExc_TypeError);
  PyErr_Restore(PyExc_TypeError, message, (PyObject *)NULL);
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Dtool_Raise_BadArgumentsError
//  Description: Raises a TypeError of the form:
//               Arguments must match:
//               <list of overloads>
//
//               However, in release builds, this instead is defined
//               to a function that just prints out a generic
//               message, to help reduce the amount of strings in
//               the compiled library.
//
//               Always returns NULL so that it can be conveniently
//               used as a return expression for wrapper functions
//               that return a PyObject pointer.
////////////////////////////////////////////////////////////////////
PyObject *_Dtool_Raise_BadArgumentsError() {
  return Dtool_Raise_TypeError("arguments do not match any function overload");
}

////////////////////////////////////////////////////////////////////
//     Function: Dtool_Return_None
//  Description: Convenience method that checks for exceptions, and
//               if one occurred, returns NULL, otherwise Py_None.
////////////////////////////////////////////////////////////////////
PyObject *_Dtool_Return_None() {
  if (_PyErr_OCCURRED()) {
    return NULL;
  }
#ifndef NDEBUG
  if (Notify::ptr()->has_assert_failed()) {
    return Dtool_Raise_AssertionError();
  }
#endif
  Py_INCREF(Py_None);
  return Py_None;
}

////////////////////////////////////////////////////////////////////
//     Function: Dtool_Return_Bool
//  Description: Convenience method that checks for exceptions, and
//               if one occurred, returns NULL, otherwise the given
//               boolean value as a PyObject *.
////////////////////////////////////////////////////////////////////
PyObject *Dtool_Return_Bool(bool value) {
  if (_PyErr_OCCURRED()) {
    return NULL;
  }
#ifndef NDEBUG
  if (Notify::ptr()->has_assert_failed()) {
    return Dtool_Raise_AssertionError();
  }
#endif
  PyObject *result = (value ? Py_True : Py_False);
  Py_INCREF(result);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Dtool_Return
//  Description: Convenience method that checks for exceptions, and
//               if one occurred, returns NULL, otherwise the given
//               return value.  Its reference count is not increased.
////////////////////////////////////////////////////////////////////
PyObject *_Dtool_Return(PyObject *value) {
  if (_PyErr_OCCURRED()) {
    return NULL;
  }
#ifndef NDEBUG
  if (Notify::ptr()->has_assert_failed()) {
    return Dtool_Raise_AssertionError();
  }
#endif
  return value;
}

////////////////////////////////////////////////////////////////////////
//  Function : DTool_CreatePyInstanceTyped
//
// this function relies on the behavior of typed objects in the panda system.
//
////////////////////////////////////////////////////////////////////////
PyObject *DTool_CreatePyInstanceTyped(void *local_this_in, Dtool_PyTypedObject &known_class_type, bool memory_rules, bool is_const, int type_index) {
  // We can't do the NULL check here like in DTool_CreatePyInstance, since
  // the caller will have to get the type index to pass to this function
  // to begin with.  That code probably would have crashed by now if it was
  // really NULL for whatever reason.
  nassertr(local_this_in != NULL, NULL);

  /////////////////////////////////////////////////////
  // IF the class is possibly a run time typed object
  /////////////////////////////////////////////////////
  if (type_index > 0) {
    /////////////////////////////////////////////////////
    // get best fit class...
    /////////////////////////////////////////////////////
    Dtool_PyTypedObject *target_class = Dtool_RuntimeTypeDtoolType(type_index);
    if (target_class != NULL) {
      /////////////////////////////////////////////////////
      // cast to the type...
      //////////////////////////////////////////////////////
      void *new_local_this = target_class->_Dtool_DowncastInterface(local_this_in, &known_class_type);
      if (new_local_this != NULL) {
        /////////////////////////////////////////////
        // ask class to allocate an instance..
        /////////////////////////////////////////////
        Dtool_PyInstDef *self = (Dtool_PyInstDef *) target_class->_PyType.tp_new(&target_class->_PyType, NULL, NULL);
        if (self != NULL) {
          self->_ptr_to_object = new_local_this;
          self->_memory_rules = memory_rules;
          self->_is_const = is_const;
          //self->_signature = PY_PANDA_SIGNATURE;
          self->_My_Type = target_class;
          return (PyObject *)self;
        }
      }
    }
  }

  /////////////////////////////////////////////////////
  // if we get this far .. just wrap the thing in the known type ??
  //    better than aborting...I guess....
  /////////////////////////////////////////////////////
  Dtool_PyInstDef *self = (Dtool_PyInstDef *) known_class_type._PyType.tp_new(&known_class_type._PyType, NULL, NULL);
  if (self != NULL) {
    self->_ptr_to_object = local_this_in;
    self->_memory_rules = memory_rules;
    self->_is_const = is_const;
    //self->_signature = PY_PANDA_SIGNATURE;
    self->_My_Type = &known_class_type;
  }
  return (PyObject *)self;
}

////////////////////////////////////////////////////////////////////////
// DTool_CreatePyInstance .. wrapper function to finalize the existance of a general
//    dtool py instance..
////////////////////////////////////////////////////////////////////////
PyObject *DTool_CreatePyInstance(void *local_this, Dtool_PyTypedObject &in_classdef, bool memory_rules, bool is_const) {
  if (local_this == NULL) {
    // This is actually a very common case, so let's allow this, but return
    // Py_None consistently.  This eliminates code in the wrappers.
    Py_INCREF(Py_None);
    return Py_None;
  }

  Dtool_PyTypedObject *classdef = &in_classdef;
  Dtool_PyInstDef *self = (Dtool_PyInstDef *) classdef->_PyType.tp_new(&classdef->_PyType, NULL, NULL);
  if (self != NULL) {
    self->_ptr_to_object = local_this;
    self->_memory_rules = memory_rules;
    self->_is_const = is_const;
    self->_My_Type = classdef;
  }
  return (PyObject *)self;
}

///////////////////////////////////////////////////////////////////////////////
/// Th Finalizer for simple instances..
///////////////////////////////////////////////////////////////////////////////
int DTool_PyInit_Finalize(PyObject *self, void *local_this, Dtool_PyTypedObject *type, bool memory_rules, bool is_const) {
  // lets put some code in here that checks to see the memory is properly configured..
  // prior to my call ..

  ((Dtool_PyInstDef *)self)->_My_Type = type;
  ((Dtool_PyInstDef *)self)->_ptr_to_object = local_this;
  ((Dtool_PyInstDef *)self)->_memory_rules = memory_rules;
  ((Dtool_PyInstDef *)self)->_is_const = is_const;
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// A helper function to glue method definition together .. that can not be done
// at code generation time because of multiple generation passes in interrogate..
//
///////////////////////////////////////////////////////////////////////////////
void Dtool_Accum_MethDefs(PyMethodDef in[], MethodDefmap &themap) {
  for (; in->ml_name != NULL; in++) {
    if (themap.find(in->ml_name) == themap.end()) {
      themap[in->ml_name] = in;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
//  ** HACK ** alert..
//
//      Need to keep a runtime type dictionary ... that is forward declared of typed object.
//        We rely on the fact that typed objects are uniquly defined by an integer.
//
///////////////////////////////////////////////////////////////////////////////
void
RegisterNamedClass(const string &name, Dtool_PyTypedObject &otype) {
  pair<NamedTypeMap::iterator, bool> result =
    named_type_map.insert(NamedTypeMap::value_type(name, &otype));

  if (!result.second) {
    // There was already a class with this name in the dictionary.
    interrogatedb_cat.warning()
      << "Double definition for class " << name << "\n";
  }
}

void
RegisterRuntimeTypedClass(Dtool_PyTypedObject &otype) {
  int type_index = otype._type.get_index();

  if (type_index == 0) {
    interrogatedb_cat.warning()
      << "Class " << otype._PyType.tp_name
      << " has a zero TypeHandle value; check that init_type() is called.\n";

  } else if (type_index < 0 || type_index >= TypeRegistry::ptr()->get_num_typehandles()) {
    interrogatedb_cat.warning()
      << "Class " << otype._PyType.tp_name
      << " has an illegal TypeHandle value; check that init_type() is called.\n";

  } else {
    pair<RuntimeTypeMap::iterator, bool> result =
      runtime_type_map.insert(RuntimeTypeMap::value_type(type_index, &otype));
    if (!result.second) {
      // There was already an entry in the dictionary for type_index.
      Dtool_PyTypedObject *other_type = (*result.first).second;
      interrogatedb_cat.warning()
        << "Classes " << otype._PyType.tp_name
        << " and " << other_type->_PyType.tp_name
        << " share the same TypeHandle value (" << type_index
        << "); check class definitions.\n";

    } else {
      runtime_type_set.insert(type_index);
    }
  }
}

Dtool_PyTypedObject *
LookupNamedClass(const string &name) {
  NamedTypeMap::const_iterator it;
  it = named_type_map.find(name);

  if (it == named_type_map.end()) {
    // Find a type named like this in the type registry.
    TypeHandle handle = TypeRegistry::ptr()->find_type(name);
    if (handle.get_index() > 0) {
      RuntimeTypeMap::const_iterator it2;
      it2 = runtime_type_map.find(handle.get_index());
      if (it2 != runtime_type_map.end()) {
        return it2->second;
      }
    }

    interrogatedb_cat.error()
      << "Attempt to use type " << name << " which has not yet been defined!\n";
    return NULL;
  } else {
    return it->second;
  }
}

Dtool_PyTypedObject *
LookupRuntimeTypedClass(TypeHandle handle) {
  RuntimeTypeMap::const_iterator it;
  it = runtime_type_map.find(handle.get_index());

  if (it == runtime_type_map.end()) {
    interrogatedb_cat.error()
      << "Attempt to use type " << handle << " which has not yet been defined!\n";
    return NULL;
  } else {
    return it->second;
  }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
Dtool_PyTypedObject *Dtool_RuntimeTypeDtoolType(int type) {
  RuntimeTypeMap::iterator di = runtime_type_map.find(type);
  if (di != runtime_type_map.end()) {
    return di->second;
  } else {
    int type2 = get_best_parent_from_Set(type, runtime_type_set);
    di = runtime_type_map.find(type2);
    if (di != runtime_type_map.end()) {
      return di->second;
    }
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
#if PY_MAJOR_VERSION >= 3
PyObject *Dtool_PyModuleInitHelper(LibraryDef *defs[], PyModuleDef *module_def) {
#else
PyObject *Dtool_PyModuleInitHelper(LibraryDef *defs[], const char *modulename) {
#endif
  // the module level function inits....
  MethodDefmap functions;
  for (int xx = 0; defs[xx] != NULL; xx++) {
    Dtool_Accum_MethDefs(defs[xx]->_methods, functions);
  }

  PyMethodDef *newdef = new PyMethodDef[functions.size() + 1];
  MethodDefmap::iterator mi;
  int offset = 0;
  for (mi = functions.begin(); mi != functions.end(); mi++, offset++) {
    newdef[offset] = *mi->second;
  }
  newdef[offset].ml_doc = NULL;
  newdef[offset].ml_name = NULL;
  newdef[offset].ml_meth = NULL;
  newdef[offset].ml_flags = 0;

#if PY_MAJOR_VERSION >= 3
  module_def->m_methods = newdef;
  PyObject *module = PyModule_Create(module_def);
#else
  PyObject *module = Py_InitModule((char *)modulename, newdef);
#endif

  if (module == NULL) {
#if PY_MAJOR_VERSION >= 3
    return Dtool_Raise_TypeError("PyModule_Create returned NULL");
#else
    return Dtool_Raise_TypeError("Py_InitModule returned NULL");
#endif
  }

  PyModule_AddIntConstant(module, "Dtool_PyNativeInterface", 1);
  return module;
}

///////////////////////////////////////////////////////////////////////////////
///  HACK.... Be careful
//
//  Dtool_BorrowThisReference
//      This function can be used to grab the "THIS" pointer from an object and use it
//      Required to support historical inheritance in the form of "is this instance of"..
//
///////////////////////////////////////////////////////////////////////////////
PyObject *Dtool_BorrowThisReference(PyObject *self, PyObject *args) {
  PyObject *from_in = NULL;
  PyObject *to_in = NULL;
  if (PyArg_UnpackTuple(args, "Dtool_BorrowThisReference", 2, 2, &to_in, &from_in)) {

    if (DtoolCanThisBeAPandaInstance(from_in) && DtoolCanThisBeAPandaInstance(to_in)) {
      Dtool_PyInstDef *from = (Dtool_PyInstDef *) from_in;
      Dtool_PyInstDef *to = (Dtool_PyInstDef *) to_in;

      //if (PyObject_TypeCheck(to_in, Py_TYPE(from_in))) {
      if (from->_My_Type == to->_My_Type) {
        to->_memory_rules = false;
        to->_is_const = from->_is_const;
        to->_ptr_to_object = from->_ptr_to_object;

        Py_INCREF(Py_None);
        return Py_None;
      }

      return PyErr_Format(PyExc_TypeError, "types %s and %s do not match",
                          Py_TYPE(from)->tp_name, Py_TYPE(to)->tp_name);
    } else {
      return Dtool_Raise_TypeError("One of these does not appear to be DTOOL Instance ??");
    }
  }
  return (PyObject *) NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// We do expose a dictionay for dtool classes .. this should be removed at some point..
//////////////////////////////////////////////////////////////////////////////////////////////
PyObject *Dtool_AddToDictionary(PyObject *self1, PyObject *args) {
  PyObject *self;
  PyObject *subject;
  PyObject *key;
  if (PyArg_ParseTuple(args, "OSO", &self, &key, &subject)) {
    PyObject *dict = ((PyTypeObject *)self)->tp_dict;
    if (dict == NULL || !PyDict_Check(dict)) {
      return Dtool_Raise_TypeError("No dictionary On Object");
    } else {
      PyDict_SetItem(dict, key, subject);
    }
  }
  if (PyErr_Occurred()) {
    return (PyObject *)NULL;
  }
  Py_INCREF(Py_None);
  return Py_None;
}

///////////////////////////////////////////////////////////////////////////////////
Py_hash_t DTOOL_PyObject_HashPointer(PyObject *self) {
  if (self != NULL && DtoolCanThisBeAPandaInstance(self)) {
    Dtool_PyInstDef * pyself = (Dtool_PyInstDef *) self;
    return (Py_hash_t) pyself->_ptr_to_object;
  }
  return -1;
}

/* Compare v to w.  Return
   -1 if v <  w or exception (PyErr_Occurred() true in latter case).
    0 if v == w.
    1 if v > w.
   XXX The docs (C API manual) say the return value is undefined in case
   XXX of error.
*/

int DTOOL_PyObject_ComparePointers(PyObject *v1, PyObject *v2) {
  // try this compare
  void *v1_this = DTOOL_Call_GetPointerThis(v1);
  void *v2_this = DTOOL_Call_GetPointerThis(v2);
  if (v1_this != NULL && v2_this != NULL) { // both are our types...
    if (v1_this < v2_this) {
      return -1;
    }
    if (v1_this > v2_this) {
      return 1;
    }
    return 0;
  }

  // ok self compare...
  if (v1 < v2) {
    return -1;
  }
  if (v1 > v2) {
    return 1;
  }
  return 0;
}

int DTOOL_PyObject_Compare(PyObject *v1, PyObject *v2) {
  // First try compareTo function..
  PyObject * func = PyObject_GetAttrString(v1, "compare_to");
  if (func == NULL) {
    PyErr_Clear();
  } else {
    PyObject *res = NULL;
    PyObject *args = PyTuple_Pack(1, v2);
    if (args != NULL) {
      res = PyObject_Call(func, args, NULL);
      Py_DECREF(args);
    }
    Py_DECREF(func);
    PyErr_Clear(); // just in case the function threw an error
    // only use if the function returns an INT... hmm
    if (res != NULL) {
      if (PyLong_Check(res)) {
        long answer = PyLong_AsLong(res);
        Py_DECREF(res);

        // Python really wants us to return strictly -1, 0, or 1.
        if (answer < 0) {
          return -1;
        } else if (answer > 0) {
          return 1;
        } else {
          return 0;
        }
      }
#if PY_MAJOR_VERSION < 3
      else if (PyInt_Check(res)) {
        long answer = PyInt_AsLong(res);
        Py_DECREF(res);

        // Python really wants us to return strictly -1, 0, or 1.
        if (answer < 0) {
          return -1;
        } else if (answer > 0) {
          return 1;
        } else {
          return 0;
        }
      }
#endif
      Py_DECREF(res);
    }
  }

  return DTOOL_PyObject_ComparePointers(v1, v2);
}

PyObject *DTOOL_PyObject_RichCompare(PyObject *v1, PyObject *v2, int op) {
  int cmpval = DTOOL_PyObject_Compare(v1, v2);
  bool result;
  switch (op) {
  case Py_LT:
    result = (cmpval < 0);
    break;
  case Py_LE:
    result = (cmpval <= 0);
    break;
  case Py_EQ:
    result = (cmpval == 0);
    break;
  case Py_NE:
    result = (cmpval != 0);
    break;
  case Py_GT:
    result = (cmpval > 0);
    break;
  case Py_GE:
    result = (cmpval >= 0);
  }
  return PyBool_FromLong(result);
}

////////////////////////////////////////////////////////////////////
//     Function: copy_from_make_copy
//  Description: This is a support function for a synthesized
//               __copy__() method from a C++ make_copy() method.
////////////////////////////////////////////////////////////////////
PyObject *copy_from_make_copy(PyObject *self, PyObject *noargs) {
  return PyObject_CallMethod(self, (char *)"make_copy", (char *)"()");
}

////////////////////////////////////////////////////////////////////
//     Function: copy_from_copy_constructor
//  Description: This is a support function for a synthesized
//               __copy__() method from a C++ copy constructor.
////////////////////////////////////////////////////////////////////
PyObject *copy_from_copy_constructor(PyObject *self, PyObject *noargs) {
  PyObject *this_class = PyObject_Type(self);
  if (this_class == NULL) {
    return NULL;
  }

  PyObject *result = PyObject_CallFunction(this_class, (char *)"(O)", self);
  Py_DECREF(this_class);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: map_deepcopy_to_copy
//  Description: This is a support function for a synthesized
//               __deepcopy__() method for any class that has a
//               __copy__() method.  The sythethic method simply
//               invokes __copy__().
////////////////////////////////////////////////////////////////////
PyObject *map_deepcopy_to_copy(PyObject *self, PyObject *args) {
  return PyObject_CallMethod(self, (char *)"__copy__", (char *)"()");
}

////////////////////////////////////////////////////////////////////
//     Function: PyLongOrInt_FromUnsignedLong
//  Description: Similar to PyLong_FromUnsignedLong(), but returns
//               either a regular integer or a long integer, according
//               to whether the indicated value will fit.
////////////////////////////////////////////////////////////////////
#if PY_MAJOR_VERSION < 3
EXPCL_INTERROGATEDB PyObject *
PyLongOrInt_FromUnsignedLong(unsigned long value) {
  if ((long)value < 0) {
    return PyLong_FromUnsignedLong(value);
  } else {
    return PyInt_FromLong((long)value);
  }
}
#endif

#endif  // HAVE_PYTHON
