/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file py_panda.cxx
 * @author drose
 * @date 2005-07-04
 */

#include "py_panda.h"
#include "config_interrogatedb.h"
#include "executionEnvironment.h"

#ifdef HAVE_PYTHON

PyTupleObject Dtool_EmptyTuple;

PyMemberDef standard_type_members[] = {
  {(char *)"this", (sizeof(void*) == sizeof(int)) ? T_UINT : T_ULONGLONG, offsetof(Dtool_PyInstDef, _ptr_to_object), READONLY, (char *)"C++ 'this' pointer, if any"},
  {(char *)"this_ownership", T_BOOL, offsetof(Dtool_PyInstDef, _memory_rules), READONLY, (char *)"C++ 'this' ownership rules"},
  {(char *)"this_const", T_BOOL, offsetof(Dtool_PyInstDef, _is_const), READONLY, (char *)"C++ 'this' const flag"},
// {(char *)"this_signature", T_INT, offsetof(Dtool_PyInstDef, _signature),
// READONLY, (char *)"A type check signature"},
  {(char *)"this_metatype", T_OBJECT, offsetof(Dtool_PyInstDef, _My_Type), READONLY, (char *)"The dtool meta object"},
  {NULL}  /* Sentinel */
};

static RuntimeTypeMap runtime_type_map;
static RuntimeTypeSet runtime_type_set;
static NamedTypeMap named_type_map;

#if PY_MAJOR_VERSION < 3
/**
 * Given a long or int, returns a size_t, or raises an OverflowError if it is
 * out of range.
 */
size_t PyLongOrInt_AsSize_t(PyObject *vv) {
  if (PyInt_Check(vv)) {
    long value = PyInt_AS_LONG(vv);
    if (value < 0) {
      PyErr_SetString(PyExc_OverflowError,
                      "can't convert negative value to size_t");
      return (size_t)-1;
    }
    return (size_t)value;
  }

  if (!PyLong_Check(vv)) {
    Dtool_Raise_TypeError("a long or int was expected");
    return (size_t)-1;
  }

  size_t bytes;
  int one = 1;
  int res = _PyLong_AsByteArray((PyLongObject *)vv, (unsigned char *)&bytes,
                                SIZEOF_SIZE_T, (int)*(unsigned char*)&one, 0);

  if (res < 0) {
    return (size_t)res;
  } else {
    return bytes;
  }
}
#endif

/**
 * Given a valid (non-NULL) PyObject, does a simple check to see if it might
 * be an instance of a Panda type.  It does this using a signature that is
 * encoded on each instance.
 */
bool DtoolCanThisBeAPandaInstance(PyObject *self) {
  // simple sanity check for the class type..size.. will stop basic foobars..
  // It is arguably better to use something like this:
  // PyType_IsSubtype(Py_TYPE(self), &Dtool_DTOOL_SUPER_BASE._PyType) ...but
  // probably not as fast.
  if (Py_TYPE(self)->tp_basicsize >= (int)sizeof(Dtool_PyInstDef)) {
    Dtool_PyInstDef *pyself = (Dtool_PyInstDef *) self;
    if (pyself->_signature == PY_PANDA_SIGNATURE) {
      return true;
    }
  }
  return false;
}

/**

 */
void DTOOL_Call_ExtractThisPointerForType(PyObject *self, Dtool_PyTypedObject *classdef, void **answer) {
  if (DtoolCanThisBeAPandaInstance(self)) {
    *answer = ((Dtool_PyInstDef *)self)->_My_Type->_Dtool_UpcastInterface(self, classdef);
  } else {
    *answer = NULL;
  }
}

/**
 * This is a support function for the Python bindings: it extracts the
 * underlying C++ pointer of the given type for a given Python object.  If it
 * was of the wrong type, raises an AttributeError.
 */
bool Dtool_Call_ExtractThisPointer(PyObject *self, Dtool_PyTypedObject &classdef, void **answer) {
  if (self == NULL || !DtoolCanThisBeAPandaInstance(self) || ((Dtool_PyInstDef *)self)->_ptr_to_object == NULL) {
    Dtool_Raise_TypeError("C++ object is not yet constructed, or already destructed.");
    return false;
  }

  *answer = ((Dtool_PyInstDef *)self)->_My_Type->_Dtool_UpcastInterface(self, &classdef);
  return true;
}

/**
 * The same thing as Dtool_Call_ExtractThisPointer, except that it performs
 * the additional check that the pointer is a non-const pointer.  This is
 * called by function wrappers for functions of which all overloads are non-
 * const, and saves a bit of code.
 *
 * The extra method_name argument is used in formatting the error message.
 */
bool Dtool_Call_ExtractThisPointer_NonConst(PyObject *self, Dtool_PyTypedObject &classdef,
                                            void **answer, const char *method_name) {

  if (self == NULL || !DtoolCanThisBeAPandaInstance(self) || ((Dtool_PyInstDef *)self)->_ptr_to_object == NULL) {
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

/**
 * Extracts the C++ pointer for an object, given its Python wrapper object,
 * for passing as the parameter to a C++ function.
 *
 * self is the Python wrapper object in question.
 *
 * classdef is the Python class wrapper for the C++ class in which the this
 * pointer should be returned.  (This may require an upcast operation, if self
 * is not already an instance of classdef.)
 *
 * param and function_name are used for error reporting only, and describe the
 * particular function and parameter index for this parameter.
 *
 * const_ok is true if the function is declared const and can therefore be
 * called with either a const or non-const "this" pointer, or false if the
 * function is declared non-const, and can therefore be called with only a
 * non-const "this" pointer.
 *
 * The return value is the C++ pointer that was extracted, or NULL if there
 * was a problem (in which case the Python exception state will have been
 * set).
 */
void *
DTOOL_Call_GetPointerThisClass(PyObject *self, Dtool_PyTypedObject *classdef,
                               int param, const string &function_name, bool const_ok,
                               bool report_errors) {
  // if (PyErr_Occurred()) { return NULL; }
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

/**
 * This is similar to a PyErr_Occurred() check, except that it also checks
 * Notify to see if an assertion has occurred.  If that is the case, then it
 * raises an AssertionError.
 *
 * Returns true if there is an active exception, false otherwise.
 *
 * In the NDEBUG case, this is simply a #define to _PyErr_OCCURRED() (which is
 * an undocumented inline version of PyErr_Occurred()).
 */
bool _Dtool_CheckErrorOccurred() {
  if (_PyErr_OCCURRED()) {
    return true;
  }
  if (Notify::ptr()->has_assert_failed()) {
    Dtool_Raise_AssertionError();
    return true;
  }
  return false;
}

/**
 * Raises an AssertionError containing the last thrown assert message, and
 * clears the assertion flag.  Returns NULL.
 */
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

/**
 * Raises a TypeError with the given message, and returns NULL.
 */
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

/**
 * Raises a TypeError of the form: function_name() argument n must be type,
 * not type for a given object passed to a function.
 *
 * Always returns NULL so that it can be conveniently used as a return
 * expression for wrapper functions that return a PyObject pointer.
 */
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

/**
 * Raises an AttributeError of the form: 'type' has no attribute 'attr'
 *
 * Always returns NULL so that it can be conveniently used as a return
 * expression for wrapper functions that return a PyObject pointer.
 */
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

/**
 * Raises a TypeError of the form: Arguments must match: <list of overloads>
 *
 * However, in release builds, this instead is defined to a function that just
 * prints out a generic message, to help reduce the amount of strings in the
 * compiled library.
 *
 * Always returns NULL so that it can be conveniently used as a return
 * expression for wrapper functions that return a PyObject pointer.
 */
PyObject *_Dtool_Raise_BadArgumentsError() {
  return Dtool_Raise_TypeError("arguments do not match any function overload");
}

/**
 * Convenience method that checks for exceptions, and if one occurred, returns
 * NULL, otherwise Py_None.
 */
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

/**
 * Convenience method that checks for exceptions, and if one occurred, returns
 * NULL, otherwise the given boolean value as a PyObject *.
 */
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

/**
 * Convenience method that checks for exceptions, and if one occurred, returns
 * NULL, otherwise the given return value.  Its reference count is not
 * increased.
 */
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

/**
 * Creates a Python 3.4-style enum type.  Steals reference to 'names'.
 */
PyObject *Dtool_EnumType_Create(const char *name, PyObject *names, const char *module) {
  static PyObject *enum_class = NULL;
  static PyObject *enum_meta = NULL;
  static PyObject *enum_create = NULL;
  if (enum_meta == NULL) {
    PyObject *enum_module = PyImport_ImportModule("enum");
    nassertr_always(enum_module != NULL, NULL);

    enum_class = PyObject_GetAttrString(enum_module, "Enum");
    enum_meta = PyObject_GetAttrString(enum_module, "EnumMeta");
    enum_create = PyObject_GetAttrString(enum_meta, "_create_");
    nassertr(enum_meta != NULL, NULL);
  }

  PyObject *result = PyObject_CallFunction(enum_create, (char *)"OsN", enum_class, name, names);
  nassertr(result != NULL, NULL);
  if (module != NULL) {
    PyObject *modstr = PyUnicode_FromString(module);
    PyObject_SetAttrString(result, "__module__", modstr);
    Py_DECREF(modstr);
  }
  return result;
}

/**

 */
PyObject *DTool_CreatePyInstanceTyped(void *local_this_in, Dtool_PyTypedObject &known_class_type, bool memory_rules, bool is_const, int type_index) {
  // We can't do the NULL check here like in DTool_CreatePyInstance, since the
  // caller will have to get the type index to pass to this function to begin
  // with.  That code probably would have crashed by now if it was really NULL
  // for whatever reason.
  nassertr(local_this_in != NULL, NULL);

  // IF the class is possibly a run time typed object
  if (type_index > 0) {
    // get best fit class...
    Dtool_PyTypedObject *target_class = Dtool_RuntimeTypeDtoolType(type_index);
    if (target_class != NULL) {
      // cast to the type...
      void *new_local_this = target_class->_Dtool_DowncastInterface(local_this_in, &known_class_type);
      if (new_local_this != NULL) {
        // ask class to allocate an instance..
        Dtool_PyInstDef *self = (Dtool_PyInstDef *) target_class->_PyType.tp_new(&target_class->_PyType, NULL, NULL);
        if (self != NULL) {
          self->_ptr_to_object = new_local_this;
          self->_memory_rules = memory_rules;
          self->_is_const = is_const;
          // self->_signature = PY_PANDA_SIGNATURE;
          self->_My_Type = target_class;
          return (PyObject *)self;
        }
      }
    }
  }

  // if we get this far .. just wrap the thing in the known type ?? better
  // than aborting...I guess....
  Dtool_PyInstDef *self = (Dtool_PyInstDef *) known_class_type._PyType.tp_new(&known_class_type._PyType, NULL, NULL);
  if (self != NULL) {
    self->_ptr_to_object = local_this_in;
    self->_memory_rules = memory_rules;
    self->_is_const = is_const;
    // self->_signature = PY_PANDA_SIGNATURE;
    self->_My_Type = &known_class_type;
  }
  return (PyObject *)self;
}

// DTool_CreatePyInstance .. wrapper function to finalize the existance of a
// general dtool py instance..
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

// Th Finalizer for simple instances..
int DTool_PyInit_Finalize(PyObject *self, void *local_this, Dtool_PyTypedObject *type, bool memory_rules, bool is_const) {
  // lets put some code in here that checks to see the memory is properly
  // configured.. prior to my call ..

  ((Dtool_PyInstDef *)self)->_My_Type = type;
  ((Dtool_PyInstDef *)self)->_ptr_to_object = local_this;
  ((Dtool_PyInstDef *)self)->_memory_rules = memory_rules;
  ((Dtool_PyInstDef *)self)->_is_const = is_const;
  return 0;
}

// A helper function to glue method definition together .. that can not be
// done at code generation time because of multiple generation passes in
// interrogate..
void Dtool_Accum_MethDefs(PyMethodDef in[], MethodDefmap &themap) {
  for (; in->ml_name != NULL; in++) {
    if (themap.find(in->ml_name) == themap.end()) {
      themap[in->ml_name] = in;
    }
  }
}

// ** HACK ** alert.. Need to keep a runtime type dictionary ... that is
// forward declared of typed object.  We rely on the fact that typed objects
// are uniquly defined by an integer.
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

#if PY_MAJOR_VERSION >= 3
PyObject *Dtool_PyModuleInitHelper(LibraryDef *defs[], PyModuleDef *module_def) {
#else
PyObject *Dtool_PyModuleInitHelper(LibraryDef *defs[], const char *modulename) {
#endif
  // Check the version so we can print a helpful error if it doesn't match.
  string version = Py_GetVersion();

  if (version[0] != '0' + PY_MAJOR_VERSION ||
      version[2] != '0' + PY_MINOR_VERSION) {
    // Raise a helpful error message.  We can safely do this because the
    // signature and behavior for PyErr_SetString has remained consistent.
    ostringstream errs;
    errs << "this module was compiled for Python "
         << PY_MAJOR_VERSION << "." << PY_MINOR_VERSION << ", which is "
         << "incompatible with Python " << version.substr(0, 3);
    string error = errs.str();
    PyErr_SetString(PyExc_ImportError, error.c_str());
    return (PyObject *)NULL;
  }

  // Initialize the types we define in py_panda.
  static bool dtool_inited = false;
  if (!dtool_inited) {
    dtool_inited = true;

    if (PyType_Ready(&Dtool_SequenceWrapper_Type) < 0) {
      return Dtool_Raise_TypeError("PyType_Ready(Dtool_SequenceWrapper)");
    }

    if (PyType_Ready(&Dtool_MappingWrapper_Type) < 0) {
      return Dtool_Raise_TypeError("PyType_Ready(Dtool_MappingWrapper)");
    }

    if (PyType_Ready(&Dtool_SeqMapWrapper_Type) < 0) {
      return Dtool_Raise_TypeError("PyType_Ready(Dtool_SeqMapWrapper)");
    }

    if (PyType_Ready(&Dtool_StaticProperty_Type) < 0) {
      return Dtool_Raise_TypeError("PyType_Ready(Dtool_StaticProperty_Type)");
    }

    // Initialize the "empty tuple".
    (void)PyObject_INIT_VAR((PyObject *)&Dtool_EmptyTuple, &PyTuple_Type, 0);

    // Initialize the base class of everything.
    Dtool_PyModuleClassInit_DTOOL_SUPER_BASE(NULL);
  }

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

  // MAIN_DIR needs to be set very early; this seems like a convenient place
  // to do that.  Perhaps we'll find a better place for this in the future.
  static bool initialized_main_dir = false;
  if (!initialized_main_dir) {
    if (interrogatedb_cat.is_debug()) {
      // Good opportunity to print this out once, at startup.
      interrogatedb_cat.debug()
        << "Python " << version << "\n";
    }

    // Grab the __main__ module.
    PyObject *main_module = PyImport_ImportModule("__main__");
    if (main_module == NULL) {
      interrogatedb_cat.warning() << "Unable to import __main__\n";
    }

    // Extract the __file__ attribute, if present.
    Filename main_dir;
    PyObject *file_attr = PyObject_GetAttrString(main_module, "__file__");
    if (file_attr == NULL) {
      // Must be running in the interactive interpreter.  Use the CWD.
      main_dir = ExecutionEnvironment::get_cwd();
    } else {
#if PY_MAJOR_VERSION >= 3
      Py_ssize_t length;
      wchar_t *buffer = PyUnicode_AsWideCharString(file_attr, &length);
      if (buffer != NULL) {
        main_dir = Filename::from_os_specific_w(std::wstring(buffer, length));
        main_dir.make_absolute();
        main_dir = main_dir.get_dirname();
        PyMem_Free(buffer);
      }
#else
      char *buffer;
      Py_ssize_t length;
      if (PyString_AsStringAndSize(file_attr, &buffer, &length) != -1) {
        main_dir = Filename::from_os_specific(std::string(buffer, length));
        main_dir.make_absolute();
        main_dir = main_dir.get_dirname();
      }
#endif
      else {
        interrogatedb_cat.warning() << "Invalid string for __main__.__file__\n";
      }
    }
    ExecutionEnvironment::shadow_environment_variable("MAIN_DIR", main_dir.to_os_specific());
    PyErr_Clear();
    initialized_main_dir = true;
  }

  PyModule_AddIntConstant(module, "Dtool_PyNativeInterface", 1);
  return module;
}

// HACK.... Be careful Dtool_BorrowThisReference This function can be used to
// grab the "THIS" pointer from an object and use it Required to support
// historical inheritance in the form of "is this instance of"..
PyObject *Dtool_BorrowThisReference(PyObject *self, PyObject *args) {
  PyObject *from_in = NULL;
  PyObject *to_in = NULL;
  if (PyArg_UnpackTuple(args, "Dtool_BorrowThisReference", 2, 2, &to_in, &from_in)) {

    if (DtoolCanThisBeAPandaInstance(from_in) && DtoolCanThisBeAPandaInstance(to_in)) {
      Dtool_PyInstDef *from = (Dtool_PyInstDef *) from_in;
      Dtool_PyInstDef *to = (Dtool_PyInstDef *) to_in;

      // if (PyObject_TypeCheck(to_in, Py_TYPE(from_in))) {
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

// We do expose a dictionay for dtool classes .. this should be removed at
// some point..
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
#if PY_VERSION_HEX >= 0x03060000
    PyObject *res = _PyObject_FastCall(func, &v2, 1);
#else
    PyObject *res = NULL;
    PyObject *args = PyTuple_Pack(1, v2);
    if (args != NULL) {
      res = PyObject_Call(func, args, NULL);
      Py_DECREF(args);
    }
#endif
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
  NODEFAULT
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
    break;
  }
  return PyBool_FromLong(result);
}

/**
 * This is a support function for a synthesized __copy__() method from a C++
 * make_copy() method.
 */
PyObject *copy_from_make_copy(PyObject *self, PyObject *noargs) {
  PyObject *callable = PyObject_GetAttrString(self, "make_copy");
  if (callable == NULL) {
    return NULL;
  }
  PyObject *result = _PyObject_CallNoArg(callable);
  Py_DECREF(callable);
  return result;
}

/**
 * This is a support function for a synthesized __copy__() method from a C++
 * copy constructor.
 */
PyObject *copy_from_copy_constructor(PyObject *self, PyObject *noargs) {
  PyObject *callable = (PyObject *)Py_TYPE(self);

#if PY_VERSION_HEX >= 0x03060000
  PyObject *result = _PyObject_FastCall(callable, &self, 1);
#else
  PyObject *args = PyTuple_Pack(1, self);
  PyObject *result = PyObject_Call(callable, args, NULL);
  Py_DECREF(args);
#endif
  return result;
}

/**
 * This is a support function for a synthesized __deepcopy__() method for any
 * class that has a __copy__() method.  The sythethic method simply invokes
 * __copy__().
 */
PyObject *map_deepcopy_to_copy(PyObject *self, PyObject *args) {
  PyObject *callable = PyObject_GetAttrString(self, "__copy__");
  if (callable == NULL) {
    return NULL;
  }
  PyObject *result = _PyObject_CallNoArg(callable);
  Py_DECREF(callable);
  return result;
}

/**
 * A more efficient version of PyArg_ParseTupleAndKeywords for the special
 * case where there is only a single PyObject argument.
 */
bool Dtool_ExtractArg(PyObject **result, PyObject *args, PyObject *kwds,
                      const char *keyword) {

  if (PyTuple_GET_SIZE(args) == 1) {
    if (kwds == NULL || ((PyDictObject *)kwds)->ma_used == 0) {
      *result = PyTuple_GET_ITEM(args, 0);
      return true;
    }
  } else if (PyTuple_GET_SIZE(args) == 0) {
    PyObject *key;
    Py_ssize_t ppos = 0;
    if (kwds != NULL && ((PyDictObject *)kwds)->ma_used == 1 &&
        PyDict_Next(kwds, &ppos, &key, result)) {
      // We got the item, we just need to make sure that it had the right key.
#if PY_VERSION_HEX >= 0x03060000
      return PyUnicode_CheckExact(key) && _PyUnicode_EqualToASCIIString(key, keyword);
#elif PY_MAJOR_VERSION >= 3
      return PyUnicode_CheckExact(key) && PyUnicode_CompareWithASCIIString(key, keyword) == 0;
#else
      return PyString_CheckExact(key) && strcmp(PyString_AS_STRING(key), keyword) == 0;
#endif
    }
  }

  return false;
}

/**
 * Variant of Dtool_ExtractArg that does not accept a keyword argument.
 */
bool Dtool_ExtractArg(PyObject **result, PyObject *args, PyObject *kwds) {
  if (PyTuple_GET_SIZE(args) == 1 &&
      (kwds == NULL || ((PyDictObject *)kwds)->ma_used == 0)) {
    *result = PyTuple_GET_ITEM(args, 0);
    return true;
  }
  return false;
}

/**
 * A more efficient version of PyArg_ParseTupleAndKeywords for the special
 * case where there is only a single optional PyObject argument.
 *
 * Returns true if valid (including if there were 0 items), false if there was
 * an error, such as an invalid number of parameters.
 */
bool Dtool_ExtractOptionalArg(PyObject **result, PyObject *args, PyObject *kwds,
                              const char *keyword) {

  if (PyTuple_GET_SIZE(args) == 1) {
    if (kwds == NULL || ((PyDictObject *)kwds)->ma_used == 0) {
      *result = PyTuple_GET_ITEM(args, 0);
      return true;
    }
  } else if (PyTuple_GET_SIZE(args) == 0) {
    if (kwds != NULL && ((PyDictObject *)kwds)->ma_used == 1) {
      PyObject *key;
      Py_ssize_t ppos = 0;
      if (!PyDict_Next(kwds, &ppos, &key, result)) {
        return true;
      }

      // We got the item, we just need to make sure that it had the right key.
#if PY_VERSION_HEX >= 0x03060000
      return PyUnicode_CheckExact(key) && _PyUnicode_EqualToASCIIString(key, keyword);
#elif PY_MAJOR_VERSION >= 3
      return PyUnicode_CheckExact(key) && PyUnicode_CompareWithASCIIString(key, keyword) == 0;
#else
      return PyString_CheckExact(key) && strcmp(PyString_AS_STRING(key), keyword) == 0;
#endif
    } else {
      return true;
    }
  }

  return false;
}

/**
 * Variant of Dtool_ExtractOptionalArg that does not accept a keyword argument.
 */
bool Dtool_ExtractOptionalArg(PyObject **result, PyObject *args, PyObject *kwds) {
  if (kwds != NULL && ((PyDictObject *)kwds)->ma_used != 0) {
    return false;
  }
  if (PyTuple_GET_SIZE(args) == 1) {
    *result = PyTuple_GET_ITEM(args, 0);
    return true;
  }
  return (PyTuple_GET_SIZE(args) == 0);
}

/**
 * These classes are returned from properties that require a subscript
 * interface, ie. something.children[i] = 3.
 */
static void Dtool_WrapperBase_dealloc(PyObject *self) {
  Dtool_WrapperBase *wrap = (Dtool_WrapperBase *)self;
  nassertv(wrap);
  Py_XDECREF(wrap->_self);
}

static Py_ssize_t Dtool_SequenceWrapper_length(PyObject *self) {
  Dtool_SequenceWrapper *wrap = (Dtool_SequenceWrapper *)self;
  nassertr(wrap, -1);
  if (wrap->_len_func != nullptr) {
    nassertr(wrap->_len_func, -1);
    return wrap->_len_func(wrap->_base._self);
  } else {
    Dtool_Raise_TypeError("property does not support len()");
    return -1;
  }
}

static PyObject *Dtool_SequenceWrapper_getitem(PyObject *self, Py_ssize_t index) {
  Dtool_SequenceWrapper *wrap = (Dtool_SequenceWrapper *)self;
  nassertr(wrap, nullptr);
  nassertr(wrap->_getitem_func, nullptr);
  return wrap->_getitem_func(wrap->_base._self, index);
}

static int Dtool_SequenceWrapper_setitem(PyObject *self, Py_ssize_t index, PyObject *value) {
  Dtool_SequenceWrapper *wrap = (Dtool_SequenceWrapper *)self;
  nassertr(wrap, -1);
  if (wrap->_setitem_func != nullptr) {
    return wrap->_setitem_func(wrap->_base._self, index, value);
  } else {
    Dtool_Raise_TypeError("property does not support item assignment");
    return -1;
  }
}

static PyObject *Dtool_MappingWrapper_getitem(PyObject *self, PyObject *key) {
  Dtool_MappingWrapper *wrap = (Dtool_MappingWrapper *)self;
  nassertr(wrap, nullptr);
  nassertr(wrap->_getitem_func, nullptr);
  return wrap->_getitem_func(wrap->_base._self, key);
}

static int Dtool_MappingWrapper_setitem(PyObject *self, PyObject *key, PyObject *value) {
  Dtool_MappingWrapper *wrap = (Dtool_MappingWrapper *)self;
  nassertr(wrap, -1);
  if (wrap->_setitem_func != nullptr) {
    return wrap->_setitem_func(wrap->_base._self, key, value);
  } else {
    Dtool_Raise_TypeError("property does not support item assignment");
    return -1;
  }
}

static PyObject *Dtool_SeqMapWrapper_getitem(PyObject *self, PyObject *key) {
  Dtool_SeqMapWrapper *wrap = (Dtool_SeqMapWrapper *)self;
  nassertr(wrap, nullptr);
  nassertr(wrap->_map_getitem_func, nullptr);
  return wrap->_map_getitem_func(wrap->_seq._base._self, key);
}

static int Dtool_SeqMapWrapper_setitem(PyObject *self, PyObject *key, PyObject *value) {
  Dtool_SeqMapWrapper *wrap = (Dtool_SeqMapWrapper *)self;
  nassertr(wrap, -1);
  if (wrap->_map_setitem_func != nullptr) {
    return wrap->_map_setitem_func(wrap->_seq._base._self, key, value);
  } else {
    Dtool_Raise_TypeError("property does not support item assignment");
    return -1;
  }
}

static PySequenceMethods Dtool_SequenceWrapper_SequenceMethods = {
  Dtool_SequenceWrapper_length,
  0, // sq_concat
  0, // sq_repeat
  Dtool_SequenceWrapper_getitem,
  0, // sq_slice
  Dtool_SequenceWrapper_setitem,
  0, // sq_ass_slice
  0, // sq_contains
  0, // sq_inplace_concat
  0, // sq_inplace_repeat
};

static PyMappingMethods Dtool_MappingWrapper_MappingMethods = {
  0, // mp_length
  Dtool_MappingWrapper_getitem,
  Dtool_MappingWrapper_setitem,
};

static PyMappingMethods Dtool_SeqMapWrapper_MappingMethods = {
  Dtool_SequenceWrapper_length,
  Dtool_SeqMapWrapper_getitem,
  Dtool_SeqMapWrapper_setitem,
};

/**
 * This variant defines only a sequence interface.
 */
PyTypeObject Dtool_SequenceWrapper_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "sequence wrapper",
  sizeof(Dtool_SequenceWrapper),
  0, // tp_itemsize
  Dtool_WrapperBase_dealloc,
  0, // tp_print
  0, // tp_getattr
  0, // tp_setattr
#if PY_MAJOR_VERSION >= 3
  0, // tp_reserved
#else
  0, // tp_compare
#endif
  0, // tp_repr
  0, // tp_as_number
  &Dtool_SequenceWrapper_SequenceMethods,
  0, // tp_as_mapping
  0, // tp_hash
  0, // tp_call
  0, // tp_str
  PyObject_GenericGetAttr,
  PyObject_GenericSetAttr,
  0, // tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_CHECKTYPES,
  0, // tp_doc
  0, // tp_traverse
  0, // tp_clear
  0, // tp_richcompare
  0, // tp_weaklistoffset
  0, // tp_iter
  0, // tp_iternext
  0, // tp_methods
  0, // tp_members
  0, // tp_getset
  0, // tp_base
  0, // tp_dict
  0, // tp_descr_get
  0, // tp_descr_set
  0, // tp_dictoffset
  0, // tp_init
  PyType_GenericAlloc,
  0, // tp_new
  PyObject_Del,
  0, // tp_is_gc
  0, // tp_bases
  0, // tp_mro
  0, // tp_cache
  0, // tp_subclasses
  0, // tp_weaklist
  0, // tp_del
#if PY_VERSION_HEX >= 0x02060000
  0, // tp_version_tag
#endif
#if PY_VERSION_HEX >= 0x03040000
  0, // tp_finalize
#endif
};

/**
 * This is a variant of the Python getset mechanism that permits static
 * properties.
 */
PyObject *
Dtool_NewStaticProperty(PyTypeObject *type, const PyGetSetDef *getset) {
  PyGetSetDescrObject *descr;
  descr = (PyGetSetDescrObject *)PyType_GenericAlloc(&Dtool_StaticProperty_Type, 0);
  if (descr != nullptr) {
    Py_XINCREF(type);
    descr->d_getset = (PyGetSetDef *)getset;
#if PY_MAJOR_VERSION >= 3
    descr->d_common.d_type = type;
    descr->d_common.d_name = PyUnicode_InternFromString(getset->name);
#if PY_VERSION_HEX >= 0x03030000
    descr->d_common.d_qualname = nullptr;
#endif
#else
    descr->d_type = type;
    descr->d_name = PyString_InternFromString(getset->name);
#endif
  }
  return (PyObject *)descr;
}

static void
Dtool_StaticProperty_dealloc(PyDescrObject *descr) {
  _PyObject_GC_UNTRACK(descr);
  Py_XDECREF(descr->d_type);
  Py_XDECREF(descr->d_name);
//#if PY_MAJOR_VERSION >= 3
//  Py_XDECREF(descr->d_qualname);
//#endif
  PyObject_GC_Del(descr);
}

static PyObject *
Dtool_StaticProperty_repr(PyDescrObject *descr, const char *format) {
#if PY_MAJOR_VERSION >= 3
  return PyUnicode_FromFormat("<attribute '%V' of '%s'>", descr->d_name, "?", descr->d_type->tp_name);
#else
  return PyString_FromFormat("<attribute '%V' of '%s'>", descr->d_name, "?", descr->d_type->tp_name);
#endif
}

static int
Dtool_StaticProperty_traverse(PyObject *self, visitproc visit, void *arg) {
  PyDescrObject *descr = (PyDescrObject *)self;
  Py_VISIT(descr->d_type);
  return 0;
}

static PyObject *
Dtool_StaticProperty_get(PyGetSetDescrObject *descr, PyObject *obj, PyObject *type) {
  if (descr->d_getset->get != nullptr) {
    return descr->d_getset->get(obj, descr->d_getset->closure);
  } else {
    return PyErr_Format(PyExc_AttributeError,
                        "attribute '%V' of type '%.100s' is not readable",
                        ((PyDescrObject *)descr)->d_name, "?",
                        ((PyDescrObject *)descr)->d_type->tp_name);
  }
}

static int
Dtool_StaticProperty_set(PyGetSetDescrObject *descr, PyObject *obj, PyObject *value) {
  if (descr->d_getset->set != nullptr) {
    return descr->d_getset->set(obj, value, descr->d_getset->closure);
  } else {
    PyErr_Format(PyExc_AttributeError,
                 "attribute '%V' of type '%.100s' is not writable",
                 ((PyDescrObject *)descr)->d_name, "?",
                 ((PyDescrObject *)descr)->d_type->tp_name);
    return -1;
  }
}

PyTypeObject Dtool_StaticProperty_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  "getset_descriptor",
  sizeof(PyGetSetDescrObject),
  0, // tp_itemsize
  (destructor)Dtool_StaticProperty_dealloc,
  0, // tp_print
  0, // tp_getattr
  0, // tp_setattr
  0, // tp_reserved
  (reprfunc)Dtool_StaticProperty_repr,
  0, // tp_as_number
  0, // tp_as_sequence
  0, // tp_as_mapping
  0, // tp_hash
  0, // tp_call
  0, // tp_str
  PyObject_GenericGetAttr,
  0, // tp_setattro
  0, // tp_as_buffer
  Py_TPFLAGS_DEFAULT,
  0, // tp_doc
  Dtool_StaticProperty_traverse,
  0, // tp_clear
  0, // tp_richcompare
  0, // tp_weaklistoffset
  0, // tp_iter
  0, // tp_iternext
  0, // tp_methods
  0, // tp_members
  0, // tp_getset
  0, // tp_base
  0, // tp_dict
  (descrgetfunc)Dtool_StaticProperty_get,
  (descrsetfunc)Dtool_StaticProperty_set,
  0, // tp_dictoffset
  0, // tp_init
  0, // tp_alloc
  0, // tp_new
  0, // tp_del
  0, // tp_is_gc
  0, // tp_bases
  0, // tp_mro
  0, // tp_cache
  0, // tp_subclasses
  0, // tp_weaklist
  0, // tp_del
#if PY_VERSION_HEX >= 0x02060000
  0, // tp_version_tag
#endif
#if PY_VERSION_HEX >= 0x03040000
  0, // tp_finalize
#endif
};

/**
 * This variant defines only a mapping interface.
 */
PyTypeObject Dtool_MappingWrapper_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "mapping wrapper",
  sizeof(Dtool_MappingWrapper),
  0, // tp_itemsize
  Dtool_WrapperBase_dealloc,
  0, // tp_print
  0, // tp_getattr
  0, // tp_setattr
#if PY_MAJOR_VERSION >= 3
  0, // tp_reserved
#else
  0, // tp_compare
#endif
  0, // tp_repr
  0, // tp_as_number
  0, // tp_as_sequence
  &Dtool_MappingWrapper_MappingMethods,
  0, // tp_hash
  0, // tp_call
  0, // tp_str
  PyObject_GenericGetAttr,
  PyObject_GenericSetAttr,
  0, // tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_CHECKTYPES,
  0, // tp_doc
  0, // tp_traverse
  0, // tp_clear
  0, // tp_richcompare
  0, // tp_weaklistoffset
  0, // tp_iter
  0, // tp_iternext
  0, // tp_methods
  0, // tp_members
  0, // tp_getset
  0, // tp_base
  0, // tp_dict
  0, // tp_descr_get
  0, // tp_descr_set
  0, // tp_dictoffset
  0, // tp_init
  PyType_GenericAlloc,
  0, // tp_new
  PyObject_Del,
  0, // tp_is_gc
  0, // tp_bases
  0, // tp_mro
  0, // tp_cache
  0, // tp_subclasses
  0, // tp_weaklist
  0, // tp_del
#if PY_VERSION_HEX >= 0x02060000
  0, // tp_version_tag
#endif
#if PY_VERSION_HEX >= 0x03040000
  0, // tp_finalize
#endif
};

/**
 * This variant defines both a sequence and mapping interface.
 */
PyTypeObject Dtool_SeqMapWrapper_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "sequence/mapping wrapper",
  sizeof(Dtool_SeqMapWrapper),
  0, // tp_itemsize
  Dtool_WrapperBase_dealloc,
  0, // tp_print
  0, // tp_getattr
  0, // tp_setattr
#if PY_MAJOR_VERSION >= 3
  0, // tp_reserved
#else
  0, // tp_compare
#endif
  0, // tp_repr
  0, // tp_as_number
  &Dtool_SequenceWrapper_SequenceMethods,
  &Dtool_SeqMapWrapper_MappingMethods,
  0, // tp_hash
  0, // tp_call
  0, // tp_str
  PyObject_GenericGetAttr,
  PyObject_GenericSetAttr,
  0, // tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_CHECKTYPES,
  0, // tp_doc
  0, // tp_traverse
  0, // tp_clear
  0, // tp_richcompare
  0, // tp_weaklistoffset
  0, // tp_iter
  0, // tp_iternext
  0, // tp_methods
  0, // tp_members
  0, // tp_getset
  0, // tp_base
  0, // tp_dict
  0, // tp_descr_get
  0, // tp_descr_set
  0, // tp_dictoffset
  0, // tp_init
  PyType_GenericAlloc,
  0, // tp_new
  PyObject_Del,
  0, // tp_is_gc
  0, // tp_bases
  0, // tp_mro
  0, // tp_cache
  0, // tp_subclasses
  0, // tp_weaklist
  0, // tp_del
#if PY_VERSION_HEX >= 0x02060000
  0, // tp_version_tag
#endif
#if PY_VERSION_HEX >= 0x03040000
  0, // tp_finalize
#endif
};

#endif  // HAVE_PYTHON
