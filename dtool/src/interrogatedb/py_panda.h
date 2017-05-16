/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file py_panda.h
 */

#ifndef PY_PANDA_H_
#define PY_PANDA_H_

#include <set>
#include <map>
#include <string>

#ifdef USE_DEBUG_PYTHON
#define  Py_DEBUG
#endif

#ifndef NO_RUNTIME_TYPES

#include "dtoolbase.h"
#include "typedObject.h"
#include "typeRegistry.h"

#endif

#include "pnotify.h"

#if defined(HAVE_PYTHON) && !defined(CPPPARSER)

#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#endif

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif

#define PY_SSIZE_T_CLEAN 1

#include "Python.h"
#include "structmember.h"

#ifndef HAVE_LONG_LONG
#define PyLong_FromLongLong(x) PyLong_FromLong((long) (x))
#define PyLong_FromUnsignedLongLong(x) PyLong_FromUnsignedLong((unsigned long) (x))
#define PyLong_AsLongLong(x) PyLong_AsLong(x)
#define PyLong_AsUnsignedLongLong(x) PyLong_AsUnsignedLong(x)
#define PyLong_AsUnsignedLongLongMask(x) PyLong_AsUnsignedLongMask(x)
#define PyLong_AsLongLongAndOverflow(x) PyLong_AsLongAndOverflow(x)
#endif

#if PY_VERSION_HEX < 0x02050000

// Prior to Python 2.5, we didn't have Py_ssize_t.
typedef int Py_ssize_t;
#define PyInt_FromSsize_t PyInt_FromLong
#define PyInt_AsSsize_t PyInt_AsLong

#endif  // PY_VERSION_HEX

// 2.4 macros which aren't available in 2.3
#ifndef Py_RETURN_NONE
inline PyObject* doPy_RETURN_NONE()
{   Py_INCREF(Py_None); return Py_None; }
#define Py_RETURN_NONE return doPy_RETURN_NONE()
#endif

#ifndef Py_RETURN_TRUE
inline PyObject* doPy_RETURN_TRUE()
{Py_INCREF(Py_True); return Py_True;}
#define Py_RETURN_TRUE return doPy_RETURN_TRUE()
#endif

#ifndef Py_RETURN_FALSE
inline PyObject* doPy_RETURN_FALSE()
{Py_INCREF(Py_False); return Py_False;}
#define Py_RETURN_FALSE return doPy_RETURN_FALSE()
#endif

#ifndef PyVarObject_HEAD_INIT
#define PyVarObject_HEAD_INIT(type, size) \
  PyObject_HEAD_INIT(type) size,
#endif

#ifndef Py_TYPE
#define Py_TYPE(ob) (((PyObject*)(ob))->ob_type)
#endif

#ifndef Py_TPFLAGS_CHECKTYPES
// Always on in Python 3
#define Py_TPFLAGS_CHECKTYPES 0
#endif

#if PY_MAJOR_VERSION >= 3
// For writing code that will compile in both versions.
#define nb_nonzero nb_bool
#define nb_divide nb_true_divide
#define nb_inplace_divide nb_inplace_true_divide

#define PyLongOrInt_Check(x) PyLong_Check(x)
#define PyLongOrInt_AS_LONG PyLong_AS_LONG
#define PyInt_Check PyLong_Check
#define PyInt_AsLong PyLong_AsLong
#define PyInt_AS_LONG PyLong_AS_LONG
#define PyLongOrInt_AsSize_t PyLong_AsSize_t
#else
#define PyLongOrInt_Check(x) (PyInt_Check(x) || PyLong_Check(x))
// PyInt_FromSize_t automatically picks the right type.
#define PyLongOrInt_AS_LONG PyInt_AsLong

EXPCL_INTERROGATEDB size_t PyLongOrInt_AsSize_t(PyObject *);

// For more portably defining hash functions.
typedef long Py_hash_t;
#endif

#if PY_MAJOR_VERSION >= 3
// Python 3 versions before 3.3.3 defined this incorrectly.
#undef _PyErr_OCCURRED
#define _PyErr_OCCURRED() (PyThreadState_GET()->curexc_type)

// Python versions before 3.3 did not define this.
#if PY_VERSION_HEX < 0x03030000
#define PyUnicode_AsUTF8 _PyUnicode_AsString
#define PyUnicode_AsUTF8AndSize _PyUnicode_AsStringAndSize
#endif
#endif

// Which character to use in PyArg_ParseTuple et al for a byte string.
#if PY_MAJOR_VERSION >= 3
#define FMTCHAR_BYTES "y"
#else
#define FMTCHAR_BYTES "s"
#endif

using namespace std;

// this is tempory .. untill this is glued better into the panda build system

#if defined(_WIN32) && !defined(LINK_ALL_STATIC)
#define EXPORT_THIS __declspec(dllexport)
#define IMPORT_THIS extern __declspec(dllimport)
#else
#define EXPORT_THIS
#define IMPORT_THIS extern
#endif

struct Dtool_PyTypedObject;
typedef std::map<int, Dtool_PyTypedObject *> RuntimeTypeMap;
typedef std::set<int> RuntimeTypeSet;
typedef std::map<std::string, Dtool_PyTypedObject *> NamedTypeMap;

// used to stamp dtool instance..
#define PY_PANDA_SIGNATURE 0xbeaf
typedef void *(*UpcastFunction)(PyObject *,Dtool_PyTypedObject *);
typedef void *(*DowncastFunction)(void *, Dtool_PyTypedObject *);
typedef void *(*CoerceFunction)(PyObject *, void *);
typedef void (*ModuleClassInitFunction)(PyObject *module);

// INSTANCE CONTAINER FOR ALL panda py objects....
struct Dtool_PyInstDef {
  PyObject_HEAD

  // This is a pointer to the Dtool_PyTypedObject type.  It's tempting not to
  // store this and to instead use PY_TYPE(self) and upcast that, but that
  // breaks when someone inherits from our class in Python.
  struct Dtool_PyTypedObject *_My_Type;

  // Pointer to the underlying C++ object.
  void *_ptr_to_object;

  // This is always set to PY_PANDA_SIGNATURE, so that we can quickly detect
  // whether an object is a Panda object.
  unsigned short _signature;

  // True if we own the pointer and should delete it or unref it.
  bool _memory_rules;

  // True if this is a "const" pointer.
  bool _is_const;
};

// A Offset Dictionary Defining How to read the Above Object..
extern EXPCL_INTERROGATEDB PyMemberDef standard_type_members[];

// The Class Definition Structor For a Dtool python type.
struct Dtool_PyTypedObject {
  // Standard Python Features..
  PyTypeObject _PyType;

  // May be TypeHandle::none() to indicate a non-TypedObject class.
  TypeHandle _type;

  ModuleClassInitFunction _Dtool_ModuleClassInit;

  UpcastFunction _Dtool_UpcastInterface;    // The Upcast Function By Slot
  DowncastFunction _Dtool_DowncastInterface; // The Downcast Function By Slot

  CoerceFunction _Dtool_ConstCoerce;
  CoerceFunction _Dtool_Coerce;
};

// This is now simply a forward declaration.  The actual definition is created
// by the code generator.
#define Define_Dtool_Class(MODULE_NAME, CLASS_NAME, PUBLIC_NAME) \
  extern Dtool_PyTypedObject Dtool_##CLASS_NAME;

// More Macro(s) to Implement class functions.. Usually used if C++ needs type
// information
#define Define_Dtool_new(CLASS_NAME,CNAME)\
static PyObject *Dtool_new_##CLASS_NAME(PyTypeObject *type, PyObject *args, PyObject *kwds) {\
  (void) args; (void) kwds;\
  PyObject *self = type->tp_alloc(type, 0);\
  ((Dtool_PyInstDef *)self)->_signature = PY_PANDA_SIGNATURE;\
  ((Dtool_PyInstDef *)self)->_My_Type = &Dtool_##CLASS_NAME;\
  return self;\
}

// The following used to be in the above macro, but it doesn't seem to be
// necessary as tp_alloc memsets the object to 0.
//  ((Dtool_PyInstDef *)self)->_ptr_to_object = NULL;
//  ((Dtool_PyInstDef *)self)->_memory_rules = false;
//  ((Dtool_PyInstDef *)self)->_is_const = false;

// Delete functions..
#ifdef NDEBUG
#define Define_Dtool_FreeInstance_Private(CLASS_NAME,CNAME)\
static void Dtool_FreeInstance_##CLASS_NAME(PyObject *self) {\
  Py_TYPE(self)->tp_free(self);\
}
#else // NDEBUG
#define Define_Dtool_FreeInstance_Private(CLASS_NAME,CNAME)\
static void Dtool_FreeInstance_##CLASS_NAME(PyObject *self) {\
  if (((Dtool_PyInstDef *)self)->_ptr_to_object != NULL) {\
    if (((Dtool_PyInstDef *)self)->_memory_rules) {\
      cerr << "Detected leak for " << #CLASS_NAME \
           << " which interrogate cannot delete.\n"; \
    }\
  }\
  Py_TYPE(self)->tp_free(self);\
}
#endif  // NDEBUG

#define Define_Dtool_FreeInstance(CLASS_NAME,CNAME)\
static void Dtool_FreeInstance_##CLASS_NAME(PyObject *self) {\
  if (((Dtool_PyInstDef *)self)->_ptr_to_object != NULL) {\
    if (((Dtool_PyInstDef *)self)->_memory_rules) {\
      delete ((CNAME *)((Dtool_PyInstDef *)self)->_ptr_to_object);\
    }\
  }\
  Py_TYPE(self)->tp_free(self);\
}

#define Define_Dtool_FreeInstanceRef(CLASS_NAME,CNAME)\
static void Dtool_FreeInstance_##CLASS_NAME(PyObject *self) {\
  if (((Dtool_PyInstDef *)self)->_ptr_to_object != NULL) {\
    if (((Dtool_PyInstDef *)self)->_memory_rules) {\
      unref_delete((CNAME *)((Dtool_PyInstDef *)self)->_ptr_to_object);\
    }\
  }\
  Py_TYPE(self)->tp_free(self);\
}

#define Define_Dtool_Simple_FreeInstance(CLASS_NAME, CNAME)\
static void Dtool_FreeInstance_##CLASS_NAME(PyObject *self) {\
  ((Dtool_InstDef_##CLASS_NAME *)self)->_value.~##CLASS_NAME();\
  Py_TYPE(self)->tp_free(self);\
}

// Simple Recognition Functions..
EXPCL_INTERROGATEDB bool DtoolCanThisBeAPandaInstance(PyObject *self);

// ** HACK ** allert.. Need to keep a runtime type dictionary ... that is
// forward declared of typed object.  We rely on the fact that typed objects
// are uniquly defined by an integer.

EXPCL_INTERROGATEDB void RegisterNamedClass(const string &name, Dtool_PyTypedObject &otype);
EXPCL_INTERROGATEDB void RegisterRuntimeTypedClass(Dtool_PyTypedObject &otype);

EXPCL_INTERROGATEDB Dtool_PyTypedObject *LookupNamedClass(const string &name);
EXPCL_INTERROGATEDB Dtool_PyTypedObject *LookupRuntimeTypedClass(TypeHandle handle);

EXPCL_INTERROGATEDB Dtool_PyTypedObject *Dtool_RuntimeTypeDtoolType(int type);

/**

 */
EXPCL_INTERROGATEDB void DTOOL_Call_ExtractThisPointerForType(PyObject *self, Dtool_PyTypedObject *classdef, void **answer);

EXPCL_INTERROGATEDB void *DTOOL_Call_GetPointerThisClass(PyObject *self, Dtool_PyTypedObject *classdef, int param, const string &function_name, bool const_ok, bool report_errors);

EXPCL_INTERROGATEDB void *DTOOL_Call_GetPointerThis(PyObject *self);

EXPCL_INTERROGATEDB bool Dtool_Call_ExtractThisPointer(PyObject *self, Dtool_PyTypedObject &classdef, void **answer);

EXPCL_INTERROGATEDB bool Dtool_Call_ExtractThisPointer_NonConst(PyObject *self, Dtool_PyTypedObject &classdef,
                                                              void **answer, const char *method_name);

template<class T> INLINE bool DTOOL_Call_ExtractThisPointer(PyObject *self, T *&into);

// Functions related to error reporting.
EXPCL_INTERROGATEDB bool _Dtool_CheckErrorOccurred();

// _PyErr_OCCURRED is an undocumented macro version of PyErr_Occurred.
// Some implementations of the CPython API (e.g. PyPy's cpyext) do not define
// it, so in these cases we just silently fall back to PyErr_Occurred.
#ifndef _PyErr_OCCURRED
#define _PyErr_OCCURRED() PyErr_Occurred()
#endif

#ifdef NDEBUG
#define Dtool_CheckErrorOccurred() (_PyErr_OCCURRED() != NULL)
#else
#define Dtool_CheckErrorOccurred() _Dtool_CheckErrorOccurred()
#endif

EXPCL_INTERROGATEDB PyObject *Dtool_Raise_AssertionError();
EXPCL_INTERROGATEDB PyObject *Dtool_Raise_TypeError(const char *message);
EXPCL_INTERROGATEDB PyObject *Dtool_Raise_ArgTypeError(PyObject *obj, int param, const char *function_name, const char *type_name);
EXPCL_INTERROGATEDB PyObject *Dtool_Raise_AttributeError(PyObject *obj, const char *attribute);

EXPCL_INTERROGATEDB PyObject *_Dtool_Raise_BadArgumentsError();
#ifdef NDEBUG
// Define it to a function that just prints a generic message.
#define Dtool_Raise_BadArgumentsError(x) _Dtool_Raise_BadArgumentsError()
#else
// Expand this to a TypeError listing all of the overloads.
#define Dtool_Raise_BadArgumentsError(x) Dtool_Raise_TypeError("Arguments must match:\n" x)
#endif

EXPCL_INTERROGATEDB PyObject *_Dtool_Return_None();
EXPCL_INTERROGATEDB PyObject *Dtool_Return_Bool(bool value);
EXPCL_INTERROGATEDB PyObject *_Dtool_Return(PyObject *value);

#ifdef NDEBUG
#define Dtool_Return_None() (_PyErr_OCCURRED() != NULL ? NULL : (Py_INCREF(Py_None), Py_None))
#define Dtool_Return(value) (_PyErr_OCCURRED() != NULL ? NULL : value)
#else
#define Dtool_Return_None() _Dtool_Return_None()
#define Dtool_Return(value) _Dtool_Return(value)
#endif

/**
 * Wrapper around Python 3.4's enum library, which does not have a C API.
 */
EXPCL_INTERROGATEDB PyObject *Dtool_EnumType_Create(const char *name, PyObject *names,
                                                    const char *module = NULL);

/**

 */
EXPCL_INTERROGATEDB PyObject *DTool_CreatePyInstanceTyped(void *local_this_in, Dtool_PyTypedObject &known_class_type, bool memory_rules, bool is_const, int RunTimeType);

// DTool_CreatePyInstance .. wrapper function to finalize the existance of a
// general dtool py instance..
EXPCL_INTERROGATEDB PyObject *DTool_CreatePyInstance(void *local_this, Dtool_PyTypedObject &in_classdef, bool memory_rules, bool is_const);

// These template methods allow use when the Dtool_PyTypedObject is not known.
// They require a get_class_type() to be defined for the class.
template<class T> INLINE PyObject *DTool_CreatePyInstance(const T *obj, bool memory_rules);
template<class T> INLINE PyObject *DTool_CreatePyInstance(T *obj, bool memory_rules);
template<class T> INLINE PyObject *DTool_CreatePyInstanceTyped(const T *obj, bool memory_rules);
template<class T> INLINE PyObject *DTool_CreatePyInstanceTyped(T *obj, bool memory_rules);

// Macro(s) class definition .. Used to allocate storage and init some values
// for a Dtool Py Type object.

// struct Dtool_PyTypedObject Dtool_##CLASS_NAME;

#define Define_Module_Class_Internal(MODULE_NAME,CLASS_NAME,CNAME)\
extern struct Dtool_PyTypedObject Dtool_##CLASS_NAME;\
static int Dtool_Init_##CLASS_NAME(PyObject *self, PyObject *args, PyObject *kwds);\
static PyObject *Dtool_new_##CLASS_NAME(PyTypeObject *type, PyObject *args, PyObject *kwds);

#define Define_Module_Class(MODULE_NAME,CLASS_NAME,CNAME,PUBLIC_NAME)\
Define_Module_Class_Internal(MODULE_NAME,CLASS_NAME,CNAME)\
Define_Dtool_new(CLASS_NAME,CNAME)\
Define_Dtool_FreeInstance(CLASS_NAME,CNAME)\
Define_Dtool_Class(MODULE_NAME,CLASS_NAME,PUBLIC_NAME)

#define Define_Module_Class_Private(MODULE_NAME,CLASS_NAME,CNAME,PUBLIC_NAME)\
Define_Module_Class_Internal(MODULE_NAME,CLASS_NAME,CNAME)\
Define_Dtool_new(CLASS_NAME,CNAME)\
Define_Dtool_FreeInstance_Private(CLASS_NAME,CNAME)\
Define_Dtool_Class(MODULE_NAME,CLASS_NAME,PUBLIC_NAME)

#define Define_Module_ClassRef_Private(MODULE_NAME,CLASS_NAME,CNAME,PUBLIC_NAME)\
Define_Module_Class_Internal(MODULE_NAME,CLASS_NAME,CNAME)\
Define_Dtool_new(CLASS_NAME,CNAME)\
Define_Dtool_FreeInstance_Private(CLASS_NAME,CNAME)\
Define_Dtool_Class(MODULE_NAME,CLASS_NAME,PUBLIC_NAME)

#define Define_Module_ClassRef(MODULE_NAME,CLASS_NAME,CNAME,PUBLIC_NAME)\
Define_Module_Class_Internal(MODULE_NAME,CLASS_NAME,CNAME)\
Define_Dtool_new(CLASS_NAME,CNAME)\
Define_Dtool_FreeInstanceRef(CLASS_NAME,CNAME)\
Define_Dtool_Class(MODULE_NAME,CLASS_NAME,PUBLIC_NAME)

// The finalizer for simple instances.
EXPCL_INTERROGATEDB int DTool_PyInit_Finalize(PyObject *self, void *This, Dtool_PyTypedObject *type, bool memory_rules, bool is_const);

// A heler function to glu methed definition together .. that can not be done
// at code generation time becouse of multiple generation passes in
// interigate..
typedef std::map<std::string, PyMethodDef *> MethodDefmap;

EXPCL_INTERROGATEDB void Dtool_Accum_MethDefs(PyMethodDef in[], MethodDefmap &themap);

// We need a way to runtime merge compile units into a python "Module" .. this
// is done with the fallowing structors and code.. along with the support of
// interigate_module
struct LibraryDef {
  PyMethodDef *_methods;
};

#if PY_MAJOR_VERSION >= 3
EXPCL_INTERROGATEDB PyObject *Dtool_PyModuleInitHelper(LibraryDef *defs[], PyModuleDef *module_def);
#else
EXPCL_INTERROGATEDB PyObject *Dtool_PyModuleInitHelper(LibraryDef *defs[], const char *modulename);
#endif

// HACK.... Be carefull Dtool_BorrowThisReference This function can be used to
// grab the "THIS" pointer from an object and use it Required to support fom
// historical inharatence in the for of "is this instance of"..
EXPCL_INTERROGATEDB PyObject *Dtool_BorrowThisReference(PyObject *self, PyObject *args);

// We do expose a dictionay for dtool classes .. this should be removed at
// some point..
EXPCL_INTERROGATEDB PyObject *Dtool_AddToDictionary(PyObject *self1, PyObject *args);


EXPCL_INTERROGATEDB Py_hash_t DTOOL_PyObject_HashPointer(PyObject *obj);

/* Compare v to w.  Return
   -1 if v <  w or exception (PyErr_Occurred() true in latter case).
    0 if v == w.
    1 if v > w.
   XXX The docs (C API manual) say the return value is undefined in case
   XXX of error.
*/

EXPCL_INTERROGATEDB int DTOOL_PyObject_ComparePointers(PyObject *v1, PyObject *v2);
EXPCL_INTERROGATEDB int DTOOL_PyObject_Compare(PyObject *v1, PyObject *v2);

EXPCL_INTERROGATEDB PyObject *DTOOL_PyObject_RichCompare(PyObject *v1, PyObject *v2, int op);

EXPCL_INTERROGATEDB PyObject *
copy_from_make_copy(PyObject *self, PyObject *noargs);

EXPCL_INTERROGATEDB PyObject *
copy_from_copy_constructor(PyObject *self, PyObject *noargs);

EXPCL_INTERROGATEDB PyObject *
map_deepcopy_to_copy(PyObject *self, PyObject *args);

/**
 * This class is returned from properties that require a settable interface,
 * ie. something.children[i] = 3.
 */
struct Dtool_SequenceWrapper {
  PyObject_HEAD
  PyObject *_base;
  lenfunc _len_func;
  ssizeargfunc _getitem_func;
  ssizeobjargproc _setitem_func;
};

EXPCL_INTERROGATEDB extern PyTypeObject Dtool_SequenceWrapper_Type;

/**
 * These functions check whether the arguments passed to a function conform to
 * certain expectations.
 */
ALWAYS_INLINE bool Dtool_CheckNoArgs(PyObject *args);
ALWAYS_INLINE bool Dtool_CheckNoArgs(PyObject *args, PyObject *kwds);
EXPCL_INTERROGATEDB bool Dtool_ExtractArg(PyObject **result, PyObject *args,
                                          PyObject *kwds, const char *keyword);
EXPCL_INTERROGATEDB bool Dtool_ExtractArg(PyObject **result, PyObject *args,
                                          PyObject *kwds);
EXPCL_INTERROGATEDB bool Dtool_ExtractOptionalArg(PyObject **result, PyObject *args,
                                                  PyObject *kwds, const char *keyword);
EXPCL_INTERROGATEDB bool Dtool_ExtractOptionalArg(PyObject **result, PyObject *args,
                                                  PyObject *kwds);

/**
 * These functions convert a C++ value into the corresponding Python object.
 * This used to be generated by the code generator, but it seems more reliable
 * and maintainable to define these as overloads and have the compiler sort
 * it out.
 */
ALWAYS_INLINE PyObject *Dtool_WrapValue(int value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(unsigned int value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(long value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(unsigned long value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(long long value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(unsigned long long value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(bool value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(double value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(const char *value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(const wchar_t *value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(const std::string &value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(const std::wstring &value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(const std::string *value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(const std::wstring *value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(char value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(wchar_t value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(nullptr_t);
ALWAYS_INLINE PyObject *Dtool_WrapValue(PyObject *value);
ALWAYS_INLINE PyObject *Dtool_WrapValue(const std::vector<unsigned char> &value);

#if PY_MAJOR_VERSION >= 0x02060000
ALWAYS_INLINE PyObject *Dtool_WrapValue(Py_buffer *value);
#endif

template<class T1, class T2>
ALWAYS_INLINE PyObject *Dtool_WrapValue(const std::pair<T1, T2> &value);

EXPCL_INTERROGATEDB extern struct Dtool_PyTypedObject Dtool_DTOOL_SUPER_BASE;
EXPCL_INTERROGATEDB extern void Dtool_PyModuleClassInit_DTOOL_SUPER_BASE(PyObject *module);

#define Dtool_Ptr_DTOOL_SUPER_BASE (&Dtool_DTOOL_SUPER_BASE)

#include "py_panda.I"

#endif  // HAVE_PYTHON && !CPPPARSER

#endif // PY_PANDA_H_
