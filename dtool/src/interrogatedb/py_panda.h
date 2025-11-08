/**
 * @file py_panda.h
 */

#ifndef PY_PANDA_H_
#define PY_PANDA_H_

#include <set>
#include <map>
#include <string>
#include <vector>

#ifdef USE_DEBUG_PYTHON
#define  Py_DEBUG
#endif

#include "pnotify.h"
#include "register_type.h"

#if defined(HAVE_PYTHON) && !defined(CPPPARSER)

// py_compat.h includes Python.h.
#include "py_compat.h"
#include <structmember.h>

// this is tempory .. untill this is glued better into the panda build system

#if defined(_WIN32) && !defined(LINK_ALL_STATIC)
#define EXPORT_THIS __declspec(dllexport)
#define IMPORT_THIS extern __declspec(dllimport)
#else
#define EXPORT_THIS
#define IMPORT_THIS extern
#endif

struct Dtool_PyInstDef;
struct Dtool_PyTypedObject;

// used to stamp dtool instance..
#define PY_PANDA_SIGNATURE 0xbeaf
typedef void *(*UpcastFunction)(PyObject *,Dtool_PyTypedObject *);
typedef PyObject *(*WrapFunction)(void *, PyTypeObject *);
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

// The Class Definition Structor For a Dtool python type.
struct Dtool_PyTypedObject {
  // Standard Python Features..
  PyTypeObject _PyType;

  // May be TypeHandle::none() to indicate a non-TypedObject class.
  TypeHandle _type;

  ModuleClassInitFunction _Dtool_ModuleClassInit;

  UpcastFunction _Dtool_UpcastInterface;    // The Upcast Function By Slot
  WrapFunction _Dtool_WrapInterface; // The Downcast Function By Slot

  CoerceFunction _Dtool_ConstCoerce;
  CoerceFunction _Dtool_Coerce;
};

// Extract the PyTypeObject pointer corresponding to a Dtool_PyTypedObject.
#define Dtool_GetPyTypeObject(type) (&(type)->_PyType)

// Use DtoolInstance_Check to check whether a PyObject* is a DtoolInstance.
#define DtoolInstance_Check(obj) \
  (Py_TYPE(obj)->tp_basicsize >= (int)sizeof(Dtool_PyInstDef) && \
   ((Dtool_PyInstDef *)obj)->_signature == PY_PANDA_SIGNATURE)

// These macros access the DtoolInstance without error checking.
#define DtoolInstance_TYPE(obj) (((Dtool_PyInstDef *)obj)->_My_Type)
#define DtoolInstance_IS_CONST(obj) (((Dtool_PyInstDef *)obj)->_is_const)
#define DtoolInstance_VOID_PTR(obj) (((Dtool_PyInstDef *)obj)->_ptr_to_object)
#define DtoolInstance_INIT_PTR(obj, ptr) { ((Dtool_PyInstDef *)obj)->_ptr_to_object = (void*)(ptr); }
#define DtoolInstance_UPCAST(obj, type) (((Dtool_PyInstDef *)(obj))->_My_Type->_Dtool_UpcastInterface((obj), &(type)))

class DtoolProxy {
public:
  mutable PyObject *_self;
  TypeHandle _type;
};

/**

 */
EXPCL_PYPANDA void DTOOL_Call_ExtractThisPointerForType(PyObject *self, Dtool_PyTypedObject *classdef, void **answer);

EXPCL_PYPANDA void *DTOOL_Call_GetPointerThisClass(PyObject *self, Dtool_PyTypedObject *classdef, int param, const std::string &function_name, bool const_ok, bool report_errors);

EXPCL_PYPANDA bool Dtool_Call_ExtractThisPointer(PyObject *self, Dtool_PyTypedObject &classdef, void **answer);

EXPCL_PYPANDA bool Dtool_Call_ExtractThisPointer_NonConst(PyObject *self, Dtool_PyTypedObject &classdef,
                                                          void **answer, const char *method_name);

template<class T> INLINE bool DtoolInstance_GetPointer(PyObject *self, T *&into);
template<class T> INLINE bool DtoolInstance_GetPointer(PyObject *self, T *&into, Dtool_PyTypedObject &classdef);

EXPCL_PYPANDA PyObject *Dtool_Raise_AssertionError();
EXPCL_PYPANDA PyObject *Dtool_Raise_TypeError(const char *message);
EXPCL_PYPANDA PyObject *Dtool_Raise_ArgTypeError(PyObject *obj, int param, const char *function_name, const char *type_name);
EXPCL_PYPANDA PyObject *Dtool_Raise_AttributeError(PyObject *obj, const char *attribute);
EXPCL_PYPANDA int Dtool_Raise_CantDeleteAttributeError(const char *attribute);

EXPCL_PYPANDA PyObject *_Dtool_Raise_BadArgumentsError();
EXPCL_PYPANDA PyObject *_Dtool_Raise_BadArgumentsError(const char *message);
EXPCL_PYPANDA int _Dtool_Raise_BadArgumentsError_Int();
EXPCL_PYPANDA int _Dtool_Raise_BadArgumentsError_Int(const char *message);
#ifdef NDEBUG
// Define it to a function that just prints a generic message.
#define Dtool_Raise_BadArgumentsError(x) _Dtool_Raise_BadArgumentsError()
#define Dtool_Raise_BadArgumentsError_Int(x) _Dtool_Raise_BadArgumentsError_Int()
#else
// Expand this to a TypeError listing all of the overloads.
#define Dtool_Raise_BadArgumentsError(x) _Dtool_Raise_BadArgumentsError(x)
#define Dtool_Raise_BadArgumentsError_Int(x) _Dtool_Raise_BadArgumentsError_Int(x)
#endif

/**

 */
EXPCL_PYPANDA PyObject *DTool_CreatePyInstanceTyped(void *local_this_in, Dtool_PyTypedObject &known_class_type, bool memory_rules, bool is_const, int RunTimeType);

// DTool_CreatePyInstance .. wrapper function to finalize the existance of a
// general dtool py instance..
EXPCL_PYPANDA PyObject *DTool_CreatePyInstance(void *local_this, Dtool_PyTypedObject &in_classdef, bool memory_rules, bool is_const);

// These template methods allow use when the Dtool_PyTypedObject is not known.
// They require a get_class_type() to be defined for the class.
template<class T> INLINE PyObject *DTool_CreatePyInstance(const T *obj, bool memory_rules);
template<class T> INLINE PyObject *DTool_CreatePyInstance(T *obj, bool memory_rules);
template<class T> INLINE PyObject *DTool_CreatePyInstanceTyped(const T *obj, bool memory_rules);
template<class T> INLINE PyObject *DTool_CreatePyInstanceTyped(T *obj, bool memory_rules);

// The finalizer for simple instances.
INLINE int DTool_PyInit_Finalize(PyObject *self, void *This, Dtool_PyTypedObject *type, bool memory_rules, bool is_const);

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
ALWAYS_INLINE PyObject *Dtool_WrapValue(std::nullptr_t);
ALWAYS_INLINE PyObject *Dtool_WrapValue(PyObject *value);
template<class Allocator>
ALWAYS_INLINE PyObject *Dtool_WrapValue(const std::vector<unsigned char, Allocator> &value);

#if PY_MAJOR_VERSION >= 0x02060000
ALWAYS_INLINE PyObject *Dtool_WrapValue(Py_buffer *value);
#endif

template<class T1, class T2>
ALWAYS_INLINE PyObject *Dtool_WrapValue(const std::pair<T1, T2> &value);

EXPCL_PYPANDA Dtool_PyTypedObject *Dtool_GetSuperBase();

/**
 * Creates a Python generator object from a next() function.
 */
EXPCL_PYPANDA PyObject *Dtool_NewGenerator(PyObject *self, iternextfunc func);

#include "py_panda.I"

#endif  // HAVE_PYTHON && !CPPPARSER

#endif // PY_PANDA_H_
